
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
/*
Convert the data in pipeline info as floats so it can be displayed on the oscilloscope
Also handle sample data so everything stays in sync.

*/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gr_io_signature.h>
#include <air_ms_types.h>
#include <air_ms_cvt_float.h>

air_ms_cvt_float_sptr air_make_ms_cvt_float()
{
    return air_ms_cvt_float_sptr(new air_ms_cvt_float());
}

air_ms_cvt_float::air_ms_cvt_float() :
    gr_sync_block ("ms_cvt_float",
                   gr_make_io_signature2 (2, 2, sizeof(float), sizeof(ms_plinfo)),
                   gr_make_io_signature (3, 3, sizeof(float)))
{
    set_output_multiple(1);
}


int air_ms_cvt_float::work(int noutput_items,
                          gr_vector_const_void_star &input_items,
	                  gr_vector_void_star &output_items)

{
    float *data_in = (float *)input_items[0];
    ms_plinfo *attrib_in = (ms_plinfo *)input_items[1];
    float *data_out = (float *) output_items[0];  // sample data out
    float *ref_out = (float *) output_items[1];    // attribute data out
    float *attrib_out = (float *) output_items[2];    // attribute data out

    int size = noutput_items;
    int i;
    for (i = 0; i < size; i++)
    {
	data_out[i] = data_in[i]; // Direct Copy
	ref_out[i] = attrib_in[i].reference();
        attrib_out[i] = (float)attrib_in[i].flags() *-20.;  // Make it visible by inverting it
    }
    return i;
}
