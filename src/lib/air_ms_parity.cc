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

#include <air_ms_parity.h>
#include <airi_ms_parity.h>
#include <gr_io_signature.h>
#include <air_ms_types.h>

air_ms_parity_sptr air_make_ms_parity()
{
    return air_ms_parity_sptr(new air_ms_parity());
}

air_ms_parity::air_ms_parity() :
    gr_sync_block("ms_parity",
	gr_make_io_signature(1, 1, sizeof(ms_frame_raw)),
	gr_make_io_signature(1, 1, sizeof(ms_frame_raw)))
{
}

int air_ms_parity::work(int noutput_items,
    gr_vector_const_void_star &input_items,
    gr_vector_void_star &output_items)
{
    ms_frame_raw *data_in = (ms_frame_raw *)input_items[0];
    ms_frame_raw *data_out = (ms_frame_raw *)output_items[0];

    int crc = 0;
    int i, j;
    for (i = 0; i < noutput_items; i++) {
	data_out[i] = data_in[i];
        data_out[i].count_lcbs();
	crc = ms_check_parity(data_out[i]);
	data_out[i].set_address(crc);  // assume error syndrome is address overlay
	// If the crc is good then we are done.  ACAS/TCAS Frames have a address of zero.
	if(crc == 0)
	{
		data_out[i].set_ec_quality(ms_frame_raw::crc_ok);
	}
	else // if things are not good then do a few checks
	{
		if(data_out[i].length() == MS_LONG_FRAME_LENGTH)
		{
			//See if the frame should be a short one so check if all low energy bits are in second half
			if ((data_out[i].lcb_count() >= MS_SHORT_FRAME_LENGTH) && (data_out[i].first_leb() >= MS_SHORT_FRAME_LENGTH))
			{
				data_out[i].set_short_frame();
				crc = ms_check_parity(data_out[i]);
				data_out[i].set_address(crc);  // assume error syndrome is address overlay
                                data_out[i].count_lcbs();
				if(crc == 0)  // address is zero (ACAS/TCAS)
				{
					data_out[i].set_ec_quality((ms_frame_raw::crc_ok|ms_frame_raw::eq_change_short_frame));
				}
				else if(data_out[i].lcb_count())
				{
					data_out[i].set_ec_quality((ms_frame_raw::crc_bad|ms_frame_raw::eq_change_short_frame));
				}
				else // assume it is a crc with address overlay
				{
					data_out[i].set_ec_quality((ms_frame_raw::crc_ok|ms_frame_raw::eq_change_short_frame));
				}
				continue;
			}
			else
			{
					// Check for a too short of frame  If no parity at all say it is a short frame
					data_out[i].set_address(crc);
					for(j = MS_LONG_FRAME_LENGTH - 1; j > (MS_LONG_FRAME_LENGTH - 25); j--)
					{
						if(data_out[i].flags(j) == ms_frame_raw::fl_high_confidence)
							break;
					}
					if(j == (MS_LONG_FRAME_LENGTH - 25))
					{
						data_out[i].set_ec_quality(ms_frame_raw::eq_too_short_frame);
						continue;

					}
			}
		}
		else
		{
			// Check for a too short of frame  If no parity at all say it is a short frame
			data_out[i].set_address(crc);
			for(j = MS_SHORT_FRAME_LENGTH - 1; j > (MS_SHORT_FRAME_LENGTH - 25); j--)
			{
				if(data_out[i].flags(j) == ms_frame_raw::fl_high_confidence)
					break;
			}
			if(j == (MS_SHORT_FRAME_LENGTH - 25))
			{
				data_out[i].set_ec_quality(ms_frame_raw::eq_too_short_frame);
				continue;
			}
		}
		// If low confidence bits then it is bad
		if(data_out[i].lcb_count())
		{
			data_out[i].set_ec_quality(ms_frame_raw::crc_bad);
		}
		else // assume it is a crc with address overlay
		{
			data_out[i].set_ec_quality(ms_frame_raw::crc_ok);
		}
	}
    }
    return i;
}
