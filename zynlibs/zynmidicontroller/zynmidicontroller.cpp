/*
 * ******************************************************************
 * ZYNTHIAN PROJECT: Zynmidicontroller Library
 *
 * Library providing interface to MIDI pad controllers
 *
 * Copyright (C) 2021 Brian Walton <brian@riban.co.uk>
 *
 * ******************************************************************
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * For a full copy of the GNU General Public License see the LICENSE.txt file.
 *
 * ******************************************************************
 */

#include <stdio.h> //provides printf
#include <stdlib.h> //provides exit
#include <thread> //provides thread for timer
#include <jack/jack.h> //provides JACK interface
#include <jack/midiport.h> //provides JACK MIDI interface
#include "zynmidicontroller.h" //exposes library methods as c functions
#include <cstring> //provides strstr
#include <lo/lo.h> //provides OSC interface
#include <lo/lo_cpp.h> //provides C++ OSC interface

#define DPRINTF(fmt, args...) if(g_bDebug) printf(fmt, ## args)

jack_port_t * g_pInputPortDevice; // Pointer to the JACK input port connected to controller
jack_port_t * g_pOutputPortDevice; // Pointer to the JACK output port connected to controller
jack_port_t * g_pOutputPort; // Pointer to the JACK output port connected to zynmidirouter
jack_client_t *g_pJackClient = NULL; // Pointer to the JACK client
unsigned int g_nInputProtocol = -1; // Value of the protocol used by controller connected to MIDI
unsigned int g_nOutputProtocol = -1; // Value of the protocol used by controller connected to MIDI
unsigned int g_nProtocol = -1; // Index of the protocol to use for device control
bool g_bShift = false; // True if shift button is pressed

