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

    All code in this file is based on code from DSDcc
    https://github.com/f4exb/dsdcc
*/

#include <iostream>
#include <string.h>
#include <math.h>
#include "mbedec.h"
#include "mbelib_parms.h"
#include "Golay24128.h"

const int MBEDecoder::dW[72] = {0,0,3,2,1,1,0,0,1,1,0,0,3,2,1,1,3,2,1,1,0,0,3,2,0,0,3,2,1,1,0,0,1,1,0,0,
                                3,2,1,1,3,2,1,1,0,0,3,2,0,0,3,2,1,1,0,0,1,1,0,0,3,2,1,1,3,3,2,1,0,0,3,3,};

const int MBEDecoder::dX[72] = {10,22,11,9,10,22,11,23,8,20,9,21,10,8,9,21,8,6,7,19,8,20,9,7,6,18,7,5,6,18,7,19,4,16,5,17,6,
                                4,5,17,4,2,3,15,4,16,5,3,2,14,3,1,2,14,3,15,0,12,1,13,2,0,1,13,0,12,10,11,0,12,1,13,};
const int MBEDecoder::rW[36] = {
  0, 1, 0, 1, 0, 1,
  0, 1, 0, 1, 0, 1,
  0, 1, 0, 1, 0, 1,
  0, 1, 0, 1, 0, 2,
  0, 2, 0, 2, 0, 2,
  0, 2, 0, 2, 0, 2
};

const int MBEDecoder::rX[36] = {
  23, 10, 22, 9, 21, 8,
  20, 7, 19, 6, 18, 5,
  17, 4, 16, 3, 15, 2,
  14, 1, 13, 0, 12, 10,
  11, 9, 10, 8, 9, 7,
  8, 6, 7, 5, 6, 4
};

// bit 0
const int MBEDecoder::rY[36] = {
  0, 2, 0, 2, 0, 2,
  0, 2, 0, 3, 0, 3,
  1, 3, 1, 3, 1, 3,
  1, 3, 1, 3, 1, 3,
  1, 3, 1, 3, 1, 3,
  1, 3, 1, 3, 1, 3
};

const int MBEDecoder::rZ[36] = {
  5, 3, 4, 2, 3, 1,
  2, 0, 1, 13, 0, 12,
  22, 11, 21, 10, 20, 9,
  19, 8, 18, 7, 17, 6,
  16, 5, 15, 4, 14, 3,
  13, 2, 12, 1, 11, 0
};

MBEDecoder::MBEDecoder() :
        m_upsamplerLastValue(0.0f),
        m_mbelibParms(nullptr)
{
    m_mbelibParms = new mbelibParms();
    m_audio_out_temp_buf_p = m_audio_out_temp_buf;
    memset(m_audio_out_float_buf, 0, sizeof(float) * 1120);
    m_audio_out_float_buf_p = m_audio_out_float_buf;
    memset(m_aout_max_buf, 0, sizeof(float) * 200);
    m_aout_max_buf_p = m_aout_max_buf;
    m_aout_max_buf_idx = 0;

    memset(m_audio_out_buf, 0, sizeof(short) * 2 * 48000);
    m_audio_out_buf_p = m_audio_out_buf;
    m_audio_out_nb_samples = 0;
    m_audio_out_buf_size = 48000; // given in number of unique samples
    m_audio_out_idx = 0;
    m_audio_out_idx2 = 0;

	m_aout_gain = 25;
    m_volume = 1.0f;
	m_hwrx = true;
	m_auto_gain = true;
    m_stereo = false;
    m_channels = 3; // both channels by default if stereo is set
    m_upsample = 0;

	initMbeParms();
	memset(ambe_d, 0, 49);
}

MBEDecoder::~MBEDecoder()
{
    delete m_mbelibParms;
}

void MBEDecoder::initMbeParms()
{
	mbe_initMbeParms(m_mbelibParms->m_cur_mp, m_mbelibParms->m_prev_mp, m_mbelibParms->m_prev_mp_enhanced);
	m_errs = 0;
	m_errs2 = 0;
	m_err_str[0] = 0;

    if (m_auto_gain){
        m_aout_gain = 25;
    }
}

void MBEDecoder::process_dstar(unsigned char *d)
{
    char ambe_fr[4][24];
    
    memset(ambe_fr, 0, 96);
    w = dW;
	x = dX;
	
	for(int i = 0; i < 9; ++i){
		for(int j = 0; j < 8; ++j){
			ambe_fr[*w][*x] = (1 & (d[i] >> j));
			w++;
			x++;
		}
	}

	mbe_processAmbe3600x2400Framef(m_audio_out_temp_buf, &m_errs, &m_errs2, m_err_str, ambe_fr, ambe_d,m_mbelibParms-> m_cur_mp, m_mbelibParms->m_prev_mp, m_mbelibParms->m_prev_mp_enhanced, 3);
    processAudio();
}

