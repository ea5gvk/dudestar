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

#include "ysfenc.h"

#include "YSFConvolution.h"
#include "CRCenc.h"
#include "Golay24128.h"
#include "vocoder_tables.h"

#include <iostream>
#include <cstring>

#define DEBUG

const int vd2DVSIInterleave[49] = {
		0, 3, 6,  9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 41, 43, 45, 47,
		1, 4, 7, 10, 13, 16, 19, 22, 25, 28, 31, 34, 37, 40, 42, 44, 46, 48,
		2, 5, 8, 11, 14, 17, 20, 23, 26, 29, 32, 35, 38
};

const int vd2DVSIDEInterleave[49] = {
		0, 18, 36,  1, 19, 37, 2, 20, 38, 3, 21, 39, 4, 22, 40, 5, 23, 41,
		6, 24, 42, 7, 25, 43, 8, 26, 44, 9, 27, 45, 10, 28, 46, 11, 29, 47,
		12, 30, 48, 13, 31, 14, 32, 15, 33, 16, 34, 17, 35
};

const char ysf_radioid[] = {'H', '5', '0', '0', '0'};

YSFEncoder::YSFEncoder()
{
	ysf_cnt = 0;
	::memcpy(gateway, "AD8DP     ", 10);
	::memcpy(callsign, "AD8DP     ", 10);
	::memcpy(callsign_full, "AD8DP     ", 10);
}

YSFEncoder::~YSFEncoder()
{
}

unsigned char * YSFEncoder::get_frame(unsigned char *ambe)
{
	if(!ysf_cnt){
		encode_header();
	}
	else{
		m_ambe = ambe;
		encode_dv2();
	}
	++ysf_cnt;
	return m_ysfFrame;
}

unsigned char * YSFEncoder::get_eot()
{
	encode_header(1);
	ysf_cnt = 0;
	return m_ysfFrame;
}

void YSFEncoder::encode_header(bool eot)
{
	::memcpy(m_ysfFrame + 0U, "YSFD", 4U);
	::memcpy(m_ysfFrame + 4U, callsign, YSF_CALLSIGN_LENGTH);
	::memcpy(m_ysfFrame + 14U, callsign, YSF_CALLSIGN_LENGTH);
	::memcpy(m_ysfFrame + 24U, "ALL       ", YSF_CALLSIGN_LENGTH);

	if(eot){
		m_ysfFrame[34U] = ((ysf_cnt & 0x7f) << 1U) | 1U;
	}
	else{
		m_ysfFrame[34U] = 0U;
	}
	::memcpy(m_ysfFrame + 35U, YSF_SYNC_BYTES, 5);
	
	fich.setFI(eot ? YSF_FI_TERMINATOR : YSF_FI_HEADER);
	fich.setCS(2U);
	fich.setCM(0U);
	fich.setBN(0U);
	fich.setBT(0U);
	fich.setFN(0U);
	fich.setFT(6U);
	fich.setDev(0U);
	fich.setMR(0U); // Must be 2 for Wires-X?
	fich.setVoIP(false);
	fich.setDT(YSF_DT_VD_MODE2);
	fich.setSQL(false);
	fich.setSQ(0U);
	fich.encode(m_ysfFrame + 35U);

	unsigned char csd1[20U], csd2[20U];
	memset(csd1, '*', YSF_CALLSIGN_LENGTH);
	//memset(csd1, '*', YSF_CALLSIGN_LENGTH/2);
	//memcpy(csd1 + YSF_CALLSIGN_LENGTH/2, ysf_radioid, YSF_CALLSIGN_LENGTH/2);
	memcpy(csd1 + YSF_CALLSIGN_LENGTH, callsign, YSF_CALLSIGN_LENGTH);
	memcpy(csd2, callsign, YSF_CALLSIGN_LENGTH);
	memcpy(csd2 + YSF_CALLSIGN_LENGTH, callsign, YSF_CALLSIGN_LENGTH);
	//memset(csd2, ' ', YSF_CALLSIGN_LENGTH + YSF_CALLSIGN_LENGTH);

	writeDataFRModeData1(csd1, m_ysfFrame + 35U);
	writeDataFRModeData2(csd2, m_ysfFrame + 35U);
}