const char* g_sSupported[] = {"Launchkey-Mini-MK3-MIDI-2"};
size_t g_nSupportedQuant = sizeof(g_sSupported) / sizeof(const char*);
int g_nDrumPads[] = {40,41,42,43,44,45,46,47,48,49,50,51,36,37,38,39,40,41,42,43,44,45,46,47};
int g_nSessionPads[] = {96,97,98,99,100,101,102,103,112,113,114,115,116,117,118,119};
int g_nPadColours[] = {67,35,9,47,105,63,94,126,40,81,8,45,28,95,104,44}; //!@todo Pad colours are specific to LaunchKey Mk3
int g_nPadColour[] = {67,35,9,51,105,63,94,126,67,35,9,51,105,63,94,126};
int g_nPadStatus[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int g_nDrumColour = 79;
int g_nDrumOnColour = 90;
int g_nStartingColour = 123;
int g_nStoppingColour = 120;
int g_nCCoffset = 0; // Offset to add to CC controllers (base is 21 for controller 1)
int g_nMidiChannel = 0; // MIDI channel to send CC messages

struct MIDI_MESSAGE {
    uint8_t command = 0;
    uint8_t value1 = 0;
    uint8_t value2 = 0;
};

std::vector<MIDI_MESSAGE*> g_vSendQueue; // Queue of MIDI events to send
bool g_bDebug = false; // True to output debug info
bool g_bMutex = false; // Mutex lock for access to g_vSendQueue

void onOscConfig(lo_arg **pArgs, int nArgs)
{
    printf("zynmidicontroller onOscConfig\n");
}

lo::Address g_oscClient("localhost", "1370");
lo::ServerThread g_oscServer(2001);

// ** Internal (non-public) functions  (not delcared in header so need to be in correct order in source file) **

// Enable / disable debug output
void enableDebug(bool bEnable)
{
    printf("libmidicontroller setting debug mode %s\n", bEnable?"on":"off");
    g_bDebug = bEnable;
}

// Check if both device input and output are connected
bool isDeviceConnected() {
    if(g_nInputProtocol == g_nOutputProtocol)
        g_nProtocol = g_nInputProtocol;
    return g_nProtocol != -1;
}

// Add a MIDI command to queue to be sent on next jack cycle
void sendDeviceMidi(uint8_t status, uint8_t value1, uint8_t value2)
{
    if(status < 128 || value1 > 127 && value2 > 127)
        return;
    MIDI_MESSAGE* pMsg = new MIDI_MESSAGE;
    pMsg->command = status;
    pMsg->value1 = value1;
    pMsg->value2 = value2;
    while(g_bMutex)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    g_bMutex = true;
    g_vSendQueue.push_back(pMsg);
    g_bMutex = false;
}

void stopped(int pad) {
    sendDeviceMidi(0x90, g_nSessionPads[pad], g_nPadColour[pad]);
    g_nPadStatus[pad] = 0;
}

void starting(int pad) {
    sendDeviceMidi(0x90, g_nSessionPads[pad], g_nPadColour[pad]);
    sendDeviceMidi(0x91, g_nSessionPads[pad], g_nStartingColour);
    g_nPadStatus[pad] = 1;
}

void playing(int pad) {
    sendDeviceMidi(0x92, g_nSessionPads[pad], g_nPadColour[pad]);
    g_nPadStatus[pad] = 2;
}

void stopping(int pad) {
    sendDeviceMidi(0x90, g_nSessionPads[pad], g_nPadColour[pad]);
    sendDeviceMidi(0x91, g_nSessionPads[pad], g_nStoppingColour);
    g_nPadStatus[pad] = 3;
}

void disabled(int pad) {
    sendDeviceMidi(0x90, g_nSessionPads[pad], 0);
    g_nPadStatus[pad] = 0xFFFF;
}

void selectMode(int mode) {
    sendDeviceMidi(0xbf, 3, mode);
}

void onOscStatus(lo_arg **pArgs, int nArgs)
{
    int nBank = pArgs[0]->i;
    int nSequence = pArgs[1]->i;
    int nState = pArgs[2]->i;
    int nGroup = pArgs[3]->i;
    //printf("OSC Bank %d Sequence %d State %d Group %c\n", nBank, nSequence, nState, 'A'+nGroup);
    if(nSequence > 15)
        return; //!@todo Handle pad offsets
    g_nPadColour[nSequence] = g_nPadColours[nGroup % 16];
    switch(nState) {
        case 0:
            // Stopped
            stopped(nSequence);
            break;
        case 1:
            // Playing
            playing(nSequence);
            break;
        case 2:
            // Stopping
            stopping(nSequence);
            break;
        case 3:
            // Starting
        case 4:
            // Restarting
            starting(nSequence);
            break;
        case 0xFFFF:
            // Disabled
            disabled(nSequence);
    }
}


void enableDevice(bool enable) {
    if(!isDeviceConnected())
        return;
    if(enable) {
        g_oscServer.add_method("/sequence/config", "iii", onOscConfig);
        g_oscServer.add_method("/sequence/status", "iiii", onOscStatus);
        g_oscServer.start();
        g_oscClient.send("/cuia/register", "sis", "localhost", 2001, "/SEQUENCER/STATE");
        g_oscClient.send("/cuia/register", "sis", "localhost", 2001, "/SEQUENCER/CONFIG");
    } else {
        g_oscServer.del_method("/sequence/config", "iii");
        g_oscServer.del_method("/sequence/status", "iiii");
        g_oscServer.stop();
        g_oscClient.send("/cuia/unregister", "sis", "localhost", 2001, "/SEQUENCER/STATE");
        g_oscClient.send("/cuia/unregister", "sis", "localhost", 2001, "/SEQUENCER/CONFIG");
    }

    switch(g_nProtocol) {
        case 0:
            // Novation Launchkey Mini
            sendDeviceMidi(0x9f, 12, enable?127:0);
            DPRINTF("\tSession mode %s\n", enable?"enabled":"disabled");
            if(!enable)
                return;
            for(int pad = 0; pad < 16; ++pad)
                sendDeviceMidi(0x99, g_nDrumPads[pad], g_nDrumColour);
            for(int pad = 0; pad < 16; ++pad)
                stopped(pad);
            selectKnobs(1); // Select "Volume" for CC knobs (to avoid undefined state)
            break;
        default:
            break;
    }
}

// Initialise LaunchKey device
void initLaunchkey(size_t protocol) {
    if(protocol >= g_nSupportedQuant)
        return;
    g_nProtocol = -1;
    if(!isDeviceConnected())
        return;
    g_nProtocol = protocol;
    printf("Initialising controller interface with protocol %s\n", g_sSupported[protocol]);
    enableDevice(true);
}

// Send MIDI command to normal output (not to control device)
inline void sendMidi(void* pOutputBuffer, int command, int value1, int value2) {
    unsigned char* pBuffer = jack_midi_event_reserve(pOutputBuffer, 0, 3); //!@todo Check if offset 0 is valid
    if(pBuffer == NULL)
        return; // Exceeded buffer size (or other issue)
    pBuffer[0] = command;
    pBuffer[1] = value1;
    pBuffer[2] = value2;
    //DPRINTF("Sending MIDI event 0x%2X,%d,%d to zynmidirouter\n", pBuffer[0],pBuffer[1],pBuffer[2]);
}

// Handle received MIDI events based on selected protocol
inline void protocolHandler(jack_midi_data_t* pBuffer, void* pOutputBuffer) {
    int channel = pBuffer[0] & 0x0F + 1;
    switch(g_nProtocol) {
        case 0:
            // Novation Launchkey Mini
            switch(pBuffer[0] & 0xF0) {
                case 0x90:
                    //DPRINTF("NOTE ON: Channel %d Note %d Velocity %d\n", channel, pBuffer[1], pBuffer[2]);
                    if(pBuffer[1] > 35 && pBuffer[1] < 52) {
                        // Drum pads
                        sendDeviceMidi(0x99, pBuffer[1], g_nDrumOnColour);
                        sendMidi(pOutputBuffer, 0x99, pBuffer[1], pBuffer[2]);
                    } else if(pBuffer[1] > 95 && pBuffer[1] < 104) {
                        // Launch buttons 1-8
                        g_oscClient.send("/cuia/TOGGLE_SEQUENCE", "i", pBuffer[1] - 96);
                    } else if(pBuffer[1] > 111 && pBuffer[1] < 120) {
                        // Launch buttons 9-16
                        g_oscClient.send("/cuia/TOGGLE_SEQUENCE", "i", pBuffer[1] - 104);
                    }
                    break;
                case 0x80:
                    //DPRINTF("NOTE OFF: Channel %d Note %d Velocity %d\n", channel, pBuffer[1], pBuffer[2]);
                    if(pBuffer[1] > 35 && pBuffer[1] < 52) {
                        // Drum pads
                        sendDeviceMidi(0x99, pBuffer[1], g_nDrumColour);
                        sendMidi(pOutputBuffer, 0x89, pBuffer[1], pBuffer[2]);
                    }
                    break;
                case 0xb0:
                    //DPRINTF("CC: Channel %d CC %d Value %d\n", channel, pBuffer[1], pBuffer[2]);
                    if(pBuffer[1] == 9) {
                        // Switch CC offset
                        g_nCCoffset = 8 * (pBuffer[2] - 1);
                        DPRINTF("Changing CC knob bank to %d (%d-%d)\n", pBuffer[2], 21 + g_nCCoffset, 21 + g_nCCoffset + 7);
                    } else if(pBuffer[1] == 108) {
                        // Shift button
                        g_bShift = pBuffer[2];
                        DPRINTF("Shift button %s\n", g_bShift?"pressed":"released");
                    }
                    if(g_bShift) {
                        // Shift held
                        if(pBuffer[1] == 104) {
                            // Up button
                            DPRINTF("Up button %s\n", pBuffer[2]?"pressed":"released");
                           if(pBuffer[2])
                                g_oscClient.send("/cuia/BACK_UP");
                        } else if(pBuffer[1] == 105) {
                            // Down button
                            DPRINTF("Down button %s\n", pBuffer[2]?"pressed":"released");
                           if(pBuffer[2])
                                g_oscClient.send("/cuia/BACK_DOWN");
                        } else if(pBuffer[1] == 103) {
                            // Left button
                            DPRINTF("Left button %s\n", pBuffer[2]?"pressed":"released");
                           if(pBuffer[2])
                                g_oscClient.send("/cuia/SELECT_UP");
                        } else if(pBuffer[1] == 102) {
                            // Right button
                           if(pBuffer[2])
                                g_oscClient.send("/cuia/SELECT_DOWN");
                            DPRINTF("Right button %s\n", pBuffer[2]?"pressed":"released");
                        } else if(pBuffer[1] > 20 && pBuffer[1] < 29) {
                            // CC knobs
                            sendMidi(pOutputBuffer, 0xb0 | g_nMidiChannel, pBuffer[1] + g_nCCoffset + 40, pBuffer[2]);
                        } else if(pBuffer[1] == 115) {
                            // Play button
                            DPRINTF("Shift+Play button %s\n", pBuffer[2]?"pressed":"released");
                           if(pBuffer[2])
                               g_oscClient.send("/cuia/TOGGLE_AUDIO_PLAY");
                        } else if(pBuffer[1] == 117) {
                            // Record button
                            DPRINTF("Shift+Record button %s\n", pBuffer[2]?"pressed":"released");
                           if(pBuffer[2])
                                g_oscClient.send("/cuia/TOGGLE_AUDIO_RECORD");
                        }
                    } else {
                        // Shift not held
                        if(pBuffer[1] == 104) {
                            // Launch button
                            DPRINTF("Launch button %s\n", pBuffer[2]?"pressed":"released");
                           if(pBuffer[2])
                                g_oscClient.send("/cuia/SWITCH_SELECT_SHORT");
                        } else if(pBuffer[1] == 105) {
                            // Stop/Solo/Mute button
                            DPRINTF("Stop/Solo/Mute button %s\n", pBuffer[2]?"pressed":"released");
                            if(pBuffer[2])
                                g_oscClient.send("/cuia/SWITCH_BACK_SHORT");
                        } else if(pBuffer[1] > 20 && pBuffer[1] < 29) {
                            // CC knobs
                            sendMidi(pOutputBuffer, 0xb0 | g_nMidiChannel, pBuffer[1] + g_nCCoffset, pBuffer[2]);
                        } else if(pBuffer[1] == 115) {
                            // Play button
                            DPRINTF("Play button %s\n", pBuffer[2]?"pressed":"released");
                           if(pBuffer[2])
                                g_oscClient.send("/cuia/TOGGLE_MIDI_PLAY");
                        } else if(pBuffer[1] == 117) {
                            // Record button
                            DPRINTF("Record button %s\n", pBuffer[2]?"pressed":"released");
                           if(pBuffer[2])
                                g_oscClient.send("/cuia/TOGGLE_MIDI_RECORD");
                        }
                    }
                    break;
                default:
                    // MIDI command not handled
                    break;
            }
        default:
            // Protocol not defined
            break;
    }
}

/*  Process jack cycle - must complete within single jack period
    nFrames: Quantity of frames in this period
    pArgs: Parameters passed to function by main thread (not used here)

    [For info]
    jack_last_frame_time() returns the quantity of samples since JACK started until start of this period
    jack_midi_event_write sends MIDI message at sample time sequence within this period

    [Process]
    Process incoming MIDI events
    Send pending MIDI events
    Remove events from queue
*/
int onJackProcess(jack_nframes_t nFrames, void *pArgs)
{
    if(!g_pJackClient)
        return 0;
    // Get output buffers that will be processed in this process cycle
    void* pOutputBuffer = jack_port_get_buffer(g_pOutputPort, nFrames);
    void* pDeviceOutputBuffer = jack_port_get_buffer(g_pOutputPortDevice, nFrames);
    unsigned char* pBuffer;
    jack_midi_clear_buffer(pOutputBuffer);
    jack_midi_clear_buffer(pDeviceOutputBuffer);

    // Process MIDI input
    void* pInputBuffer = jack_port_get_buffer(g_pInputPortDevice, nFrames);
    jack_midi_event_t midiEvent;
    jack_nframes_t nCount = jack_midi_get_event_count(pInputBuffer);
    for(jack_nframes_t i = 0; i < nCount; i++)
    {
        jack_midi_event_get(&midiEvent, pInputBuffer, i);
        protocolHandler(midiEvent.buffer, pOutputBuffer);
    }

    // Send MIDI output aligned with first sample of frame resulting in similar latency to audio
    while(g_bMutex)
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    g_bMutex = true;

    // Process events scheduled to be sent to device MIDI output
    for(auto it = g_vSendQueue.begin(); it != g_vSendQueue.end(); ++it) {
        pBuffer = jack_midi_event_reserve(pDeviceOutputBuffer, 0, 3); //!@todo Check if offset 0 is valid
        if(pBuffer == NULL)
            break; // Exceeded buffer size (or other issue)
        pBuffer[0] = (*it)->command;
        pBuffer[1] = (*it)->value1;
        pBuffer[2] = (*it)->value2;
        //DPRINTF("Sending MIDI event 0x%2X,%d,%d to device\n", pBuffer[0],pBuffer[1],pBuffer[2]);
        delete(*it);
    }
    g_vSendQueue.clear(); //!@todo May clear unsent messages if there was an issue with the buffer
    g_bMutex = false;
    return 0;
}

void onJackConnect(jack_port_id_t port_a, jack_port_id_t port_b, int connect, void *arg) {
    // Need to monitor supported controllers - do we support multiple simultaneous controllers?
    /*
        Check if it is one of our ports
        Check if remote port is supported device
        Check if it is connect or disconnect
        For now just accept one supported device and drop all others - may add ports for multiple devices in future
    */
    if(!g_pJackClient)
        return;
    DPRINTF("connection: %d %s %d\n", port_a, connect?"connected to":"disconnected from", port_b);
    jack_port_t* pSrcPort = jack_port_by_id(g_pJackClient, port_a);
    jack_port_t* pDstPort = jack_port_by_id(g_pJackClient, port_b);
    if(pDstPort == g_pInputPortDevice) {
        char * aliases[2];
        aliases[0] = (char *) malloc (jack_port_name_size());
        aliases[1] = (char *) malloc (jack_port_name_size());
        int nAliases = jack_port_get_aliases(pSrcPort, aliases);
        for(int i = 0; i < nAliases; ++i) {
            for(int j = 0; j < g_nSupportedQuant; ++j) {
                if(strstr(aliases[i], g_sSupported[j])) {
                    g_nInputProtocol = connect?j:-1;
                    DPRINTF("%s %s zynmidicontroller input\n", aliases[i], connect?"connected to":"disconnected from");
                    initLaunchkey(j);
                }
            }
        }
        free(aliases[0]);
        free(aliases[1]);
    }
    else if(pSrcPort == g_pOutputPortDevice) {
        char * aliases[2];
        aliases[0] = (char *) malloc (jack_port_name_size());
        aliases[1] = (char *) malloc (jack_port_name_size());
        int nAliases = jack_port_get_aliases(pDstPort, aliases);
        for(int i = 0; i < nAliases; ++i) {
            for(int j = 0; j < g_nSupportedQuant; ++j) {
                if(strstr(aliases[i], g_sSupported[j])) {
                    g_nOutputProtocol = connect?j:-1;
                    DPRINTF("zynmidicontroller output %s %s\n", connect?"connected to":"disconnected from", aliases[i]);
                    initLaunchkey(j);
                }
            }
        }
        free(aliases[0]);
        free(aliases[1]);
    }
}

// ** Library management functions **

void init() {
    // Register with Jack server
    printf("**zynmidicontroller initialising**\n");
    
    if(g_pJackClient)
    {
        fprintf(stderr, "libzynmidicontroller already initialised\n");
        return; // Already initialised
    }

    if((g_pJackClient = jack_client_open("zynmidicontroller", JackNoStartServer, NULL)) == 0)
    {
        fprintf(stderr, "libzynmidicontroller failed to start jack client\n");
        return;
    }
    // Create input port
    if(!(g_pInputPortDevice = jack_port_register(g_pJackClient, "controller input", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0)))
    {
        fprintf(stderr, "libzynmidicontroller cannot register device input port\n");
        return;
    }
    // Create output port
    if(!(g_pOutputPortDevice = jack_port_register(g_pJackClient, "controller output", JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0)))
    {
        fprintf(stderr, "libzynmidicontroller cannot register device output port\n");
        return;
    }
    
    // Create output port
    if(!(g_pOutputPort = jack_port_register(g_pJackClient, "output", JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0)))
    {
        fprintf(stderr, "libzynmidicontroller cannot register output port\n");
        return;
    }
    
    // Register JACK callbacks
    jack_set_process_callback(g_pJackClient, onJackProcess, 0);
    jack_set_port_connect_callback(g_pJackClient, onJackConnect, 0);
    
    if(jack_activate(g_pJackClient)) {
        fprintf(stderr, "libzynmidicontroller cannot activate client\n");
        return;
    }

    // Register the cleanup function to be called when program exits
    printf("zynmidicontroller initialisation complete\n");
}

__attribute__((constructor)) void zynmidicontroller(void) {
    printf("New instance of zynmidicontroller\n");
    init();
}

__attribute__((destructor)) void zynmidicontrollerend(void) {
    printf("Destroy instance of zynmidicontroller\n");
    g_bMutex = true;
    for(auto it = g_vSendQueue.begin(); it != g_vSendQueue.end(); ++it)
        delete *it;
    g_vSendQueue.clear();
    g_bMutex = false;
//    jack_client_close(g_pJackClient);
//    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
}

void activate(bool activate) {
    if(!g_pJackClient)
        return;
    if(activate)
        jack_activate(g_pJackClient);
    else
        jack_deactivate(g_pJackClient);
}

// Public functions

void setMidiChannel(unsigned int channel) {
    if(channel < 16)
        g_nMidiChannel = channel;
}

void selectKnobs(unsigned int bank) {
    switch(g_nProtocol) {
        case 0:
            // Novation Launchkey Mini
            if(isDeviceConnected() && bank < 7) {
                g_nCCoffset = bank;
                sendDeviceMidi(0xbf, 9, bank);
                DPRINTF("\tKnob bank %d selected\n", bank);
            }
    }
}

void selectPads(unsigned int mode) {
    switch(g_nProtocol) {
        case 0:
            // Novation Launchkey Mini
            if(isDeviceConnected()) {
                sendDeviceMidi(0xbf, 3, mode);
                DPRINTF("\tPad mode %d selected\n", mode);
            }
    }
}

const char* getSupported(bool reset) {
    static size_t nIndex = 0;
    if(reset) {
        if(g_nProtocol == -1)
            nIndex = 0;
        else
            nIndex = g_nProtocol;
    } else {
        if(g_nProtocol != -1) {
            if(nIndex < g_nProtocol)
                nIndex = g_nProtocol;
            else
                nIndex = g_nSupportedQuant;
        }
    }
    if(nIndex >= g_nSupportedQuant)
        return NULL;
    return(g_sSupported[nIndex++]);
}