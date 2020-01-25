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

#include <iostream>
#include <cstring>
#include "p25encoder.h"
#include "mbefec.h"

const int iW[72] = {
  0, 2, 4, 1, 3, 5,
  0, 2, 4, 1, 3, 6,
  0, 2, 4, 1, 3, 6,
  0, 2, 4, 1, 3, 6,
  0, 2, 4, 1, 3, 6,
  0, 2, 4, 1, 3, 6,
  0, 2, 5, 1, 3, 6,
  0, 2, 5, 1, 3, 6,
  0, 2, 5, 1, 3, 7,
  0, 2, 5, 1, 3, 7,
  0, 2, 5, 1, 4, 7,
  0, 3, 5, 2, 4, 7
};

const int iX[72] = {
  22, 20, 10, 20, 18, 0,
  20, 18, 8, 18, 16, 13,
  18, 16, 6, 16, 14, 11,
  16, 14, 4, 14, 12, 9,
  14, 12, 2, 12, 10, 7,
  12, 10, 0, 10, 8, 5,
  10, 8, 13, 8, 6, 3,
  8, 6, 11, 6, 4, 1,
  6, 4, 9, 4, 2, 6,
  4, 2, 7, 2, 0, 4,
  2, 0, 5, 0, 13, 2,
  0, 21, 3, 21, 11, 0
};

const int iY[72] = {
  1, 3, 5, 0, 2, 4,
  1, 3, 6, 0, 2, 4,
  1, 3, 6, 0, 2, 4,
  1, 3, 6, 0, 2, 4,
  1, 3, 6, 0, 2, 4,
  1, 3, 6, 0, 2, 5,
  1, 3, 6, 0, 2, 5,
  1, 3, 6, 0, 2, 5,
  1, 3, 6, 0, 2, 5,
  1, 3, 7, 0, 2, 5,
  1, 4, 7, 0, 3, 5,
  2, 4, 7, 1, 3, 5
};

const int iZ[72] = {
  21, 19, 1, 21, 19, 9,
  19, 17, 14, 19, 17, 7,
  17, 15, 12, 17, 15, 5,
  15, 13, 10, 15, 13, 3,
  13, 11, 8, 13, 11, 1,
  11, 9, 6, 11, 9, 14,
  9, 7, 4, 9, 7, 12,
  7, 5, 2, 7, 5, 10,
  5, 3, 0, 5, 3, 8,
  3, 1, 5, 3, 1, 6,
  1, 14, 3, 1, 22, 4,
  22, 12, 1, 22, 20, 2
};

const unsigned int IMBE_INTERLEAVE[] = {
	0,  7, 12, 19, 24, 31, 36, 43, 48, 55, 60, 67, 72, 79, 84, 91,  96, 103, 108, 115, 120, 127, 132, 139,
	1,  6, 13, 18, 25, 30, 37, 42, 49, 54, 61, 66, 73, 78, 85, 90,  97, 102, 109, 114, 121, 126, 133, 138,
	2,  9, 14, 21, 26, 33, 38, 45, 50, 57, 62, 69, 74, 81, 86, 93,  98, 105, 110, 117, 122, 129, 134, 141,
	3,  8, 15, 20, 27, 32, 39, 44, 51, 56, 63, 68, 75, 80, 87, 92,  99, 104, 111, 116, 123, 128, 135, 140,
	4, 11, 16, 23, 28, 35, 40, 47, 52, 59, 64, 71, 76, 83, 88, 95, 100, 107, 112, 119, 124, 131, 136, 143,
	5, 10, 17, 22, 29, 34, 41, 46, 53, 58, 65, 70, 77, 82, 89, 94, 101, 106, 113, 118, 125, 130, 137, 142};

const unsigned char BIT_MASK_TABLE[] = {0x80U, 0x40U, 0x20U, 0x10U, 0x08U, 0x04U, 0x02U, 0x01U};

#define WRITE_BIT(p,i,b) p[(i)>>3] = (b) ? (p[(i)>>3] | BIT_MASK_TABLE[(i)&7]) : (p[(i)>>3] & ~BIT_MASK_TABLE[(i)&7])
#define READ_BIT(p,i)    (p[(i)>>3] & BIT_MASK_TABLE[(i)&7])

p25encoder::p25encoder()
{
	m_p25cnt = 0;
}

unsigned char * p25encoder::get_frame(unsigned char *ambe)
{
	decode(ambe);
	//extract_voice_bits(ambe, imbe);
	return imbe;
}

