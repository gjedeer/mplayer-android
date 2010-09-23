/*
** FAAD2 - Freeware Advanced Audio (AAC) Decoder including SBR decoding
** Copyright (C) 2003-2004 M. Bakker, Ahead Software AG, http://www.nero.com
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**
** Any non-GPL usage of this software or parts of this software is strictly
** forbidden.
**
** Commercial non-GPL licensing of this software is possible.
** For more info contact Ahead Software through Mpeg4AAClicense@nero.com.
**
** $Id: mp4.h,v 1.21 2004/09/04 14:56:28 menno Exp $
**/

#ifndef __MP4_H__
#define __MP4_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "bits.h"
#include "decoder.h"

int8_t NEAACDECAPI NeAACDecAudioSpecificConfig(uint8_t *pBuffer,
                                               uint32_t buffer_size,
                                               mp4AudioSpecificConfig *mp4ASC);

int8_t AudioSpecificConfig2(uint8_t *pBuffer,
                            uint32_t buffer_size,
                            mp4AudioSpecificConfig *mp4ASC,
                            program_config *pce, uint8_t short_form);

int8_t AudioSpecificConfigFromBitfile(bitfile *ld,
                                      mp4AudioSpecificConfig *mp4ASC,
                                      program_config *pce, uint32_t bsize, uint8_t short_form);
#ifdef __cplusplus
}
#endif
#endif
