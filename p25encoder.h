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

#ifndef P25ENCODER_H
#define P25ENCODER_H


class p25encoder
{
public:
	p25encoder();
	unsigned char * get_frame(unsigned char *ambe);
private:
	int m_p25cnt;
	unsigned char imbe[11U];
	void decode(const unsigned char* data);
	void extract_voice_bits(const unsigned char* data, unsigned char* pimbe);
};

#endif // P25ENCODER_H