void YSFEncoder::encode_dv2()
{
	::memcpy(m_ysfFrame + 0U, "YSFD", 4U);
	::memcpy(m_ysfFrame + 4U, callsign, YSF_CALLSIGN_LENGTH);
	::memcpy(m_ysfFrame + 14U, callsign, YSF_CALLSIGN_LENGTH);
	::memcpy(m_ysfFrame + 24U, "ALL       ", YSF_CALLSIGN_LENGTH);
	m_ysfFrame[34U] = (ysf_cnt & 0x7f) << 1;
	::memcpy(m_ysfFrame + 35U, YSF_SYNC_BYTES, 5);
	unsigned int fn = (ysf_cnt - 1U) % 7U;

	fich.setFI(YSF_FI_COMMUNICATIONS);
	fich.setCS(2U);
	fich.setCM(0U);
	fich.setBN(0U);
	fich.setBT(0U);
	fich.setFN(fn);
	fich.setFT(6U);
	fich.setDev(0U);
	fich.setMR(0U); // 2/false for GM repeater
	fich.setVoIP(false);
	fich.setDT(YSF_DT_VD_MODE2);
	fich.setSQL(false);
	fich.setSQ(0U);
	fich.encode(m_ysfFrame + 35U);

	unsigned char dch[10U];

	//const uint8_t mys1[10] = {0x67, 0x22, 0x61, 0x5f, 0x2b, 0x03, 0x77, 0x00, 0x00, 0x00};
	//const uint8_t mys2[10] = {0x15, 0x22, 0x61, 0x5f, 0x28, 0x03, 0x22, 0x00, 0x00, 0x00};
	//const uint8_t mys3[10] = {0x07, 0x22, 0x61, 0x5f, 0x2b, 0x03, 0x17, 0x00, 0x00, 0x00};
	//const uint8_t mys4[10] = {0x8b, 0x22, 0x61, 0x5f, 0x25, 0x03, 0x95, 0x00, 0x00, 0x00};
	//const uint8_t mys5[10] = {0x13, 0x22, 0x61, 0x5f, 0x2a, 0x03, 0x22, 0x00, 0x00, 0x00};

	const uint8_t ft70d1[10] = {0x01, 0x22, 0x61, 0x5f, 0x2b, 0x03, 0x11, 0x00, 0x00, 0x00}; // 2019-09-10 10:50 power on
	//const uint8_t frame_6[10] = {0x30, 0x22, 0x61, 0x5f, 0x28, 0x03, 0x3d, 0x00, 0x00, 0x00}; // Openspot2 DMR2YSF W8WAC
	//const uint8_t gps1[10] = {0x16, 0x22, 0x62, 0x5f, 0x25, 0x34, 0x54, 0x34, 0x53, 0x54};
	//const uint8_t gps2[10] = {0x52, 0x33, 0x58, 0x38, 0x6c, 0x20, 0x3a, 0x20, 0x03, 0x9f};

	//const uint8_t gps3[10] = {0x03, 0x22, 0x62, 0x5f, 0x27, 0x54, 0x34, 0x50, 0x57, 0x32};
	//const uint8_t gps4[10] = {0x56, 0x70, 0x35, 0x3a, 0x6c, 0x20, 0x1c, 0x20, 0x03, 0x6e};

	//const uint8_t dt1_temp[] = {0x31, 0x22, 0x62, 0x5F, 0x29, 0x00, 0x00, 0x00, 0x00, 0x00};
	const uint8_t dt2_temp[] = {0x00, 0x00, 0x00, 0x00, 0x6C, 0x20, 0x1C, 0x20, 0x03, 0x08};

	switch (fn) {
	case 0:
		//memset(dch, '*', YSF_CALLSIGN_LENGTH/2);
		//memcpy(dch + YSF_CALLSIGN_LENGTH/2, ysf_radioid, YSF_CALLSIGN_LENGTH/2);
		writeVDMode2Data(m_ysfFrame + 35U, (const unsigned char*)"**********");
		//writeVDMode2Data(m_ysfFrame + 35U, dch);	//Dest
		break;
	case 1:
		writeVDMode2Data(m_ysfFrame + 35U, (const unsigned char*)callsign_full);		//Src
		break;
	case 2:
		writeVDMode2Data(m_ysfFrame + 35U, (const unsigned char*)callsign);	//D/L
		//writeVDMode2Data(m_ysfFrame + 35U, (const unsigned char*)"KD8GRN    ");	//D/L
		break;
	case 3:
		writeVDMode2Data(m_ysfFrame + 35U, (const unsigned char*)callsign);	//U/L
		//writeVDMode2Data(m_ysfFrame + 35U, (const unsigned char*)"N8YMT/R   ");	//U/L
		break;
	case 4:
		writeVDMode2Data(m_ysfFrame + 35U, (const unsigned char*)"          ");	//Rem1/2
		break;
	case 5:
		writeVDMode2Data(m_ysfFrame + 35U, (const unsigned char*)"          ");	//Rem3/4
		//memset(dch, ' ', YSF_CALLSIGN_LENGTH/2);
		//memcpy(dch + YSF_CALLSIGN_LENGTH/2, ysf_radioid, YSF_CALLSIGN_LENGTH/2);
		//writeVDMode2Data(m_ysfFrame + 35U, dch);	// Rem3/4
		break;
	case 6:
		//for (unsigned int i = 0U; i < 5U; i++) {
		//	::memcpy(m_ysfFrame + 65U + (18U*i), &vd2_dch[fn][i*5], 5U);
		//	AMBE2YSF(&m_ambe[9*i]);
		//	::memcpy(m_ysfFrame + 70U + (18U*i), m_vch, 13U);
		//}
		writeVDMode2Data(m_ysfFrame + 35U, ft70d1);
		break;
	case 7:
		writeVDMode2Data(m_ysfFrame + 35U, dt2_temp);
		break;
	default:
		writeVDMode2Data(m_ysfFrame + 35U, (const unsigned char*)"          ");
	}
}

