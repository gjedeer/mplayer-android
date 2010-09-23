// -*- c-basic-offset: 8; indent-tabs-mode: t -*-
// vim:ts=8:sw=8:noet:ai:
/*
 * Copyright (C) 2006 Evgeniy Stepanov <eugeni.stepanov@gmail.com>
 *
 * This file is part of libass.
 *
 * libass is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * libass is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with libass; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef LIBASS_MP_H
#define LIBASS_MP_H

#include <stdint.h>

#include "subreader.h"
#if defined(CONFIG_ASS_INTERNAL) || !defined(CONFIG_ASS)
#include "ass.h"
#else
#include <ass/ass.h>
#endif

extern ASS_Library* ass_library;
extern int ass_enabled;
extern float ass_font_scale;
extern float ass_line_spacing;
extern int ass_top_margin;
extern int ass_bottom_margin;
extern int extract_embedded_fonts;
extern char **ass_force_style_list;
extern int ass_use_margins;
extern char* ass_color;
extern char* ass_border_color;
extern char* ass_styles_file;
extern int ass_hinting;

ASS_Track* ass_default_track(ASS_Library* library);
int ass_process_subtitle(ASS_Track* track, subtitle* sub);
ASS_Track* ass_read_subdata(ASS_Library* library, sub_data* subdata, double fps);
ASS_Track* ass_read_stream(ASS_Library* library, const char *fname, char *charset);

void ass_configure(ASS_Renderer* priv, int w, int h, int hinting);
void ass_configure_fonts(ASS_Renderer* priv);
ASS_Library* ass_init(void);

typedef struct {
	ASS_Image* imgs;
	int changed;
} EOSD_ImageList;

extern int ass_force_reload;
ASS_Image* ass_mp_render_frame(ASS_Renderer *priv, ASS_Track* track, long long now, int* detect_change);

#endif /* LIBASS_MP_H */
