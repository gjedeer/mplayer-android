/*
 * This file is part of MPlayer.
 *
 * MPlayer is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * MPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with MPlayer; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef MPLAYER_VE_H
#define MPLAYER_VE_H

#include "m_option.h"

extern const m_option_t lavcopts_conf[];
extern const m_option_t vfwopts_conf[];
extern const m_option_t xvidencopts_conf[];

extern char *lavc_param_acodec;
extern char *lavc_param_audio_avopt;
extern int   lavc_param_abitrate;
extern int   lavc_param_atag;
extern int   lavc_param_audio_global_header;

#endif /* MPLAYER_VE_H */