void MBEDecoder::process_dmr(unsigned char *d)
{
	char ambe_fr[4][24];

	memset(ambe_fr, 0, 96);
	w = rW;
	x = rX;
	y = rY;
	z = rZ;

	for(int i = 0; i < 9; ++i){
		for(int j = 0; j < 8; j+=2){
			ambe_fr[*y][*z] = (1 & (d[i] >> (7 - (j+1))));
			ambe_fr[*w][*x] = (1 & (d[i] >> (7 - j)));
			w++;
			x++;
			y++;
			z++;
		}
	}

	mbe_processAmbe3600x2450Framef(m_audio_out_temp_buf, &m_errs, &m_errs2, m_err_str, ambe_fr, ambe_d,m_mbelibParms-> m_cur_mp, m_mbelibParms->m_prev_mp, m_mbelibParms->m_prev_mp_enhanced, 3);
	processAudio();
}

void MBEDecoder::process_nxdn(unsigned char *d)
{
	char ambe_data[49];

	for(int i = 0; i < 6; ++i){
		for(int j = 0; j < 8; j++){
			ambe_data[j+(8*i)] = (1 & (d[i] >> (7 - j)));
		}
	}
	ambe_data[48] = (1 & (d[6] >> 7));
	processData(ambe_data, NULL);
}

void MBEDecoder::process_p25(unsigned char *d)
{
	char imbe_data[88];

	for(int i = 0; i < 11; ++i){
		for(int j = 0; j < 8; j++){
			imbe_data[j+(8*i)] = (1 & (d[i] >> (7 - j)));
		}
	}

	processData4400(imbe_data);
}

void MBEDecoder::ambe49to72(char ambe_data[49], char data[9])
{
	int tmp = 0;
	char ambe_fr[4][24];

	for(int i = 11; i >= 0; --i){
		tmp = (tmp << 1) | ambe_data[i];
	}

	tmp = CGolay24128::encode23127(tmp);
	int p = tmp & 0xff;
	p = p ^ ((tmp >> 8) & 0xff);
	p = p ^ ((tmp >> 16) & 0xff);
	p = p ^ (p >> 4);
	p = p ^ (p >> 2);
	p = p ^ (p >> 1);
	p = p & 1;
	tmp = tmp | (p << 23);

	for(int i = 23; i >= 0; i--){
		ambe_fr[0][i] = (tmp & 1);
		tmp = tmp >> 1;
	}
	tmp = 0;
	for(int i = 23; i > 11; --i){
		tmp = (tmp << 1) | ambe_data[i];
	}
	tmp = CGolay24128::encode23127(tmp);
	for(int i = 22; i >= 0; --i){
		ambe_fr[1][i] = (tmp & 1);
		tmp = tmp >> 1;
	}
	for(int i = 10; i >= 0; --i){
		ambe_fr[2][i] = ambe_d[34 - i];
	}
	for(int i = 13; i >= 0; --i){
		ambe_fr[3][i] = ambe_d[48 - i];
	}
	int i, j, k;
	unsigned short pr[115];
	unsigned short foo = 0;

	 // create pseudo-random modulator
	for (i = 23; i >= 12; i--){
		foo <<= 1;
		foo |= ambe_fr[0][i];
	}
	pr[0] = (16 * foo);
	for (i = 1; i < 24; i++){
		 pr[i] = (173 * pr[i - 1]) + 13849 - (65536 * (((173 * pr[i - 1]) + 13849) / 65536));
	}
	for (i = 1; i < 24; i++){
		pr[i] = pr[i] / 32768;
	}

	  // demodulate ambe_fr with pr
	k = 1;
	for (j = 22; j >= 0; j--){
		ambe_fr[1][j] = ((ambe_fr[1][j]) ^ pr[k]);
		k++;
	}
	//char data[9];
	char bit0, bit1;
	int bitIndex = 0;
	int ww = 0;
	int xx = 0;
	int yy = 0;
	int zz = 0;
	for(i = 0; i < 36; ++i){
		bit1 = ambe_fr[rW[ww]][rX[xx]];
		bit0 = ambe_fr[rY[yy]][rZ[zz]];
		data[bitIndex / 8] = ((data[bitIndex / 8] << 1) & 0xfe) | ((bit1) ? 1 : 0);
		bitIndex += 1;

		data[bitIndex / 8] = ((data[bitIndex / 8] << 1) & 0xfe) | ((bit0) ? 1 : 0);
		bitIndex += 1;

		ww += 1;
		xx += 1;
		yy += 1;
		zz += 1;
	}
}

void MBEDecoder::process_frame(char ambe_fr[4][24])
{
	mbe_processAmbe3600x2450Framef(m_audio_out_temp_buf, &m_errs, &m_errs2, m_err_str, ambe_fr, ambe_d,m_mbelibParms-> m_cur_mp, m_mbelibParms->m_prev_mp, m_mbelibParms->m_prev_mp_enhanced, 3);
	processAudio();
}

