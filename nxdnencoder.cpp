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

#include "nxdnencoder.h"
#include <cstring>
const uint8_t NXDN_LICH_RFCT_RDCH			= 2U;
const uint8_t NXDN_LICH_USC_SACCH_NS		= 0U;
const uint8_t NXDN_LICH_USC_SACCH_SS		= 2U;
const uint8_t NXDN_LICH_STEAL_FACCH			= 0U;
const uint8_t NXDN_LICH_STEAL_NONE			= 3U;
const uint8_t NXDN_LICH_DIRECTION_INBOUND	= 0U;
const uint8_t NXDN_MESSAGE_TYPE_VCALL       = 1U;
const uint8_t NXDN_MESSAGE_TYPE_TX_REL      = 8U;

const unsigned char BIT_MASK_TABLE[] = { 0x80U, 0x40U, 0x20U, 0x10U, 0x08U, 0x04U, 0x02U, 0x01U };
#define WRITE_BIT1(p,i,b) p[(i)>>3] = (b) ? (p[(i)>>3] | BIT_MASK_TABLE[(i)&7]) : (p[(i)>>3] & ~BIT_MASK_TABLE[(i)&7])
#define READ_BIT1(p,i)    (p[(i)>>3] & BIT_MASK_TABLE[(i)&7])

NXDNEncoder::NXDNEncoder()
{
	m_nxdncnt = 0;
}

unsigned char * NXDNEncoder::get_frame(unsigned char *ambe)
{
	if(!m_nxdncnt){
		encode_header();
	}
	else{
		m_ambe = ambe;
		encode_data();
	}
	build_frame();
	++m_nxdncnt;
	return m_nxdnframe;
}

void NXDNEncoder::encode_header()
{
	const uint8_t idle[3U] = {0x10, 0x00, 0x00};
	m_lich = 0;
	memset(m_sacch, 0, 5U);
	memset(m_layer3, 0, 22U);
	set_lich_rfct(NXDN_LICH_RFCT_RDCH);
	set_lich_fct(NXDN_LICH_USC_SACCH_NS);
	set_lich_option(NXDN_LICH_STEAL_FACCH);
	set_lich_dir(NXDN_LICH_DIRECTION_INBOUND);
	m_nxdnframe[0U] = get_lich();

	set_sacch_ran(0x01);
	set_sacch_struct(0); //Single
	set_sacch_data(idle);
	get_sacch(&m_nxdnframe[1U]);

	set_layer3_msgtype(NXDN_MESSAGE_TYPE_VCALL);
	set_layer3_srcid(m_srcid);
	set_layer3_dstid(m_dstid);
	set_layer3_grp(true);
	set_layer3_blks(0U);
	memcpy(&m_nxdnframe[5U], m_layer3, 14U);
	memcpy(&m_nxdnframe[19U], m_layer3, 14U);
}

void NXDNEncoder::encode_data()
{
	uint8_t msg[3U];
	m_lich = 0;
	memset(m_sacch, 0, 5U);
	memset(m_layer3, 0, 22U);
	set_lich_rfct(NXDN_LICH_RFCT_RDCH);
	set_lich_fct(NXDN_LICH_USC_SACCH_SS);
	set_lich_option(NXDN_LICH_STEAL_NONE);
	set_lich_dir(NXDN_LICH_DIRECTION_INBOUND);
	m_nxdnframe[0U] = get_lich();

	set_layer3_msgtype(NXDN_MESSAGE_TYPE_VCALL);
	set_layer3_srcid(m_srcid);
	set_layer3_dstid(m_dstid);
	set_layer3_grp(true);
	set_layer3_blks(0U);

	switch(m_nxdncnt % 4){
	case 0:
		set_sacch_struct(3);
		layer3_encode(msg, 18U, 0U);
		set_sacch_data(msg);
		break;
	case 1:
		set_sacch_struct(2);
		layer3_encode(msg, 18U, 18U);
		set_sacch_data(msg);
		break;
	case 2:
		set_sacch_struct(1);
		layer3_encode(msg, 18U, 36U);
		set_sacch_data(msg);
		break;
	case 3:
		set_sacch_struct(0);
		layer3_encode(msg, 18U, 54U);
		set_sacch_data(msg);
		break;
	}
}

