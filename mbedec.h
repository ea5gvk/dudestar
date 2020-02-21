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

#ifndef MBE_H_
#define MBE_H_

#include <queue>

struct mbelibParms;

class MBEDecoder
{
public:
    explicit MBEDecoder();
    ~MBEDecoder();

    void initMbeParms();
	void process_dstar(unsigned char *d);
	void process_dmr(unsigned char *d);
	void process_nxdn(unsigned char *d);
	void process_p25(unsigned char *d);
	void ambe49to72(char ambe_data[49], char data[9]);
	void process_frame(char ambe_fr[4][24]);
	void processData(char ambe_data[49], unsigned char ambe_frame[9]);
	void processData4400(char imbe_data[88]);
    short *getAudio(int& nbSamples)
    {
        nbSamples = m_audio_out_nb_samples;
        return m_audio_out_buf;
    }

    void resetAudio()
    {
        m_audio_out_nb_samples = 0;
        m_audio_out_buf_p = m_audio_out_buf;
    }

    void setAudioGain(float aout_gain) { m_aout_gain = aout_gain; }
    void setAutoGain(bool auto_gain) { m_auto_gain = auto_gain; }
    void setVolume(float volume) { m_volume = volume; }
    void setStereo(bool stereo) { m_stereo = stereo; }
    void setChannels(unsigned char channels) { m_channels = channels % 4; }
    void setUpsamplingFactor(int upsample) { m_upsample = upsample; }
    int getUpsamplingFactor() const { return m_upsample; }
	void set_hwrx(bool h){ m_hwrx = h; }
	std::queue<char> ambe72;

private:
    void processAudio();
    void upsample(int upsampling, float invalue);

    float m_upsamplerLastValue;

    mbelibParms *m_mbelibParms;
    int m_errs;
    int m_errs2;
    char m_err_str[64];

    float m_audio_out_temp_buf[160];   //!< output of decoder
    float *m_audio_out_temp_buf_p;

    float m_audio_out_float_buf[1120]; //!< output of upsampler - 1 frame of 160 samples upampled up to 7 times
    float *m_audio_out_float_buf_p;

    float m_aout_max_buf[200];
    float *m_aout_max_buf_p;
    int m_aout_max_buf_idx;

    short m_audio_out_buf[2*48000];    //!< final result - 1s of L+R S16LE samples
    short *m_audio_out_buf_p;
    int   m_audio_out_nb_samples;
    int   m_audio_out_buf_size;
    int   m_audio_out_idx;
    int   m_audio_out_idx2;

    float m_aout_gain;
    float m_volume;
    bool m_auto_gain;
    int m_upsample;            //!< upsampling factor
    bool m_stereo;             //!< double each audio sample to produce L+R channels
    unsigned char m_channels;  //!< when in stereo output to none (0) or only left (1), right (2) or both (3) channels
	const int *w, *x, *y, *z;
	static const int dW[72];
	static const int dX[72];
	static const int rW[36];
	static const int rX[36];
	static const int rY[36];
	static const int rZ[36];
	char ambe_d[49];
	static const int dvsi_interleave[49];
	bool m_hwrx;
};

#endif /* MBE_H_ */
