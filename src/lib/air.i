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

%feature("autodoc","1");
%include "exception.i"
%import "gnuradio.i"

%{
#include "gnuradio_swig_bug_workaround.h"	// mandatory bug fix
#include "air_ms_pulse_detect.h"
#include "air_ms_preamble.h"
#include "air_ms_framer.h"
#include "air_ms_ppm_decode.h"
#include "air_ms_parity.h"
#include "air_ms_ec_brute.h"
#include "air_ms_fmt_log.h"
#include "air_ms_cvt_float.h"
#include <stdexcept>
%}

// ----------------------------------------------------------------

GR_SWIG_BLOCK_MAGIC(air,ms_pulse_detect);

air_ms_pulse_detect_sptr air_make_ms_pulse_detect(float alpha, float beta, int width);

class air_ms_pulse_detect : public gr_sync_block
{
private:
    air_ms_pulse_detect(float alpha, float beta, int width);

public:
};

// ----------------------------------------------------------------

GR_SWIG_BLOCK_MAGIC(air,ms_preamble);

air_ms_preamble_sptr air_make_ms_preamble(int channel_rate);

class air_ms_preamble : public gr_sync_block
{
private:
    air_ms_preamble(int channel_rate);

public:
};

// ----------------------------------------------------------------

GR_SWIG_BLOCK_MAGIC(air,ms_framer);

air_ms_framer_sptr air_make_ms_framer(int channel_rate);

class air_ms_framer : public gr_sync_block
{
private:
    air_ms_framer(int channel_rate);

public:
};

// ----------------------------------------------------------------

GR_SWIG_BLOCK_MAGIC(air,ms_ppm_decode);

air_ms_ppm_decode_sptr air_make_ms_ppm_decode(int channel_rate);

class air_ms_ppm_decode : public gr_block
{
private:
    air_ms_ppm_decode(int channel_rate);

public:
};

// ----------------------------------------------------------------

GR_SWIG_BLOCK_MAGIC(air,ms_parity);

air_ms_parity_sptr air_make_ms_parity();

class air_ms_parity : public gr_sync_block
{
private:
    air_ms_parity();

public:
};

// ----------------------------------------------------------------

GR_SWIG_BLOCK_MAGIC(air,ms_ec_brute);

air_ms_ec_brute_sptr air_make_ms_ec_brute();

class air_ms_ec_brute : public gr_sync_block
{
private:
    air_ms_ec_brute();

public:
};

// ----------------------------------------------------------------

GR_SWIG_BLOCK_MAGIC(air,ms_fmt_log);

air_ms_fmt_log_sptr air_make_ms_fmt_log(int pass_all, gr_msg_queue_sptr queue);

class air_ms_fmt_log : public gr_sync_block
{
private:
    air_ms_fmt_log(int pass_all, gr_msg_queue_sptr queue);

public:
};

// ----------------------------------------------------------------

GR_SWIG_BLOCK_MAGIC(air,ms_cvt_float);

air_ms_cvt_float_sptr air_make_ms_cvt_float();

class air_ms_cvt_float : public gr_sync_block
{
private:
    air_ms_cvt_float();

public:
};
