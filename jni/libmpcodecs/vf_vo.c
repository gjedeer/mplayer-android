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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "mp_msg.h"
#include "mpcommon.h"
#include "mp_image.h"
#include "vf.h"

#include "libvo/sub.h"
#include "libvo/video_out.h"

#include "eosd.h"

//===========================================================================//

struct vf_priv_s {
    double pts;
    const vo_functions_t *vo;
};
#define video_out (vf->priv->vo)

static int query_format(struct vf_instance *vf, unsigned int fmt); /* forward declaration */
static void draw_slice(struct vf_instance *vf, unsigned char** src, int* stride, int w,int h, int x, int y);

static int config(struct vf_instance *vf,
        int width, int height, int d_width, int d_height,
	unsigned int flags, unsigned int outfmt){

    if ((width <= 0) || (height <= 0) || (d_width <= 0) || (d_height <= 0))
    {
	mp_msg(MSGT_CPLAYER, MSGL_ERR, "VO: invalid dimensions!\n");
	return 0;
    }

  if(video_out->info)
  { const vo_info_t *info = video_out->info;
    mp_msg(MSGT_CPLAYER,MSGL_INFO,"VO: [%s] %dx%d => %dx%d %s %s%s%s%s\n",info->short_name,
         width, height,
         d_width, d_height,
	 vo_format_name(outfmt),
         (flags&VOFLAG_FULLSCREEN)?" [fs]":"",
         (flags&VOFLAG_MODESWITCHING)?" [vm]":"",
         (flags&VOFLAG_SWSCALE)?" [zoom]":"",
         (flags&VOFLAG_FLIPPING)?" [flip]":"");
    mp_msg(MSGT_CPLAYER,MSGL_V,"VO: Description: %s\n",info->name);
    mp_msg(MSGT_CPLAYER,MSGL_V,"VO: Author: %s\n", info->author);
    if(info->comment && strlen(info->comment) > 0)
        mp_msg(MSGT_CPLAYER,MSGL_V,"VO: Comment: %s\n", info->comment);
  }

    // save vo's stride capability for the wanted colorspace:
    vf->default_caps=query_format(vf,outfmt);
    vf->draw_slice = (vf->default_caps & VOCAP_NOSLICES) ? NULL : draw_slice;

    if(config_video_out(video_out,width,height,d_width,d_height,flags,"MPlayer",outfmt))
	return 0;

    ++vo_config_count;
    return 1;
}

static int control(struct vf_instance *vf, int request, void* data)
{
    switch(request){
    case VFCTRL_GET_DEINTERLACE:
    {
        if(!video_out) return CONTROL_FALSE; // vo not configured?
        return(video_out->control(VOCTRL_GET_DEINTERLACE, data)
                == VO_TRUE) ? CONTROL_TRUE : CONTROL_FALSE;
    }
    case VFCTRL_SET_DEINTERLACE:
    {
        if(!video_out) return CONTROL_FALSE; // vo not configured?
        return(video_out->control(VOCTRL_SET_DEINTERLACE, data)
                == VO_TRUE) ? CONTROL_TRUE : CONTROL_FALSE;
    }
    case VFCTRL_DRAW_OSD:
	if(!vo_config_count) return CONTROL_FALSE; // vo not configured?
	video_out->draw_osd();
	return CONTROL_TRUE;
    case VFCTRL_FLIP_PAGE:
    {
	if(!vo_config_count) return CONTROL_FALSE; // vo not configured?
	video_out->flip_page();
	return CONTROL_TRUE;
    }
    case VFCTRL_SET_EQUALIZER:
    {
	vf_equalizer_t *eq=data;
	if(!vo_config_count) return CONTROL_FALSE; // vo not configured?
	return (video_out->control(VOCTRL_SET_EQUALIZER, eq->item, eq->value) == VO_TRUE) ? CONTROL_TRUE : CONTROL_FALSE;
    }
    case VFCTRL_GET_EQUALIZER:
    {
	vf_equalizer_t *eq=data;
	if(!vo_config_count) return CONTROL_FALSE; // vo not configured?
	return (video_out->control(VOCTRL_GET_EQUALIZER, eq->item, &eq->value) == VO_TRUE) ? CONTROL_TRUE : CONTROL_FALSE;
    }
#ifdef CONFIG_ASS
    case VFCTRL_INIT_EOSD:
    {
        return CONTROL_TRUE;
    }
    case VFCTRL_DRAW_EOSD:
    {
        EOSD_ImageList images = {NULL, 2};
        mp_eosd_res_t res = {0};
        double pts = vf->priv->pts;
        if (!vo_config_count) return CONTROL_FALSE;
        if (video_out->control(VOCTRL_GET_EOSD_RES, &res) == VO_TRUE)
            eosd_configure(&res, !!(vf->default_caps & VFCAP_EOSD_UNSCALED));
        images.imgs = eosd_render_frame(pts, &images.changed);
        return (video_out->control(VOCTRL_DRAW_EOSD, &images) == VO_TRUE) ? CONTROL_TRUE : CONTROL_FALSE;
    }
#endif
    case VFCTRL_GET_PTS:
    {
	*(double *)data = vf->priv->pts;
	return CONTROL_TRUE;
    }
    }
    // return video_out->control(request,data);
    return CONTROL_UNKNOWN;
}