void MBEDecoder::processData(char ambe_data[49], unsigned char ambe_frame[9])
{
	if(m_hwrx){

		//char d[9];
		//ambe49to72(ambe_data, d);
		//fprintf(stderr, "MBEDecoder: ");
		for(int i = 0; i < 9; ++i){
			ambe72.push(ambe_frame[i]);
			//fprintf(stderr, "%02x ", (unsigned char)ambe_frame[i]);
		}
		//fprintf(stderr, "\n");
		//fflush(stderr);
		//std::cerr << "ambe72 size == " << ambe72.size() << std::endl;
	}
	else{
		mbe_processAmbe2450Dataf(m_audio_out_temp_buf, &m_errs,&m_errs2, m_err_str, ambe_data, m_mbelibParms->m_cur_mp,m_mbelibParms->m_prev_mp, m_mbelibParms->m_prev_mp_enhanced, 3);
		processAudio();
	}
}

void MBEDecoder::processData4400(char imbe_data[88])
{
	mbe_processImbe4400Dataf(m_audio_out_temp_buf, &m_errs,&m_errs2, m_err_str, imbe_data, m_mbelibParms->m_cur_mp,m_mbelibParms->m_prev_mp, m_mbelibParms->m_prev_mp_enhanced, 3);
	processAudio();
}

void MBEDecoder::processAudio()
{
    int i, n;
    float aout_abs, max, gainfactor, gaindelta, maxbuf;

    if (m_auto_gain){
        // detect max level
        max = 0;
        m_audio_out_temp_buf_p = m_audio_out_temp_buf;

        for (n = 0; n < 160; n++){
            aout_abs = fabsf(*m_audio_out_temp_buf_p);

            if (aout_abs > max){
                max = aout_abs;
            }

            m_audio_out_temp_buf_p++;
        }

        *m_aout_max_buf_p = max;
        m_aout_max_buf_p++;
        m_aout_max_buf_idx++;

        if (m_aout_max_buf_idx > 24){
            m_aout_max_buf_idx = 0;
            m_aout_max_buf_p = m_aout_max_buf;
        }

        // lookup max history
        for (i = 0; i < 25; i++){
            maxbuf = m_aout_max_buf[i];

            if (maxbuf > max){
                max = maxbuf;
            }
        }

        // determine optimal gain level
        if (max > static_cast<float>(0)){
            gainfactor = (static_cast<float>(30000) / max);
        }
        else{
            gainfactor = static_cast<float>(50);
        }

        if (gainfactor < m_aout_gain){
            m_aout_gain = gainfactor;
            gaindelta = static_cast<float>(0);
        }
        else{
            if (gainfactor > static_cast<float>(50)){
                gainfactor = static_cast<float>(50);
            }

            gaindelta = gainfactor - m_aout_gain;

            if (gaindelta > (static_cast<float>(0.05) * m_aout_gain)){
                gaindelta = (static_cast<float>(0.05) * m_aout_gain);
            }
        }

        gaindelta /= static_cast<float>(160);

        // adjust output gain
        m_audio_out_temp_buf_p = m_audio_out_temp_buf;

        for (n = 0; n < 160; n++){
            *m_audio_out_temp_buf_p = (m_aout_gain
                    + (static_cast<float>(n) * gaindelta)) * (*m_audio_out_temp_buf_p);
            m_audio_out_temp_buf_p++;
        }

        m_aout_gain += (static_cast<float>(160) * gaindelta);
    }
    else{
        gaindelta = static_cast<float>(0);
    }

    // copy audio data to output buffer and upsample if necessary
    m_audio_out_temp_buf_p = m_audio_out_temp_buf;

    if (m_audio_out_nb_samples + 160 >= m_audio_out_buf_size){
            resetAudio();
    }

    m_audio_out_float_buf_p = m_audio_out_float_buf;

    for (n = 0; n < 160; n++){
       if (*m_audio_out_temp_buf_p > static_cast<float>(32760)){
            *m_audio_out_temp_buf_p = static_cast<float>(32760);
        }
        else if (*m_audio_out_temp_buf_p < static_cast<float>(-32760)){
            *m_audio_out_temp_buf_p = static_cast<float>(-32760);
        }

        *m_audio_out_buf_p = static_cast<short>(*m_audio_out_temp_buf_p);
        m_audio_out_buf_p++;

        if (m_stereo){
            *m_audio_out_buf_p = static_cast<short>(*m_audio_out_temp_buf_p);
            m_audio_out_buf_p++;
        }

        m_audio_out_nb_samples++;
        m_audio_out_temp_buf_p++;
        m_audio_out_idx++;
        m_audio_out_idx2++;
    }
}
