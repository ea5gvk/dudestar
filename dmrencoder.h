/*
	Copyright (C) 2019 Doug McLain

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef DMRENCODER_H
#define DMRENCODER_H

#include <inttypes.h>
#include "DMRData.h"
#include "cbptc19696.h"

class DMREncoder
{
public:
	DMREncoder();
	unsigned char * get_frame(unsigned char *ambe);
	unsigned char * get_eot();
	void set_srcid(uint32_t s){m_srcid = s;}
	void set_dstid(uint32_t d){m_dstid = d;}
private:
	uint8_t m_dmrFrame[55];
	uint8_t m_dataType;
	uint32_t m_colorcode;
	uint32_t m_srcid;
	uint32_t m_defsrcid;
	uint32_t m_dstid;
	uint32_t m_dmrcnt;
	FLCO m_dmrflco;
	CBPTC19696 m_bptc;
	bool m_raw[128U];
	bool m_data[72U];
	unsigned char *m_ambe;

	void byteToBitsBE(uint8_t byte, bool* bits);
	void bitsToByteBE(const bool* bits, uint8_t& byte);

	void build_frame();
	void encode_header();
	void encode_data();
	void encode16114(bool* d);
	void encode_qr1676(uint8_t* data);
	void get_slot_data(uint8_t* data);
	void lc_get_data(uint8_t*);
	void lc_get_data(bool* bits);
	void encode_embedded_data();
	uint8_t get_embedded_data(uint8_t* data, uint8_t n);
	void get_emb_data(uint8_t* data, uint8_t lcss);
	void full_lc_encode(uint8_t* data, uint8_t type);
	void addDMRDataSync(uint8_t* data, bool duplex);
	void addDMRAudioSync(uint8_t* data, bool duplex);
};

#endif // DMRENCODER_H
