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
   Format Mode S frames in text format for logging.  It is possible to read
   in the text log file and decode the frames by external programs

Some typical lines and description
008602b8                7be1c3  34.082 bd647bba 46897d33   0 00000001 00 a7502c
008182b8                a3dc20  36.069 bd64f551 46897d33   0 00000001 00 a7502c
8d4005a6 9904a194409f09 0cc9f2  33.959 bd6543dc 46897d33   0 00000001 17 000000
Short    Extended       PI      Ref Lv TS       Time     LCB EC Qual  Ty Addr

Short is the first 32 bits of the frame and represents the data portion of a short frame.

Extended is not present in a short frame and represnets the extended data portion of a long frame.

PI is the parity bits of both the short and long frames and represents the last 24 bits of a frame.

Ref Lv is the Reference Level used and can be used as a RSSI.

TS is the sample number.  It will roll over after a short period of time.  It represnets the preample
   start sample and may be useful to determine short time differences between Mode S Frames.

Time is the decode time using the unix time() function (Seconds since Jan 1, 1970)  It represents when
the whole frame was received and processed.  This may be useful for the standard 5 minute delay when sending
information on the internet

LCB is the number of Low Confidence Bits in the frame

EC Qual is the bits indicating the Error Correction result (See air_ms_types.h)

Ty is the first five bits of the frame in decimal format and represents the frame type.

Addr is the overlayed address, error syndrome, or both depending on the frame type and errors

A side effect of logging Mode S frames in a busy area is the megabytes of text data generated
in a short period of time.

*/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <air_ms_fmt_log.h>
#include <gr_io_signature.h>
#include <air_ms_types.h>
#include <ctype.h>
#include <iostream>

air_ms_fmt_log_sptr air_make_ms_fmt_log(int pass_all, gr_msg_queue_sptr queue)
{
    return air_ms_fmt_log_sptr(new air_ms_fmt_log(pass_all, queue));
}

air_ms_fmt_log::air_ms_fmt_log(int pass_all, gr_msg_queue_sptr queue) :
    gr_sync_block("ms_fmt_log",
    gr_make_io_signature(1, 1, sizeof(ms_frame_raw)),
    gr_make_io_signature(0, 0, 0)),
        d_queue(queue), d_pass_all(pass_all)
{
    d_count = 0;
}

int air_ms_fmt_log::work(int noutput_items,
    gr_vector_const_void_star &input_items,
    gr_vector_void_star &output_items)
{
    ms_frame_raw *data_in = (ms_frame_raw *)input_items[0];

    int i;
    for(i = 0;i < noutput_items; i++)
    {
        // If pass all or data good then send it out otherwise move on
        if(d_pass_all || (data_in[i].ec_quality() & (ms_frame_raw::crc_ok | ms_frame_raw::eq_ec_corrected)))
        {
            format_data(data_in[i]);
            d_count++;
        }
    }

    return i;
}


void air_ms_fmt_log::format_data(ms_frame_raw &frame)
{
    int i;
    int typecode = 0;
    int data = 0;
    int pi = 0;
    for (i = 0; i < 5; i++)
    {
       typecode = (typecode << 1) + frame.bit(i);
       data = (data << 1) + frame.bit(i);
    }
    for (i = 5; i < 32; i++)
    {
        data = (data << 1) + frame.bit(i);
    }

    d_payload.str("");
    d_payload.width(8);
    d_payload.fill('0');
    d_payload << std::hex <<  data << FIELD_DELIM;

    if(frame.length() >= MS_LONG_FRAME_LENGTH)
    {
        data = 0;
        d_payload.width(7);
        for (i = 32; i < 60; i++)
        {
             data = (data << 1) + frame.bit(i);
        }
        d_payload << data;
        data = 0;
        d_payload.width(7);
        for (i = 60; i < 88; i++)
        {
             data = (data << 1) + frame.bit(i);
        }
        d_payload << data << FIELD_DELIM;
        d_payload.width(6);
        for (i = 88; i < MS_LONG_FRAME_LENGTH; i++)
        {
             pi = (pi << 1) + frame.bit(i);
        }
        d_payload << pi << FIELD_DELIM;
    }
    else
    {
        for (i = 32; i < MS_SHORT_FRAME_LENGTH; i++)
        {
             pi = (pi << 1) + frame.bit(i);
        }
        d_payload << FIELD_DELIM << "             " << FIELD_DELIM;
        d_payload.width(6);
        d_payload << pi << FIELD_DELIM;
    }
    
    d_payload.width(7);
    d_payload.precision(5);
    d_payload.fill(' ');
    d_payload << std::dec << frame.reference() << FIELD_DELIM;
    
    d_payload.width(8);
    d_payload.fill('0');
    d_payload << std::hex << frame.timestamp() << FIELD_DELIM << frame.rx_time() << FIELD_DELIM;
    
    d_payload.width(3);
    d_payload.fill(' ');
    d_payload << std::dec << frame.lcb_count() << FIELD_DELIM;
    
    d_payload.width(8);
    d_payload.fill('0');
    d_payload << std::hex << frame.ec_quality() << FIELD_DELIM;
    
    d_payload.width(2);
    d_payload << std::dec << typecode << FIELD_DELIM;
    
    d_payload.width(6);
    d_payload << std::hex << frame.address();

    gr_message_sptr msg = gr_make_message_from_string(std::string(d_payload.str()));
    d_queue->handle(msg);
}

