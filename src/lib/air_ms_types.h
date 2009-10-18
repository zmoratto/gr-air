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

#ifndef INCLUDED_AIR_MS_TYPES_H
#define INCLUDED_AIR_MS_TYPES_H

#include <time.h>            // For time_t
#include <air_ms_consts.h>   // For Mode S const values

/*!
 * \brief pipeline info that flows besides the data
 *
 * Not all modules need all the info
 */
class ms_plinfo {
public:
  ms_plinfo () : _flags (0), _reference (0.0) { }

  // accessors

  bool valid_pulse () const { return (_flags & fl_valid_pulse) != 0; }
  bool leading_edge () const { return (_flags & fl_leading_edge) != 0; }
  bool preamble_start () const { return (_flags & fl_preamble_start) != 0; }
  bool data_start () const { return (_flags & fl_data_start) != 0; }
  bool data_end () const { return (_flags & fl_data_end) != 0; }

  float reference ()	const { return _reference; }
  unsigned int flags () const { return _flags; }

  // setters

  void set_valid_pulse ()
  {
    _flags |= fl_valid_pulse;
  }

  void set_leading_edge ()
  {
    _flags |= fl_leading_edge;
  }

  void set_reference (float reference)
  {
    _reference = reference;
  }
  void set_preamble_start (float reference)
  {
    _reference = reference;
    _flags |= fl_preamble_start;
  }

  void set_data_start (float reference)
  {
    _reference = reference;
    _flags |= fl_data_start;
  }
  void set_data_end ()
  {
    _flags |= fl_data_end;
  }

 // resetters

  void reset_all ()
  {
    _reference = 0.0;
    _flags = 0;
  }
  void reset_preamble_start ()
  {
    _reference = 0.0;
    _flags &= ~fl_preamble_start;
  }
  // overload equality operator
  bool operator== (const ms_plinfo &other) const {
    return (_flags == other._flags && _reference == other._reference);
  }

  bool operator!= (const ms_plinfo &other) const {
    return !(_flags == other._flags && _reference == other._reference);
  }


protected:
  unsigned short	_flags;		// bitmask
  float			_reference;	// reference level

  //     This value is above the threshold
  static const int	fl_valid_pulse		= 0x0001;
  //	 This marks the leading edge of a pulse
  static const int	fl_leading_edge		= 0x0002;
  //	 This marks the start of a preamble
  static const int	fl_preamble_start	= 0x0004;
  //	 This marks the start of the data segment
  static const int	fl_data_start	= 0x0008;
  //	 This marks the end of the data segment
  static const int	fl_data_end	= 0x0010;
};

/*!
 * \brief Raw mode select data frame
 *
 *
 */
class ms_frame_raw {
public:
  ms_frame_raw () : _timestamp (0), _length(0),  _lcb_count(0), _first_lcb(-1), _last_lcb(-1),
                    _leb_count(0), _first_leb(-1), _last_leb(-1) { }

  // accessors

  bool bit_high_confidence (int index) const { return _flags[index] == 0; }
  bool bit_low_confidence (int index) const { return (_flags[index] & fl_low_confidence) != 0; }
  bool bit_low_energy (int index) const { return (_flags[index] & fl_low_energy) == fl_low_energy; }
  unsigned char bit (int index)	const { return _bits[index]; }
  unsigned int flags (int index) const { return _flags[index]; }
  int timestamp () const { return _timestamp; }
  float reference ()	const { return _reference; }
  int length() const { return _length; }
  short lcb_count() const { return _lcb_count; }
  short first_lcb() const { return _first_lcb; }
  short last_lcb() const { return _last_lcb; }
  short leb_count() const { return _leb_count; }
  short first_leb() const { return _first_leb; }
  short last_leb() const { return _last_leb; }
  unsigned short ec_quality() const { return _ec_quality; }
  time_t rx_time() const { return _rx_time; }
  unsigned int address() const { return _address; }

  // setters

  void set_timestamp(int timestamp)
  {
	_timestamp = timestamp;
  }

  void set_reference(float reference)
  {
	_reference = reference;
  }

  void set_short_frame ()
  {
	_length = MS_SHORT_FRAME_LENGTH;
  }

  void set_long_frame ()
  {
	_length = MS_LONG_FRAME_LENGTH;
  }

