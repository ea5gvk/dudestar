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
#ifndef NXDNENCODER_H
#define NXDNENCODER_H
#include <inttypes.h>

class NXDNEncoder
{
public:
	NXDNEncoder();
	unsigned char * get_frame(unsigned char *ambe);
	unsigned char * get_eot();
private:
	uint8_t m_nxdnframe[55];
	uint32_t m_nxdncnt;
	uint16_t m_srcid;
	uint16_t m_dstid;
	uint8_t m_lich;
	uint8_t m_sacch[5];
	uint8_t m_layer3[22];
	unsigned char *m_ambe;

	void build_frame();
	void encode_header();
	void encode_data();
	void set_lich_rfct(uint8_t);
	void set_lich_fct(uint8_t);
	void set_lich_option(uint8_t);
	void set_lich_dir(uint8_t);
	void set_sacch_ran(uint8_t);
	void set_sacch_struct(uint8_t);
	void set_sacch_data(const uint8_t *);
	void set_layer3_msgtype(uint8_t);
	void set_layer3_srcid(uint16_t);
	void set_layer3_dstid(uint16_t);
	void set_layer3_grp(bool);
	void set_layer3_blks(uint8_t);
	void layer3_encode(uint8_t*, uint8_t, uint8_t);

	uint8_t get_lich();
	void get_sacch(uint8_t *);
	void encode_crc6(uint8_t *, uint8_t);
};

#endif // NXDNENCODER_H
