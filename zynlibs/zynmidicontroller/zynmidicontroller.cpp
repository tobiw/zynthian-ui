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
#include <cstring> //provides strcmp

#define DPRINTF(fmt, args...) if(g_bDebug) printf(fmt, ## args)

jack_port_t * g_pInputPortDevice; // Pointer to the JACK input port connected to controller
jack_port_t * g_pOutputPortDevice; // Pointer to the JACK output port connected to controller
jack_port_t * g_pOutputPort; // Pointer to the JACK output port connected to zynmidirouter
jack_client_t *g_pJackClient = NULL; // Pointer to the JACK client
bool g_bInputConnected = false; // True if MIDI connected to MIDI controller input and output
bool g_bOutputConnected = false; // True if MIDI connected to MIDI controller input and output
bool g_bShift = false; // True if shift button is pressed

int g_nDrumPads[] = {40,41,42,43,44,45,46,47,48,49,50,51,36,37,38,39,40,41,42,43,44,45,46,47};
int g_nSessionPads[] = {96,97,98,99,100,101,102,103,112,113,114,115,116,117,118,119};
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

// ** Internal (non-public) functions  (not delcared in header so need to be in correct order in source file) **

// Enable / disable debug output
void enableDebug(bool bEnable)
{
    printf("libmidicontroller setting debug mode %s\n", bEnable?"on":"off");
    g_bDebug = bEnable;
}