void YSFEncoder::AMBE2YSF(unsigned char *data)
{
	unsigned char vch[13U];
	unsigned char ysfFrame[13U];
	::memset(vch, 0U, 13U);
	::memset(ysfFrame, 0, 13U);
	unsigned int a = 0U, b = 0U, c = 0U;

	unsigned int MASK = 0x800000U;
	for (unsigned int i = 0U; i < 24U; i++, MASK >>= 1) {
		unsigned int a1Pos = DMR_A_TABLE[i];

		if (READ_BIT(data, a1Pos))
			a |= MASK;

	}

	MASK = 0x400000U;
	for (unsigned int i = 0U; i < 23U; i++, MASK >>= 1) {
		unsigned int b1Pos = DMR_B_TABLE[i];

		if (READ_BIT(data, b1Pos))
			b |= MASK;
	}

	MASK = 0x1000000U;
	for (unsigned int i = 0U; i < 25U; i++, MASK >>= 1) {
		unsigned int c1Pos = DMR_C_TABLE[i];

		if (READ_BIT(data, c1Pos))
			c |= MASK;
	}

	unsigned int dat_a = a >> 12;

	// The PRNG
	b ^= (PRNG_TABLE[dat_a] >> 1);

	unsigned int dat_b = b >> 11;

	for (unsigned int i = 0U; i < 12U; i++) {
		bool s = (dat_a << (20U + i)) & 0x80000000U;
		WRITE_BIT(vch, 3*i + 0U, s);
		WRITE_BIT(vch, 3*i + 1U, s);
		WRITE_BIT(vch, 3*i + 2U, s);
	}
	
	for (unsigned int i = 0U; i < 12U; i++) {
		bool s = (dat_b << (20U + i)) & 0x80000000U;
		WRITE_BIT(vch, 3*(i + 12U) + 0U, s);
		WRITE_BIT(vch, 3*(i + 12U) + 1U, s);
		WRITE_BIT(vch, 3*(i + 12U) + 2U, s);
	}
	
	for (unsigned int i = 0U; i < 3U; i++) {
		bool s = (c << (7U + i)) & 0x80000000U;
		WRITE_BIT(vch, 3*(i + 24U) + 0U, s);
		WRITE_BIT(vch, 3*(i + 24U) + 1U, s);
		WRITE_BIT(vch, 3*(i + 24U) + 2U, s);
	}

	for (unsigned int i = 0U; i < 22U; i++) {
		bool s = (c << (10U + i)) & 0x80000000U;
		WRITE_BIT(vch, i + 81U, s);
	}
	
	WRITE_BIT(vch, 103U, 0U);

	// Scramble
	for (unsigned int i = 0U; i < 13U; i++)
		vch[i] ^= WHITENING_DATA[i];

	// Interleave
	for (unsigned int i = 0U; i < 104U; i++) {
		unsigned int n = INTERLEAVE_TABLE_26_4[i];
		bool s = READ_BIT(vch, i);
		WRITE_BIT(ysfFrame, n, s);
	}
	::memcpy(m_vch, ysfFrame, 13);
}


