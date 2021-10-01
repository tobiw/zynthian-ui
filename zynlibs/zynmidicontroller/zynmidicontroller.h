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

/*  This file declares the library interface. Only _public_ methods are exposed here.
*/

#include <cstdint>
#include <vector>

//-----------------------------------------------------------------------------
// Library Initialization
//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C"
{
#endif


// ** Library management functions **

/** @brief  Initialise library and connect to jackd server
*   @note   Call init() before any other functions will work
*/
void init();

/** @brief  Enable debug output
*   @param  bEnable True to enable debug output
*/
void enableDebug(bool bEnable);

/** @brief  Set the MIDI channel to send CC knob messages
* @param   channel MIDI channel (0..15)
*/
void setMidiChannel(unsigned int channel);

/** @brief  Select bank for CC knobs
*   @param  bank Index of bank [1: Volume, 2: Device, 3: Pan, 4: Send 1, 5: Send 2, 6: Custom]
*/
void selectKnobs(unsigned int bank);

/** @brief  Select mode for pads
*   @param  mode Index of mode [1: Drum pad, 2: Session, 5: Custom, 6: Drum pad 2, 7: Toggle, 8: Program change]
*/
void selectPads(unsigned int mode);

/** @brief  Enable Session mode
*   @param  enable True to enable, false to disable
*/
void enableSession(bool enable);

#ifdef __cplusplus
}
#endif