void p25encoder::extract_voice_bits(const unsigned char* d, unsigned char* pimbe)
{
	int i, j, k, errs;
	unsigned short pr[115];
	unsigned short foo;
	char tmpstr[24];
	char imbe_fr[8][23];
	unsigned char in[23], out[23], gin[23], gout[23], hin[15], hout[15];
	const int *w, *x, *y, *z;
	::memset(imbe, 0, 11);
	pimbe = imbe;

	w = iW;
	x = iX;
	y = iY;
	z = iZ;

	for(i = 0; i < 18; ++i){
		for(j = 0; j < 8; j+=2){
			//imbe_fr[*y][*z] = (1 & (d[i] >> (j+1)));
			//imbe_fr[*w][*x] = (1 & (d[i] >> j));
			imbe_fr[*y][*z] = (1 & (d[i] >> (7 - (j+1))));
			imbe_fr[*w][*x] = (1 & (d[i] >> (7 - j)));
			w++;
			x++;
			y++;
			z++;
		}
	}
	//eccImbe7200x4400C0
	for (j = 0; j < 23; j++){
		in[j] = imbe_fr[0][j];
	}
	errs = GolayMBE::mbe_golay2312 (in, out);
	for (j = 0; j < 23; j++){
		imbe_fr[0][j] = out[j];
	}
	//emodulateImbe7200x4400Data
	j = 0;
	tmpstr[12] = 0;
	for (i = 22; i >= 11; i--){
		tmpstr[j] = (imbe_fr[0][i] + 48);
		j++;
	}
	foo = strtol (tmpstr, NULL, 2);
	pr[0] = (16 * foo);
	for (i = 1; i < 115; i++){
		pr[i] = (173 * pr[i - 1]) + 13849 - (65536 * (((173 * pr[i - 1]) + 13849) / 65536));
	}
	for (i = 1; i < 115; i++){
		pr[i] = pr[i] / 32768;
	}

	// demodulate imbe with pr
	k = 1;
	for (i = 1; i < 4; i++){
		for (j = 22; j >= 0; j--){
			imbe_fr[i][j] = ((imbe_fr[i][j]) ^ pr[k]);
			k++;
		}
	}
	for (i = 4; i < 7; i++){
		for (j = 14; j >= 0; j--){
			imbe_fr[i][j] = ((imbe_fr[i][j]) ^ pr[k]);
			k++;
		}
	}
	//eccImbe7200x4400Data
	for (i = 0; i < 4; i++){
		if (i > 0){
			for (j = 0; j < 23; j++){
				gin[j] = imbe_fr[i][j];
			}
			errs += GolayMBE::mbe_golay2312 (gin, gout);
			for (j = 22; j > 10; j--){
				*pimbe = gout[j];
				pimbe++;
			}
		}
		else{
			for (j = 22; j > 10; j--){
				*pimbe = imbe_fr[i][j];
				pimbe++;
			}
		}
	}
	for (i = 4; i < 7; i++){
		for (j = 0; j < 15; j++){
			hin[j] = imbe_fr[i][j];
		}
		errs += HammingMBE::mbe_hamming1511 (hin, hout);
		for (j = 14; j >= 4; j--){
			*pimbe = hout[j];
			pimbe++;
		}
	}
	for (j = 6; j >= 0; j--){
		*pimbe = imbe_fr[7][j];
		pimbe++;
	}
}

void p25encoder::decode(const unsigned char* data)
{
	bool bit[144U];
	::memset(imbe, 0, 11);
/*
	fprintf(stderr, "AMBERX : ");
	for(int i = 0; i < 18; ++i){
		fprintf(stderr, "%02x ", data[i]);
	}
	fprintf(stderr, "\n");
	fflush(stderr);
*/
	// De-interleave
	for (unsigned int i = 0U; i < 144U; i++) {
		unsigned int n = IMBE_INTERLEAVE[i];
		bit[i] = READ_BIT(data, n);
	}

	// now ..

	// 12 voice bits     0
	// 11 golay bits     12
	//
	// 12 voice bits     23
	// 11 golay bits     35
	//
	// 12 voice bits     46
	// 11 golay bits     58
	//
	// 12 voice bits     69
	// 11 golay bits     81
	//
	// 11 voice bits     92
	//  4 hamming bits   103
	//
	// 11 voice bits     107
	//  4 hamming bits   118
	//
	// 11 voice bits     122
	//  4 hamming bits   133
	//
	//  7 voice bits     137

	// c0
	unsigned int c0data = 0U;
	for (unsigned int i = 0U; i < 12U; i++)
		c0data = (c0data << 1) | (bit[i] ? 0x01U : 0x00U);

	bool prn[114U];

	// Create the whitening vector and save it for future use
	unsigned int p = 16U * c0data;
	for (unsigned int i = 0U; i < 114U; i++) {
		p = (173U * p + 13849U) % 65536U;
		prn[i] = p >= 32768U;
	}

	// De-whiten some bits
	for (unsigned int i = 0U; i < 114U; i++)
		bit[i + 23U] ^= prn[i];

	unsigned int offset = 0U;
	for (unsigned int i = 0U; i < 12U; i++, offset++)
		WRITE_BIT(imbe, offset, bit[i + 0U]);
	for (unsigned int i = 0U; i < 12U; i++, offset++)
		WRITE_BIT(imbe, offset, bit[i + 23U]);
	for (unsigned int i = 0U; i < 12U; i++, offset++)
		WRITE_BIT(imbe, offset, bit[i + 46U]);
	for (unsigned int i = 0U; i < 12U; i++, offset++)
		WRITE_BIT(imbe, offset, bit[i + 69U]);
	for (unsigned int i = 0U; i < 11U; i++, offset++)
		WRITE_BIT(imbe, offset, bit[i + 92U]);
	for (unsigned int i = 0U; i < 11U; i++, offset++)
		WRITE_BIT(imbe, offset, bit[i + 107U]);
	for (unsigned int i = 0U; i < 11U; i++, offset++)
		WRITE_BIT(imbe, offset, bit[i + 122U]);
	for (unsigned int i = 0U; i < 7U; i++, offset++)
		WRITE_BIT(imbe, offset, bit[i + 137U]);


	fprintf(stderr, "IMBE : ");
	for(int i = 0; i < 11; ++i){
		fprintf(stderr, "%02x ", imbe[i]);
	}
	fprintf(stderr, "\n");
	fflush(stderr);

}