void NXDNEncoder::build_frame()
{

}

void NXDNEncoder::set_lich_rfct(uint8_t rfct)
{
	m_lich &= 0x3FU;
	m_lich |= (rfct << 6) & 0xC0U;
}

void NXDNEncoder::set_lich_fct(uint8_t fct)
{
	m_lich &= 0xCFU;
	m_lich |= (fct << 4) & 0x30U;
}

void NXDNEncoder::set_lich_option(uint8_t o)
{
	m_lich &= 0xF3U;
	m_lich |= (o << 2) & 0x0CU;
}

void NXDNEncoder::set_lich_dir(uint8_t d)
{
	m_lich &= 0xFDU;
	m_lich |= (d << 1) & 0x02U;
}

uint8_t NXDNEncoder::get_lich()
{
	bool parity;
	switch (m_lich & 0xF0U) {
	case 0x80U:
	case 0xB0U:
		parity = true;
	default:
		parity = false;
	}
	if (parity)
		m_lich |= 0x01U;
	else
		m_lich &= 0xFEU;

	return m_lich;
}


void NXDNEncoder::set_sacch_ran(uint8_t ran)
{
	m_sacch[0] &= 0xC0U;
	m_sacch[0] |= ran;
}

void NXDNEncoder::set_sacch_struct(uint8_t s)
{
	m_sacch[0] &= 0x3FU;
	m_sacch[0] |= (s << 6) & 0xC0U;;
}

void NXDNEncoder::set_sacch_data(const uint8_t *d)
{
	uint8_t offset = 8U;
	for (uint8_t i = 0U; i < 18U; i++, offset++) {
		bool b = READ_BIT1(d, i);
		WRITE_BIT1(m_sacch, offset, b);
	}
}

void NXDNEncoder::get_sacch(uint8_t *d)
{
	memcpy(d, m_sacch, 4U);
	encode_crc6(d, 26);
}

void NXDNEncoder::set_layer3_msgtype(uint8_t t)
{
	m_layer3[0] &= 0xC0U;
	m_layer3[0] |= t & 0x3FU;
}

void NXDNEncoder::set_layer3_srcid(uint16_t src)
{
	m_layer3[3U] = (src >> 8) & 0xFF;
	m_layer3[4U] = (src >> 0) & 0xFF ;
}

void NXDNEncoder::set_layer3_dstid(uint16_t dst)
{
	m_layer3[5U] = (dst >> 8) & 0xFF;
	m_layer3[6U] = (dst >> 0) & 0xFF ;
}

void NXDNEncoder::set_layer3_grp(bool grp)
{
	m_layer3[2U] |= grp ? 0x20U : 0x20U;
}

void NXDNEncoder::set_layer3_blks(uint8_t b)
{
	m_layer3[8U] &= 0xF0U;
	m_layer3[8U] |= b & 0x0FU;
}

void NXDNEncoder::layer3_encode(uint8_t* d, uint8_t len, uint8_t offset)
{
	for (uint32_t i = 0U; i < len; i++, offset++) {
		bool b = READ_BIT1(m_layer3, offset);
		WRITE_BIT1(d, i, b);
	}
}

void NXDNEncoder::encode_crc6(uint8_t *d, uint8_t len)
{
	uint8_t crc = 0x3FU;

	for (unsigned int i = 0U; i < len; i++) {
		bool bit1 = READ_BIT1(d, i) != 0x00U;
		bool bit2 = (crc & 0x20U) == 0x20U;
		crc <<= 1;

		if (bit1 ^ bit2)
			crc ^= 0x27U;
	}
	crc &= 0x3FU;
	uint8_t n = len;
	for (uint8_t i = 2U; i < 8U; i++, n++) {
		bool b = READ_BIT1((&crc), i);
		WRITE_BIT1(d, n, b);
	}
}
