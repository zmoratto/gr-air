/*
 * Copyright 2006 Free Software Foundation, Inc.
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

#ifndef INCLUDED_AIR_MS_PARITY_H
#define INCLUDED_AIR_MS_PARITY_H

#include <gr_sync_block.h>

class air_ms_parity;
typedef boost::shared_ptr<air_ms_parity> air_ms_parity_sptr;

air_ms_parity_sptr air_make_ms_parity();

/*!
 * \brief Mode Select Parity Check
 * \ingroup block
 */

class air_ms_parity : public gr_sync_block
{
private:
    // Constructors
    friend air_ms_parity_sptr air_make_ms_parity();
    air_ms_parity();

public:
    int work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items);
};

#endif /* INCLUDED_AIR_MS_PARITY_H */
