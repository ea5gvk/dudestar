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

#ifndef MBELIB_H_
#define MBELIB_H_

#include <stdlib.h>
extern "C" {
#include <mbelib.h>
}

struct mbelibParms
{
    mbe_parms *m_cur_mp;
    mbe_parms *m_prev_mp;
    mbe_parms *m_prev_mp_enhanced;

    mbelibParms()
    {
        m_cur_mp = (mbe_parms *) malloc(sizeof(mbe_parms));
        m_prev_mp = (mbe_parms *) malloc(sizeof(mbe_parms));
        m_prev_mp_enhanced = (mbe_parms *) malloc(sizeof(mbe_parms));
    }

    ~mbelibParms()
    {
        free(m_prev_mp_enhanced);
        free(m_prev_mp);
        free(m_cur_mp);
    }
};

#endif /* MBELIB_H_ */