void YSFEncoder::writeDataFRModeData1(const unsigned char* dt, unsigned char* data)
{
	//assert(dt != NULL);
	//assert(data != NULL);

	data += YSF_SYNC_LENGTH_BYTES + YSF_FICH_LENGTH_BYTES;

	unsigned char output[25U];
	for (unsigned int i = 0U; i < 20U; i++)
		output[i] = dt[i] ^ WHITENING_DATA[i];

	CCRC::addCCITT162(output, 22U);
	output[22U] = 0x00U;

	unsigned char convolved[45U];

	CYSFConvolution conv;
	conv.encode(output, convolved, 180U);

	unsigned char bytes[45U];
	unsigned int j = 0U;
	for (unsigned int i = 0U; i < 180U; i++) {
		unsigned int n = INTERLEAVE_TABLE_9_20[i];

		bool s0 = READ_BIT(convolved, j) != 0U;
		j++;

		bool s1 = READ_BIT(convolved, j) != 0U;
		j++;

		WRITE_BIT(bytes, n, s0);

		n++;
		WRITE_BIT(bytes, n, s1);
	}

	unsigned char* p1 = data;
	unsigned char* p2 = bytes;
	for (unsigned int i = 0U; i < 5U; i++) {
		::memcpy(p1, p2, 9U);
		p1 += 18U; p2 += 9U;
	}
}

void YSFEncoder::writeDataFRModeData2(const unsigned char* dt, unsigned char* data)
{
	data += YSF_SYNC_LENGTH_BYTES + YSF_FICH_LENGTH_BYTES;

	unsigned char output[25U];
	for (unsigned int i = 0U; i < 20U; i++)
		output[i] = dt[i] ^ WHITENING_DATA[i];

	CCRC::addCCITT162(output, 22U);
	output[22U] = 0x00U;

	unsigned char convolved[45U];

	CYSFConvolution conv;
	conv.encode(output, convolved, 180U);

	unsigned char bytes[45U];
	unsigned int j = 0U;
	for (unsigned int i = 0U; i < 180U; i++) {
		unsigned int n = INTERLEAVE_TABLE_9_20[i];

		bool s0 = READ_BIT(convolved, j) != 0U;
		j++;

		bool s1 = READ_BIT(convolved, j) != 0U;
		j++;

		WRITE_BIT(bytes, n, s0);

		n++;
		WRITE_BIT(bytes, n, s1);
	}

	unsigned char* p1 = data + 9U;
	unsigned char* p2 = bytes;
	for (unsigned int i = 0U; i < 5U; i++) {
		::memcpy(p1, p2, 9U);
		p1 += 18U; p2 += 9U;
	}
}

void YSFEncoder::ysf_scramble(uint8_t *buf, const int len)
{	// buffer is (de)scrambled in place
	static const uint8_t scramble_code[180] = {
	1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1,
	0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1,
	1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0,
	1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1,
	1, 1, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 1, 1,
	0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0,
	1, 1, 1, 0, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0,
	0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0
	};

	for (int i=0; i<len; i++) {
		buf[i] = buf[i] ^ scramble_code[i];
	}
}

