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

#ifndef INCLUDED_AIR_MS_ATTRIBUTES_H
#define INCLUDED_AIR_MS_ATTRIBUTES_H

// Attribute Codes

const int MS_VALID_PULSE    = 0x10000000;  // Pulse is above threshold
const int MS_LEADING_EDGE   = 0x20000000; // Pulse is above threshold with a good leading edge
const int MS_PREAMBLE_START = 0x40000000;  // Start of Preamble or Frame
const int MS_FRAME_END      = 0x80000000;  // Frame has ended
const int MS_DATA_MASK      = 0x0FFFFFFF;  // Data passed downstream

// Data Offsets from Preamble Start  (Used downstream from the framer)
const int MS__OFFSET_REFERENCE = 0;   // Reference Level
const int MS__OFFSET_COUNT_HIGH = 1;  // Sample Count High 16 bits
const int MS__OFFSET_COUNT_LOW = 2;  // Sample Count Low 16 bits

#endif /* INCLUDED_AIR_MS_ATTRIBUTES_H */