static int query_format(struct vf_instance *vf, unsigned int fmt){
    int flags=video_out->control(VOCTRL_QUERY_FORMAT,&fmt);
    // draw_slice() accepts stride, draw_frame() doesn't:
    if(flags)
	if(fmt==IMGFMT_YV12 || fmt==IMGFMT_I420 || fmt==IMGFMT_IYUV)
	    flags|=VFCAP_ACCEPT_STRIDE;
    return flags;
}

static void get_image(struct vf_instance *vf,
        mp_image_t *mpi){
    if(!vo_config_count) return;
    // GET_IMAGE is required for hardware-accelerated formats
    if(vo_directrendering ||
       IMGFMT_IS_XVMC(mpi->imgfmt) || IMGFMT_IS_VDPAU(mpi->imgfmt))
	video_out->control(VOCTRL_GET_IMAGE,mpi);
}

static int put_image(struct vf_instance *vf,
        mp_image_t *mpi, double pts){
  if(!vo_config_count) return 0; // vo not configured?
  // record pts (potentially modified by filters) for main loop
  vf->priv->pts = pts;
  // first check, maybe the vo/vf plugin implements draw_image using mpi:
  if(video_out->control(VOCTRL_DRAW_IMAGE,mpi)==VO_TRUE) return 1; // done.
  // nope, fallback to old draw_frame/draw_slice:
  if(!(mpi->flags&(MP_IMGFLAG_DIRECT|MP_IMGFLAG_DRAW_CALLBACK))){
    // blit frame:
//    if(mpi->flags&MP_IMGFLAG_PLANAR)
    if(vf->default_caps&VFCAP_ACCEPT_STRIDE)
        video_out->draw_slice(mpi->planes,mpi->stride,mpi->w,mpi->h,mpi->x,mpi->y);
    else
        video_out->draw_frame(mpi->planes);
  }
  return 1;
}

static void start_slice(struct vf_instance *vf,
		       mp_image_t *mpi) {
    if(!vo_config_count) return; // vo not configured?
    video_out->control(VOCTRL_START_SLICE,mpi);
}

static void draw_slice(struct vf_instance *vf,
        unsigned char** src, int* stride, int w,int h, int x, int y){
    if(!vo_config_count) return; // vo not configured?
    video_out->draw_slice(src,stride,w,h,x,y);
}

static void uninit(struct vf_instance *vf)
{
        free(vf->priv);
}
//===========================================================================//

static int vf_open(vf_instance_t *vf, char *args){
    vf->config=config;
    vf->control=control;
    vf->query_format=query_format;
    vf->get_image=get_image;
    vf->put_image=put_image;
    vf->draw_slice=draw_slice;
    vf->start_slice=start_slice;
    vf->uninit=uninit;
    vf->priv=calloc(1, sizeof(struct vf_priv_s));
    vf->priv->vo = (const vo_functions_t *)args;
    if(!video_out) return 0; // no vo ?
    return 1;
}

const vf_info_t vf_info_vo = {
    "libvo wrapper",
    "vo",
    "A'rpi",
    "for internal use",
    vf_open,
    NULL
};

//===========================================================================//