void YSFEncoder::generate_vch_vd2(const uint8_t *a)
{
	uint8_t buf[104];
	uint8_t result[104];
	//unsigned char a[56];
	uint8_t vch[13];
	memset(vch, 0, 13);
/*
	for(int i = 0; i < 7; ++i){
		for(int j = 0; j < 8; ++j){
			a[(8*i)+j] = (1 & (input[i] >> (7-j)));
			//a[((8*i)+j)+1] = (1 & (data[5-i] >> j));
		}
	}
*/
	for (int i=0; i<27; i++) {
		buf[0+i*3] = a[i];
		buf[1+i*3] = a[i];
		buf[2+i*3] = a[i];
	}
	memcpy(buf+81, a+27, 22);
	buf[103] = 0;
	ysf_scramble(buf, 104);

	//uint8_t bit_result[104];
	int x=4;
	int y=26;
	for (int i=0; i<x; i++) {
		for (int j=0; j<y; j++) {
			result[i+j*x] = buf[j+i*y];
		}
	}
	for(int i = 0; i < 13; ++i){
		for(int j = 0; j < 8; ++j){
			//ambe_bytes[i] |= (ambe_frame[((8-i)*8)+(7-j)] << (7-j));
			vch[i] |= (result[(i*8)+j] << (7-j));
		}
	}
	::memcpy(m_vch, vch, 13);
}

void YSFEncoder::writeVDMode2Data(unsigned char* data, const unsigned char* dt)
{
	data += YSF_SYNC_LENGTH_BYTES + YSF_FICH_LENGTH_BYTES;
	
	unsigned char dt_tmp[13];
	::memcpy(dt_tmp, dt, YSF_CALLSIGN_LENGTH);

	for (unsigned int i = 0U; i < 10U; i++)
		dt_tmp[i] ^= WHITENING_DATA[i];

	CCRC::addCCITT162(dt_tmp, 12U);
	dt_tmp[12U] = 0x00U;

	unsigned char convolved[25U];
	CYSFConvolution conv;
	conv.start();
	conv.encode(dt_tmp, convolved, 100U);

	unsigned char bytes[25U];
	unsigned int j = 0U;
	for (unsigned int i = 0U; i < 100U; i++) {
		unsigned int n = INTERLEAVE_TABLE_5_20[i];

		bool s0 = READ_BIT(convolved, j) != 0U;
		j++;

		bool s1 = READ_BIT(convolved, j) != 0U;
		j++;

		WRITE_BIT(bytes, n, s0);

		n++;
		WRITE_BIT(bytes, n, s1);
	}

	unsigned char* p1 = data;
	unsigned char* p2 = bytes;
#ifdef DEBUG
	fprintf(stderr, "AMBE: ");
	for(int i = 0; i < 45; ++i){
		fprintf(stderr, "%02x ", m_ambe[i]);
	}
	fprintf(stderr, "\n");
	fflush(stderr);
#endif
	for (unsigned int i = 0U; i < 5U; i++) {
		::memcpy(p1, p2, 5U);
		if(use_hw){
			char ambe_bits[56];
			unsigned char di_bits[56];
			unsigned char *d = &m_ambe[9*i];
			for(int ii = 0; ii < 7; ++ii){
				for(int j = 0; j < 8; j++){
					ambe_bits[j+(8*ii)] = (1 & (d[ii] >> (7 - j)));
					//ambe_bits[j+(8*ii)] = (1 & (d[ii] >> j));
				}
			}
			for(int ii = 0; ii < 49; ++ii){
				di_bits[ii] = ambe_bits[vd2DVSIInterleave[ii]];
			}
			generate_vch_vd2(di_bits);
			//AMBE2YSF(&m_ambe[9*i]);
		}
		else{
			unsigned char a[56];
			unsigned char *d = &m_ambe[9*i];
			for(int ii = 0; ii < 7; ++ii){
				for(int j = 0; j < 8; ++j){
					a[(8*ii)+j] = (1 & (d[ii] >> (7-j)));
					//a[((8*i)+j)+1] = (1 & (data[5-i] >> j));
				}
			}
			//generate_vch_vd2(&m_ambe[9*i]);
			generate_vch_vd2(a);
		}
		::memcpy(p1+5, m_vch, 13);
		p1 += 18U; p2 += 5U;
	}
}
