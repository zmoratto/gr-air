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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <math.h>              // for pow()
#include <gr_io_signature.h>
#include <air_ms_types.h>
#include <air_ms_pulse_detect.h>

air_ms_pulse_detect_sptr air_make_ms_pulse_detect(float alpha, float beta, int width)
{
    return air_ms_pulse_detect_sptr(new air_ms_pulse_detect(alpha, beta, width));
}

air_ms_pulse_detect::air_ms_pulse_detect(float alpha, float beta, int width) :
    gr_sync_block ("ms_pulse_detect",
                   gr_make_io_signature (1, 1, sizeof(float)),
                   gr_make_io_signature2 (2, 2, sizeof(float), sizeof(ms_plinfo)))
{
    d_alpha = powf(10., alpha/20.);  // Convert leading edge threshold from db to ratio
    d_beta = beta;                   // Threshold of valid pulse
    d_width = width;                 // width of valid pulse - 1
    set_history(2);	// need to look at two inputs
    set_output_multiple(1+d_width); // Look ahead for a valid pulse width
}

int air_ms_pulse_detect::work(int noutput_items,
                          gr_vector_const_void_star &input_items,
		                  gr_vector_void_star &output_items)
{
    float *in = (float *) input_items[0];
    float *data_out = (float *) output_items[0];  // sample data out
    ms_plinfo   *attrib_out = (ms_plinfo *) output_items[1];    // attribute data out
    int t_count = 0;  // Count of samples over the threshold
    in += 1;	      // ensure that in[-1] is valid

    for (int i = 0; i < noutput_items; i++)
    {
	attrib_out[i].reset_all();  // No attributes to start
	data_out[i] = in[i]; // Direct Copy
	if(in[i] >= d_beta) // Only interested in samples at or above threshold
	{
		if(++t_count > d_width) // Check there are enough samples above threshold
		{
			int pos = i - d_width;  // rewind and set attributes
			attrib_out[pos].set_valid_pulse();
    			if((in[pos] >= (in[pos - 1] * d_alpha)) && (in[pos + 1] < (in[pos] * d_alpha)))
			{
				attrib_out[pos].set_leading_edge();
			}
		}
	}
	else
		t_count = 0;
    }
    return noutput_items-d_width;
}