// Check if both device input and output are connected
bool isDeviceConnected() {
    return g_bOutputConnected & g_bInputConnected;
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

void selectMode(int mode) {
    sendDeviceMidi(0xbf, 3, mode);
}

// Initialise LaunchKey device
void initLaunchkey() {
    if(!isDeviceConnected())
        return;
    printf("Initialising LaunckKey\n");
    enableSession(true);
    for(int pad = 0; pad < 16; ++pad)
        sendDeviceMidi(0x99, g_nDrumPads[pad], g_nDrumColour);
    for(int pad = 0; pad < 16; ++pad)
        stopped(pad);
    selectKnobs(1); // Select "Volume" for CC knobs (to avoid undefined state)
}

// Send MIDI command to normal output (not to control device)
inline void sendMidi(void* pOutputBuffer, int command, int value1, int value2) {
    unsigned char* pBuffer = jack_midi_event_reserve(pOutputBuffer, 0, 3); //!@todo Check if offset 0 is valid
    if(pBuffer == NULL)
        return; // Exceeded buffer size (or other issue)
    pBuffer[0] = command;
    pBuffer[1] = value1;
    pBuffer[2] = value2;
    DPRINTF("Sending MIDI event 0x%2X,%d,%d to zynmidirouter\n", pBuffer[0],pBuffer[1],pBuffer[2]);
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
        int channel = midiEvent.buffer[0] & 0x0F + 1;
        jack_midi_event_get(&midiEvent, pInputBuffer, i);
        switch(midiEvent.buffer[0] & 0xF0)
        {
            case 0x90:
                DPRINTF("NOTE ON: Channel %d Note %d Velocity %d\n", channel, midiEvent.buffer[1], midiEvent.buffer[2]);
                if(midiEvent.buffer[1] > 35 && midiEvent.buffer[1] < 52) {
                    // Drum pads
                    sendDeviceMidi(0x99, midiEvent.buffer[1], g_nDrumOnColour);
                    sendMidi(pOutputBuffer, 0x99, midiEvent.buffer[1], midiEvent.buffer[2]);
                }
                break;
            case 0x80:
                DPRINTF("NOTE OFF: Channel %d Note %d Velocity %d\n", channel, midiEvent.buffer[1], midiEvent.buffer[2]);
                if(midiEvent.buffer[1] > 35 && midiEvent.buffer[1] < 52) {
                    // Drum pads
                    sendDeviceMidi(0x99, midiEvent.buffer[1], g_nDrumColour);
                    sendMidi(pOutputBuffer, 0x89, midiEvent.buffer[1], midiEvent.buffer[2]);
                }
                break;
            case 0xb0:
                DPRINTF("CC: Channel %d CC %d Value %d\n", channel, midiEvent.buffer[1], midiEvent.buffer[2]);
                if(midiEvent.buffer[1] == 9) {
                    // Switch CC offset
                    g_nCCoffset = 8 * (midiEvent.buffer[2] - 1);
                    DPRINTF("Changing CC knob bank to %d (%d-%d)\n", midiEvent.buffer[2], 21 + g_nCCoffset, 21 + g_nCCoffset + 7);
                } else if(midiEvent.buffer[1] == 108) {
                    // Shift button
                    g_bShift = midiEvent.buffer[2];
                    DPRINTF("Shift button %s\n", g_bShift?"pressed":"released");
                }

                if(g_bShift) {
                    // Shift held
                    if(midiEvent.buffer[1] == 104) {
                        // Up button
                        DPRINTF("Up button %s\n", midiEvent.buffer[2]?"pressed":"released");
                    } else if(midiEvent.buffer[1] == 105) {
                        // Down button
                        DPRINTF("Down button %s\n", midiEvent.buffer[2]?"pressed":"released");
                    } else if(midiEvent.buffer[1] == 103) {
                        // Left button
                        DPRINTF("Left button %s\n", midiEvent.buffer[2]?"pressed":"released");
                    } else if(midiEvent.buffer[1] == 102) {
                        // Right button
                        DPRINTF("Right button %s\n", midiEvent.buffer[2]?"pressed":"released");
                    } else if(midiEvent.buffer[1] > 20 && midiEvent.buffer[1] < 29) {
                        // CC knobs
                        sendMidi(pOutputBuffer, 0xb0 | g_nMidiChannel, midiEvent.buffer[1] + g_nCCoffset + 40, midiEvent.buffer[2]);
                    } else if(midiEvent.buffer[1] == 115) {
                        // Play button
                        DPRINTF("Shift+Play button %s\n", midiEvent.buffer[2]?"pressed":"released");
                    } else if(midiEvent.buffer[1] == 117) {
                        // Record button
                        DPRINTF("Shift+Record button %s\n", midiEvent.buffer[2]?"pressed":"released");
                    }
                } else {
                    // Shift not held
                    if(midiEvent.buffer[1] == 104) {
                        // Launch button
                        DPRINTF("Launch button %s\n", midiEvent.buffer[2]?"pressed":"released");
                    } else if(midiEvent.buffer[1] == 105) {
                        // Stop/Solo/Mute button
                        DPRINTF("Stop/Solo/Mute button %s\n", midiEvent.buffer[2]?"pressed":"released");
                    } else if(midiEvent.buffer[1] > 20 && midiEvent.buffer[1] < 29) {
                        // CC knobs
                        sendMidi(pOutputBuffer, 0xb0 | g_nMidiChannel, midiEvent.buffer[1] + g_nCCoffset, midiEvent.buffer[2]);
                    } else if(midiEvent.buffer[1] == 115) {
                        // Play button
                        DPRINTF("Play button %s\n", midiEvent.buffer[2]?"pressed":"released");
                    } else if(midiEvent.buffer[1] == 117) {
                        // Record button
                        DPRINTF("Record button %s\n", midiEvent.buffer[2]?"pressed":"released");
                    }
                }
                break;
            default:
                break;
        }
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
        DPRINTF("Sending MIDI event 0x%2X,%d,%d to device\n", pBuffer[0],pBuffer[1],pBuffer[2]);
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
    DPRINTF("connection: %d %s %d\n", port_a, connect?"connected to":"disconnected from", port_b);
    jack_port_t* pSrcPort = jack_port_by_id(g_pJackClient, port_a);
    jack_port_t* pDstPort = jack_port_by_id(g_pJackClient, port_b);
    if(pDstPort == g_pInputPortDevice) {
        char * aliases[2];
        aliases[0] = (char *) malloc (jack_port_name_size());
        aliases[1] = (char *) malloc (jack_port_name_size());
        int nAliases = jack_port_get_aliases(pSrcPort, aliases);
        for(int i=0; i < nAliases; ++i) {
            if(strstr(aliases[i], "Launchkey-Mini-MK3-MIDI-2")) {
                g_bInputConnected = connect;
                DPRINTF("Launchkey Mini Mk3 control port %s zynmidicontroller input\n", g_bInputConnected?"connected to":"disconnected from");
                initLaunchkey();
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
        for(int i=0; i < nAliases; ++i) {
            if(strstr(aliases[i], "Launchkey-Mini-MK3-MIDI-2")) {
                g_bOutputConnected = connect;
                DPRINTF("zynmidicontroller %s Launchkey Mini Mk3 input control port\n", g_bOutputConnected?"connected to":"disconnected from");
                initLaunchkey();
            }
        }
        free(aliases[0]);
        free(aliases[1]);
    }
}

void end()
{
    DPRINTF("zynmidicontroller exit\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

// ** Library management functions **

__attribute__((constructor)) void zynmidicontroller(void) {
    printf("New instance of zynmidicontroller\n");
    init();
}

void init() {
    // Register with Jack server
    printf("**zynmidicontroller initialising**\n");
    char *sServerName = NULL;
    jack_status_t nStatus;
    jack_options_t nOptions = JackNoStartServer;
    
    if(g_pJackClient)
    {
        fprintf(stderr, "libzynmidicontroller already initialised\n");
        return; // Already initialised
    }

    if((g_pJackClient = jack_client_open("zynmidicontroller", nOptions, &nStatus, sServerName)) == 0)
    {
        fprintf(stderr, "libzynmidicontroller failed to start jack client: %d\n", nStatus);
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
    atexit(end);
    printf("zynmidicontroller initialisation complete\n");
}


// Public functions

void setMidiChannel(unsigned int channel) {
    if(channel < 16)
        g_nMidiChannel = channel;
}

void selectKnobs(unsigned int bank) {
    if(isDeviceConnected() && bank < 7) {
        g_nCCoffset = bank;
        sendDeviceMidi(0xbf, 9, bank);
        DPRINTF("Knob bank %d selected\n", bank);
    }
}

void selectPads(unsigned int mode) {
    if(isDeviceConnected()) {
        sendDeviceMidi(0xbf, 3, mode);
        DPRINTF("Pad mode %d selected\n", mode);
    }
}

void enableSession(bool enable) {
    if(isDeviceConnected()) {
        sendDeviceMidi(0x9f, 12, enable?127:0);
        DPRINTF("Session mode %\n", enable?"enabled":"disabled");
    }
}