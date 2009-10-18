/*
 * Copyright 2007 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef INCLUDED_AIR_MS_CONSTS_H
#define INCLUDED_AIR_MS_CONSTS_H


const int MS_SHORT_FRAME_LENGTH    =  56;  // Data length for short frame
const int MS_LONG_FRAME_LENGTH     = 112;

const int MS_DATA_RATE             = 1000000;
const int MS_PREAMBLE_PULSE_COUNT  =   4;
// Frame Times in microSeconds
const int MS_PREAMBLE_TIME_US      =   8;
const int MS_BIT_TIME_US           =   1;
#endif /* INCLUDED_AIR_MS_CONSTS_H */