  void set_bit_high_confidence (int index, unsigned char bit)
  {
    if(index < 0 || index >= MS_LONG_FRAME_LENGTH)
	return;
    _bits[index] = bit;
    _flags[index] = fl_high_confidence;
  }

  void set_bit_low_confidence (int index, unsigned char bit)
  {
    if(index < 0 || index >= MS_LONG_FRAME_LENGTH)
	return;
    _bits[index] = bit;
    _flags[index] = fl_low_confidence;
  }

  void set_bit_low_energy (int index, unsigned char bit)
  {
    if(index < 0 || index >= MS_LONG_FRAME_LENGTH)
	return;
    _bits[index] = bit;
    _flags[index] = fl_low_energy;
  }

  void set_bit_flipped (int index)
  {
    if(index < 0 || index >= MS_LONG_FRAME_LENGTH)
	return;
    _bits[index] = (_bits[index])?0:1;
  }

  void count_lcbs ()
  {
    reset_lcb();
    for (int i = 0; i < _length; i++)
    {
    	if(_flags[i] == fl_low_confidence)
    	{
    		if(_first_lcb < 0)
			_first_lcb = i;
   		_last_lcb = i;
    		_lcb_count++;
    	}
    	else if(_flags[i] == fl_low_energy)
	{
    		if(_first_lcb < 0)
			_first_lcb = i;
   		_last_lcb = i;
    		_lcb_count++;
    		if(_first_leb < 0)
			_first_leb = i;
    		_last_leb = i;
    		_leb_count++;
	}
    }
  }

  void set_ec_quality(int ec_quality)
  {
	_ec_quality = ec_quality;
  }

  void set_rx_time(time_t rx_time)
  {
	_rx_time = rx_time;
  }

  void set_address(int address)
  {
	_address = address;
  }

 // resetters

  void reset_lcb()
  {
    _lcb_count = 0;
    _first_lcb = -1;
    _last_lcb = -1;
    _leb_count = 0;
    _first_leb = -1;
    _last_leb = -1;
  }

  void reset_all ()
  {
    _timestamp = 0;
    _reference = 0.0;
    _length = 0;
    _lcb_count = 0;
    _first_lcb = -1;
    _last_lcb = -1;
    _leb_count = 0;
    _first_leb = -1;
    _last_leb = -1;
    _ec_quality = ec_unknown;
  }
protected:
  	static const int NPAD = 134;
	int _timestamp;  // Timestamp in number of samples since start
	float _reference; // "Signal Strength"
	int _length;     // Length
        unsigned char _bits[MS_LONG_FRAME_LENGTH];  // The bits of the frame
	unsigned short _flags[MS_LONG_FRAME_LENGTH];  // The quality flags
	short _lcb_count;  // count of low confidence bits
	short _first_lcb;  // first low confidence bits
	short _last_lcb;  // first low confidence bits
	short _leb_count;  // count of low energy bits
	short _first_leb;  // first low energy bits
	short _last_leb;  // last low energy bits
	unsigned short _ec_quality;  // The quality of this
        time_t _rx_time;
	unsigned char  _frame_type;  // 1st 5 bits
	unsigned int   _address;   // airframe or interrogator address
	unsigned char _pad_[NPAD];

public:
  //     This bit is of high confidence
  static const int	fl_high_confidence	= 0x0000;
  //     This bit is of low confidence
  static const int	fl_low_confidence	= 0x0001;
  //     This bit is of low confidence (low energy)
  static const int	fl_low_energy		= 0x0003;

  //  Quality
  static const int	ec_unknown		= 0x0000;  // No checking done
  static const int	crc_bad			= 0x8000;  // totally NG
  static const int	eq_too_short_frame	= 0x4000;  // Frame too short
  static const int	eq_crc_overlayed	= 0x2000;  // CRC has data overlay and low confidence bits present
  static const int	eq_ec_na		= 0x0800;  // EC can not be done
  static const int	eq_ec_multiple		= 0x0400;  // EC can not be done (Multiple solutions
  static const int	eq_ec_corrected		= 0x0004;  // EC was done
  static const int	eq_change_short_frame	= 0x0002;  // Long frame is really short
  static const int	crc_ok			= 0x0001;  // Everything ok

};
#endif /* INCLUDED_AIR_MS_TYPES_H */
