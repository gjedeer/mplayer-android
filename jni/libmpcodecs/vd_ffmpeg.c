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
#include <assert.h>
#include <time.h>

#include "config.h"
#include "mp_msg.h"
#include "help_mp.h"
#include "av_opts.h"

#include "libavutil/common.h"
#include "libavutil/intreadwrite.h"
#include "mpbswap.h"
#include "fmt-conversion.h"

#include "vd_internal.h"
#include "vd_ffmpeg.h"

static const vd_info_t info = {
    "FFmpeg's libavcodec codec family",
    "ffmpeg",
    "A'rpi",
    "A'rpi, Michael, Alex",
    "native codecs"
};

LIBVD_EXTERN(ffmpeg)

#include "libavcodec/avcodec.h"

#if AVPALETTE_SIZE > 1024
#error palette too large, adapt libmpcodecs/vf.c:vf_get_image
#endif

#if CONFIG_XVMC
#include "libavcodec/xvmc.h"
#endif

int avcodec_initialized=0;

typedef struct {
    AVCodecContext *avctx;
    AVFrame *pic;
    enum PixelFormat pix_fmt;
    int do_slices;
    int do_dr1;
    int vo_initialized;
    int best_csp;
    int b_age;
    int ip_age[2];
    int qp_stat[32];
    double qp_sum;
    double inv_qp_sum;
    int ip_count;
    int b_count;
    AVRational last_sample_aspect_ratio;
} vd_ffmpeg_ctx;

#include "m_option.h"

static int get_buffer(AVCodecContext *avctx, AVFrame *pic);
static void release_buffer(AVCodecContext *avctx, AVFrame *pic);
static void draw_slice(struct AVCodecContext *s, const AVFrame *src, int offset[4],
                       int y, int type, int height);

static enum PixelFormat get_format(struct AVCodecContext *avctx,
                                   const enum PixelFormat *pix_fmt);

static int lavc_param_workaround_bugs= FF_BUG_AUTODETECT;
static int lavc_param_error_resilience=2;
static int lavc_param_error_concealment=3;
static int lavc_param_gray=0;
static int lavc_param_vstats=0;
static int lavc_param_idct_algo=0;
static int lavc_param_debug=0;
static int lavc_param_vismv=0;
static int lavc_param_skip_top=0;
static int lavc_param_skip_bottom=0;
static int lavc_param_fast=0;
static int lavc_param_lowres=0;
static char *lavc_param_lowres_str=NULL;
static char *lavc_param_skip_loop_filter_str = NULL;
static char *lavc_param_skip_idct_str = NULL;
static char *lavc_param_skip_frame_str = NULL;
static int lavc_param_threads=1;
static int lavc_param_bitexact=0;
static char *lavc_avopt = NULL;

static const mp_image_t mpi_no_picture =
{
	.type = MP_IMGTYPE_INCOMPLETE
};

const m_option_t lavc_decode_opts_conf[]={
    {"bug", &lavc_param_workaround_bugs, CONF_TYPE_INT, CONF_RANGE, -1, 999999, NULL},
    {"er", &lavc_param_error_resilience, CONF_TYPE_INT, CONF_RANGE, 0, 99, NULL},
    {"gray", &lavc_param_gray, CONF_TYPE_FLAG, 0, 0, CODEC_FLAG_PART, NULL},
    {"idct", &lavc_param_idct_algo, CONF_TYPE_INT, CONF_RANGE, 0, 99, NULL},
    {"ec", &lavc_param_error_concealment, CONF_TYPE_INT, CONF_RANGE, 0, 99, NULL},
    {"vstats", &lavc_param_vstats, CONF_TYPE_FLAG, 0, 0, 1, NULL},
    {"debug", &lavc_param_debug, CONF_TYPE_INT, CONF_RANGE, 0, 9999999, NULL},
    {"vismv", &lavc_param_vismv, CONF_TYPE_INT, CONF_RANGE, 0, 9999999, NULL},
    {"st", &lavc_param_skip_top, CONF_TYPE_INT, CONF_RANGE, 0, 999, NULL},
    {"sb", &lavc_param_skip_bottom, CONF_TYPE_INT, CONF_RANGE, 0, 999, NULL},
    {"fast", &lavc_param_fast, CONF_TYPE_FLAG, 0, 0, CODEC_FLAG2_FAST, NULL},
    {"lowres", &lavc_param_lowres_str, CONF_TYPE_STRING, 0, 0, 0, NULL},
    {"skiploopfilter", &lavc_param_skip_loop_filter_str, CONF_TYPE_STRING, 0, 0, 0, NULL},
    {"skipidct", &lavc_param_skip_idct_str, CONF_TYPE_STRING, 0, 0, 0, NULL},
    {"skipframe", &lavc_param_skip_frame_str, CONF_TYPE_STRING, 0, 0, 0, NULL},
    {"threads", &lavc_param_threads, CONF_TYPE_INT, CONF_RANGE, 1, 8, NULL},
    {"bitexact", &lavc_param_bitexact, CONF_TYPE_FLAG, 0, 0, CODEC_FLAG_BITEXACT, NULL},
    {"o", &lavc_avopt, CONF_TYPE_STRING, 0, 0, 0, NULL},
    {NULL, NULL, 0, 0, 0, 0, NULL}
};

static enum AVDiscard str2AVDiscard(char *str) {
    if (!str)                               return AVDISCARD_DEFAULT;
    if (strcasecmp(str, "none"   ) == 0)    return AVDISCARD_NONE;
    if (strcasecmp(str, "default") == 0)    return AVDISCARD_DEFAULT;
    if (strcasecmp(str, "nonref" ) == 0)    return AVDISCARD_NONREF;
    if (strcasecmp(str, "bidir"  ) == 0)    return AVDISCARD_BIDIR;
    if (strcasecmp(str, "nonkey" ) == 0)    return AVDISCARD_NONKEY;
    if (strcasecmp(str, "all"    ) == 0)    return AVDISCARD_ALL;
    mp_msg(MSGT_DECVIDEO, MSGL_ERR, "Unknown discard value %s\n", str);
    return AVDISCARD_DEFAULT;
}

// to set/get/query special features/parameters
static int control(sh_video_t *sh, int cmd, void *arg, ...){
    vd_ffmpeg_ctx *ctx = sh->context;
    AVCodecContext *avctx = ctx->avctx;
    switch(cmd){
    case VDCTRL_QUERY_FORMAT:
    {
        int format =(*((int *)arg));
        if(format == ctx->best_csp) return CONTROL_TRUE;//supported
        // possible conversions:
        switch(format){
        case IMGFMT_YV12:
        case IMGFMT_IYUV:
        case IMGFMT_I420:
            // "converted" using pointer/stride modification
            if(avctx->pix_fmt==PIX_FMT_YUV420P) return CONTROL_TRUE;// u/v swap
            if(avctx->pix_fmt==PIX_FMT_YUV422P && !ctx->do_dr1) return CONTROL_TRUE;// half stride
            break;
#if CONFIG_XVMC
        case IMGFMT_XVMC_IDCT_MPEG2:
        case IMGFMT_XVMC_MOCO_MPEG2:
            if(avctx->pix_fmt==PIX_FMT_XVMC_MPEG2_IDCT) return CONTROL_TRUE;
#endif
        }
        return CONTROL_FALSE;
    }
    case VDCTRL_RESYNC_STREAM:
        avcodec_flush_buffers(avctx);
        return CONTROL_TRUE;
    case VDCTRL_QUERY_UNSEEN_FRAMES:
        return avctx->has_b_frames + 10;
    }
    return CONTROL_UNKNOWN;
}

static void mp_msp_av_log_callback(void *ptr, int level, const char *fmt,
                                   va_list vl)
{
    static int print_prefix=1;
    AVClass *avc= ptr ? *(AVClass **)ptr : NULL;
    int type= MSGT_FIXME;
    int mp_level;
    char buf[256];

    switch(level){
    case AV_LOG_VERBOSE: mp_level = MSGL_V ; break;
    case AV_LOG_DEBUG:  mp_level= MSGL_V   ; break;
    case AV_LOG_INFO :  mp_level= MSGL_INFO; break;
    case AV_LOG_ERROR:  mp_level= MSGL_ERR ; break;
    default          :  mp_level= MSGL_ERR ; break;
    }

    if(ptr){
        if(!strcmp(avc->class_name, "AVCodecContext")){
            AVCodecContext *s= ptr;
            if(s->codec){
                if(s->codec->type == CODEC_TYPE_AUDIO){
                    if(s->codec->decode)
                        type= MSGT_DECAUDIO;
                }else if(s->codec->type == CODEC_TYPE_VIDEO){
                    if(s->codec->decode)
                        type= MSGT_DECVIDEO;
                }
                //FIXME subtitles, encoders (what msgt for them? there is no appropriate ...)
            }
        }else if(!strcmp(avc->class_name, "AVFormatContext")){
#if 0 //needs libavformat include FIXME iam too lazy to do this cleanly, probably the whole should be moved out of this file ...
            AVFormatContext *s= ptr;
            if(s->iformat)
                type= MSGT_DEMUXER;
            else if(s->oformat)
                type= MSGT_MUXER;
#endif
        }
    }

    if (!mp_msg_test(type, mp_level)) return;

    if(print_prefix && avc) {
        mp_msg(type, mp_level, "[%s @ %p]", avc->item_name(ptr), avc);
    }

    print_prefix= strchr(fmt, '\n') != NULL;
    vsnprintf(buf, sizeof(buf), fmt, vl);
    mp_msg(type, mp_level, buf);
}

static void set_format_params(struct AVCodecContext *avctx, enum PixelFormat fmt){
    int imgfmt;
    if (fmt == PIX_FMT_NONE)
        return;
    imgfmt = pixfmt2imgfmt(fmt);
    if (IMGFMT_IS_XVMC(imgfmt) || IMGFMT_IS_VDPAU(imgfmt)) {
        sh_video_t *sh     = avctx->opaque;
        vd_ffmpeg_ctx *ctx = sh->context;
        ctx->do_dr1    = 1;
        ctx->do_slices = 1;
        avctx->thread_count    = 1;
        avctx->get_buffer      = get_buffer;
        avctx->release_buffer  = release_buffer;
        avctx->reget_buffer    = get_buffer;
        avctx->draw_horiz_band = draw_slice;
        mp_msg(MSGT_DECVIDEO, MSGL_INFO, MSGTR_MPCODECS_XVMCAcceleratedMPEG2);
        avctx->slice_flags = SLICE_FLAG_CODED_ORDER|SLICE_FLAG_ALLOW_FIELD;
    }
}

void init_avcodec(void)
{
    if (!avcodec_initialized) {
        avcodec_init();
        avcodec_register_all();
        avcodec_initialized = 1;
        av_log_set_callback(mp_msp_av_log_callback);
    }
}

// init driver
static int init(sh_video_t *sh){
    AVCodecContext *avctx;
    vd_ffmpeg_ctx *ctx;
    AVCodec *lavc_codec;
    int lowres_w=0;
    int do_vis_debug= lavc_param_vismv || (lavc_param_debug&(FF_DEBUG_VIS_MB_TYPE|FF_DEBUG_VIS_QP));

    init_avcodec();

    ctx = sh->context = malloc(sizeof(vd_ffmpeg_ctx));
    if (!ctx)
        return 0;
    memset(ctx, 0, sizeof(vd_ffmpeg_ctx));

    lavc_codec = avcodec_find_decoder_by_name(sh->codec->dll);
    if(!lavc_codec){
        mp_msg(MSGT_DECVIDEO, MSGL_ERR, MSGTR_MissingLAVCcodec, sh->codec->dll);
        uninit(sh);
        return 0;
    }

    if(vd_use_slices && (lavc_codec->capabilities&CODEC_CAP_DRAW_HORIZ_BAND) && !do_vis_debug)
        ctx->do_slices=1;

    if(lavc_codec->capabilities&CODEC_CAP_DR1 && !do_vis_debug && lavc_codec->id != CODEC_ID_H264 && lavc_codec->id != CODEC_ID_INTERPLAY_VIDEO && lavc_codec->id != CODEC_ID_ROQ && lavc_codec->id != CODEC_ID_VP8)
        ctx->do_dr1=1;
    ctx->b_age= ctx->ip_age[0]= ctx->ip_age[1]= 256*256*256*64;
    ctx->ip_count= ctx->b_count= 0;

    ctx->pic = avcodec_alloc_frame();
    ctx->avctx = avcodec_alloc_context();
    avctx = ctx->avctx;
    avctx->opaque = sh;
    avctx->codec_type = CODEC_TYPE_VIDEO;
    avctx->codec_id = lavc_codec->id;

#if CONFIG_VDPAU
    if(lavc_codec->capabilities & CODEC_CAP_HWACCEL_VDPAU){
        avctx->get_format = get_format;
    }
#endif /* CONFIG_VDPAU */
#if CONFIG_XVMC
    if(lavc_codec->capabilities & CODEC_CAP_HWACCEL){
        mp_msg(MSGT_DECVIDEO, MSGL_INFO, MSGTR_MPCODECS_XVMCAcceleratedCodec);
        avctx->get_format= get_format;//for now only this decoder will use it
        // HACK around badly placed checks in mpeg_mc_decode_init
        set_format_params(avctx, PIX_FMT_XVMC_MPEG2_IDCT);
    }
#endif /* CONFIG_XVMC */
    if(ctx->do_dr1){
        avctx->flags|= CODEC_FLAG_EMU_EDGE;
        avctx->get_buffer= get_buffer;
        avctx->release_buffer= release_buffer;
        avctx->reget_buffer= get_buffer;
    }

    avctx->flags|= lavc_param_bitexact;

    avctx->coded_width = sh->disp_w;
    avctx->coded_height= sh->disp_h;
    avctx->workaround_bugs= lavc_param_workaround_bugs;
    avctx->error_recognition= lavc_param_error_resilience;
    if(lavc_param_gray) avctx->flags|= CODEC_FLAG_GRAY;
    avctx->flags2|= lavc_param_fast;
    avctx->codec_tag= sh->format;
    avctx->stream_codec_tag= sh->video.fccHandler;
    avctx->idct_algo= lavc_param_idct_algo;
    avctx->error_concealment= lavc_param_error_concealment;
    avctx->debug= lavc_param_debug;
    if (lavc_param_debug)
        av_log_set_level(AV_LOG_DEBUG);
    avctx->debug_mv= lavc_param_vismv;
    avctx->skip_top   = lavc_param_skip_top;
    avctx->skip_bottom= lavc_param_skip_bottom;
    if(lavc_param_lowres_str != NULL)
    {
        sscanf(lavc_param_lowres_str, "%d,%d", &lavc_param_lowres, &lowres_w);
        if(lavc_param_lowres < 1 || lavc_param_lowres > 16 || (lowres_w > 0 && avctx->width < lowres_w))
            lavc_param_lowres = 0;
        avctx->lowres = lavc_param_lowres;
    }
    avctx->skip_loop_filter = str2AVDiscard(lavc_param_skip_loop_filter_str);
    avctx->skip_idct = str2AVDiscard(lavc_param_skip_idct_str);
    avctx->skip_frame = str2AVDiscard(lavc_param_skip_frame_str);

    if(lavc_avopt){
        if(parse_avopts(avctx, lavc_avopt) < 0){
            mp_msg(MSGT_DECVIDEO, MSGL_ERR, "Your options /%s/ look like gibberish to me pal\n", lavc_avopt);
            uninit(sh);
            return 0;
        }
    }

    mp_dbg(MSGT_DECVIDEO, MSGL_DBG2, "libavcodec.size: %d x %d\n", avctx->width, avctx->height);
    switch (sh->format) {
    case mmioFOURCC('S','V','Q','3'):
    /* SVQ3 extradata can show up as sh->ImageDesc if demux_mov is used, or
       in the phony AVI header if demux_lavf is used. The first case is
       handled here; the second case falls through to the next section. */
        if (sh->ImageDesc) {
            avctx->extradata_size = (*(int *)sh->ImageDesc) - sizeof(int);
            avctx->extradata = av_mallocz(avctx->extradata_size + FF_INPUT_BUFFER_PADDING_SIZE);
            memcpy(avctx->extradata, ((int *)sh->ImageDesc)+1, avctx->extradata_size);
            break;
        }
        /* fallthrough */

    case mmioFOURCC('A','V','R','n'):
    case mmioFOURCC('M','J','P','G'):
    /* AVRn stores huffman table in AVI header */
    /* Pegasus MJPEG stores it also in AVI header, but it uses the common
       MJPG fourcc :( */
        if (!sh->bih || sh->bih->biSize <= sizeof(BITMAPINFOHEADER))
            break;
        avctx->flags |= CODEC_FLAG_EXTERN_HUFF;
        avctx->extradata_size = sh->bih->biSize-sizeof(BITMAPINFOHEADER);
        avctx->extradata = av_mallocz(avctx->extradata_size + FF_INPUT_BUFFER_PADDING_SIZE);
        memcpy(avctx->extradata, sh->bih+1, avctx->extradata_size);

#if 0
        {
            int x;
            uint8_t *p = avctx->extradata;

            for (x=0; x<avctx->extradata_size; x++)
                mp_msg(MSGT_DECVIDEO, MSGL_INFO, "[%x] ", p[x]);
            mp_msg(MSGT_DECVIDEO, MSGL_INFO, "\n");
        }
#endif
        break;

    case mmioFOURCC('R', 'V', '1', '0'):
    case mmioFOURCC('R', 'V', '1', '3'):
    case mmioFOURCC('R', 'V', '2', '0'):
    case mmioFOURCC('R', 'V', '3', '0'):
    case mmioFOURCC('R', 'V', '4', '0'):
        if(sh->bih->biSize<sizeof(*sh->bih)+8){
            /* only 1 packet per frame & sub_id from fourcc */
            avctx->extradata_size= 8;
            avctx->extradata = av_mallocz(avctx->extradata_size + FF_INPUT_BUFFER_PADDING_SIZE);
            ((uint32_t *)avctx->extradata)[0] = 0;
            ((uint32_t *)avctx->extradata)[1] =
                (sh->format == mmioFOURCC('R', 'V', '1', '3')) ? 0x10003001 : 0x10000000;
        } else {
            /* has extra slice header (demux_rm or rm->avi streamcopy) */
            avctx->extradata_size = sh->bih->biSize-sizeof(BITMAPINFOHEADER);
            avctx->extradata = av_mallocz(avctx->extradata_size + FF_INPUT_BUFFER_PADDING_SIZE);
            memcpy(avctx->extradata, sh->bih+1, avctx->extradata_size);
        }
        avctx->sub_id= AV_RB32(avctx->extradata+4);

//        printf("%X %X %d %d\n", extrahdr[0], extrahdr[1]);
        break;

    default:
        if (!sh->bih || sh->bih->biSize <= sizeof(BITMAPINFOHEADER))
            break;
        avctx->extradata_size = sh->bih->biSize-sizeof(BITMAPINFOHEADER);
        avctx->extradata = av_mallocz(avctx->extradata_size + FF_INPUT_BUFFER_PADDING_SIZE);
        memcpy(avctx->extradata, sh->bih+1, avctx->extradata_size);
        break;
    }
    /* Pass palette to codec */
    if (sh->bih && (sh->bih->biBitCount <= 8)) {
        avctx->palctrl = calloc(1, sizeof(AVPaletteControl));
        avctx->palctrl->palette_changed = 1;
        if (sh->bih->biSize-sizeof(BITMAPINFOHEADER))
            /* Palette size in biSize */
            memcpy(avctx->palctrl->palette, sh->bih+1,
                   FFMIN(sh->bih->biSize-sizeof(BITMAPINFOHEADER), AVPALETTE_SIZE));
        else
            /* Palette size in biClrUsed */
            memcpy(avctx->palctrl->palette, sh->bih+1,
                   FFMIN(sh->bih->biClrUsed * 4, AVPALETTE_SIZE));
        }

    if(sh->bih)
        avctx->bits_per_coded_sample= sh->bih->biBitCount;

    if(lavc_param_threads > 1)
        avcodec_thread_init(avctx, lavc_param_threads);
    /* open it */
    if (avcodec_open(avctx, lavc_codec) < 0) {
        mp_msg(MSGT_DECVIDEO, MSGL_ERR, MSGTR_CantOpenCodec);
        uninit(sh);
        return 0;
    }
    // this is necessary in case get_format was never called and init_vo is
    // too late e.g. for H.264 VDPAU
    set_format_params(avctx, avctx->pix_fmt);
    mp_msg(MSGT_DECVIDEO, MSGL_V, "INFO: libavcodec init OK!\n");
    return 1; //mpcodecs_config_vo(sh, sh->disp_w, sh->disp_h, IMGFMT_YV12);
}

// uninit driver
static void uninit(sh_video_t *sh){
    vd_ffmpeg_ctx *ctx = sh->context;
    AVCodecContext *avctx = ctx->avctx;

    if(lavc_param_vstats){
        int i;
        for(i=1; i<32; i++){
            mp_msg(MSGT_DECVIDEO, MSGL_INFO, "QP: %d, count: %d\n", i, ctx->qp_stat[i]);
        }
        mp_msg(MSGT_DECVIDEO, MSGL_INFO, MSGTR_MPCODECS_ArithmeticMeanOfQP,
            ctx->qp_sum / avctx->coded_frame->coded_picture_number,
            1.0/(ctx->inv_qp_sum / avctx->coded_frame->coded_picture_number)
            );
    }

    if (avctx) {
        if (avctx->codec && avcodec_close(avctx) < 0)
            mp_msg(MSGT_DECVIDEO, MSGL_ERR, MSGTR_CantCloseCodec);

        av_freep(&avctx->extradata);
        av_freep(&avctx->palctrl);
        av_freep(&avctx->slice_offset);
    }

    av_freep(&avctx);
    av_freep(&ctx->pic);
    if (ctx)
        free(ctx);
}

static void draw_slice(struct AVCodecContext *s,
                        const AVFrame *src, int offset[4],
                        int y, int type, int height){
    sh_video_t *sh = s->opaque;
    uint8_t *source[MP_MAX_PLANES]= {src->data[0] + offset[0], src->data[1] + offset[1], src->data[2] + offset[2]};
    int strides[MP_MAX_PLANES] = {src->linesize[0], src->linesize[1], src->linesize[2]};
#if 0
    int start=0, i;
    int width= s->width;
    int skip_stride= ((width<<lavc_param_lowres)+15)>>4;
    uint8_t *skip= &s->coded_frame->mbskip_table[(y>>4)*skip_stride];
    int threshold= s->coded_frame->age;
    if(s->pict_type!=B_TYPE){
        for(i=0; i*16<width+16; i++){
            if(i*16>=width || skip[i]>=threshold){
                if(start==i) start++;
                else{
                    uint8_t *src2[3]= {src[0] + start*16,
                                     src[1] + start*8,
                                     src[2] + start*8};
//printf("%2d-%2d x %d\n", start, i, y);
                    mpcodecs_draw_slice (sh, src2, stride, (i-start)*16, height, start*16, y);
                    start= i+1;
                }
            }
        }
    }else
#endif
    if (height < 0)
    {
        int i;
        height = -height;
        y -= height;
        for (i = 0; i < MP_MAX_PLANES; i++)
        {
            strides[i] = -strides[i];
            source[i] -= strides[i];
        }
    }
    if (y < sh->disp_h) {
        height = FFMIN(height, sh->disp_h-y);
        mpcodecs_draw_slice (sh, source, strides, sh->disp_w, height, 0, y);
    }
}


static int init_vo(sh_video_t *sh, enum PixelFormat pix_fmt){
    vd_ffmpeg_ctx *ctx = sh->context;
    AVCodecContext *avctx = ctx->avctx;
    float aspect= av_q2d(avctx->sample_aspect_ratio) * avctx->width / avctx->height;
    int width, height;

    width = avctx->width;
    height = avctx->height;

    // HACK!
    // if sh->ImageDesc is non-NULL, it means we decode QuickTime(tm) video.
    // use dimensions from BIH to avoid black borders at the right and bottom.
    if (sh->bih && sh->ImageDesc) {
        width = sh->bih->biWidth>>lavc_param_lowres;
        height = sh->bih->biHeight>>lavc_param_lowres;
    }

     // it is possible another vo buffers to be used after vo config()
     // lavc reset its buffers on width/heigh change but not on aspect change!!!
    if (av_cmp_q(avctx->sample_aspect_ratio, ctx->last_sample_aspect_ratio) ||
        width != sh->disp_w  ||
        height != sh->disp_h ||
        pix_fmt != ctx->pix_fmt ||
        !ctx->vo_initialized)
    {
        // this is a special-case HACK for MPEG-1/2 VDPAU that uses neither get_format nor
        // sets the value correctly in avcodec_open.
        set_format_params(avctx, avctx->pix_fmt);
        mp_msg(MSGT_DECVIDEO, MSGL_V, "[ffmpeg] aspect_ratio: %f\n", aspect);
        if (sh->aspect == 0 ||
            av_cmp_q(avctx->sample_aspect_ratio,
                     ctx->last_sample_aspect_ratio))
            sh->aspect = aspect;
        ctx->last_sample_aspect_ratio = avctx->sample_aspect_ratio;
        sh->disp_w = width;
        sh->disp_h = height;
        ctx->pix_fmt = pix_fmt;
        ctx->best_csp = pixfmt2imgfmt(pix_fmt);
        if (!mpcodecs_config_vo(sh, sh->disp_w, sh->disp_h, ctx->best_csp))
            return -1;
        ctx->vo_initialized = 1;
    }
    return 0;
}

static int get_buffer(AVCodecContext *avctx, AVFrame *pic){
    sh_video_t *sh = avctx->opaque;
    vd_ffmpeg_ctx *ctx = sh->context;
    mp_image_t *mpi=NULL;
    int flags= MP_IMGFLAG_ACCEPT_STRIDE | MP_IMGFLAG_PREFER_ALIGNED_STRIDE;
    int type= MP_IMGTYPE_IPB;
    int width= avctx->width;
    int height= avctx->height;
    avcodec_align_dimensions(avctx, &width, &height);
//printf("get_buffer %d %d %d\n", pic->reference, ctx->ip_count, ctx->b_count);

    if (pic->buffer_hints) {
        mp_msg(MSGT_DECVIDEO, MSGL_DBG2, "Buffer hints: %u\n", pic->buffer_hints);
        type = MP_IMGTYPE_TEMP;
        if (pic->buffer_hints & FF_BUFFER_HINTS_READABLE)
            flags |= MP_IMGFLAG_READABLE;
        if (pic->buffer_hints & FF_BUFFER_HINTS_PRESERVE) {
            type = MP_IMGTYPE_STATIC;
            flags |= MP_IMGFLAG_PRESERVE;
        }
        if (pic->buffer_hints & FF_BUFFER_HINTS_REUSABLE) {
            type = MP_IMGTYPE_STATIC;
            flags |= MP_IMGFLAG_PRESERVE;
        }
        flags|=(!avctx->hurry_up && ctx->do_slices) ?
                 MP_IMGFLAG_DRAW_CALLBACK:0;
        mp_msg(MSGT_DECVIDEO, MSGL_DBG2, type == MP_IMGTYPE_STATIC ? "using STATIC\n" : "using TEMP\n");
    } else {
        if(!pic->reference){
            ctx->b_count++;
            flags|=(!avctx->hurry_up && ctx->do_slices) ?
                     MP_IMGFLAG_DRAW_CALLBACK:0;
        }else{
            ctx->ip_count++;
            flags|= MP_IMGFLAG_PRESERVE|MP_IMGFLAG_READABLE
                      | (ctx->do_slices ? MP_IMGFLAG_DRAW_CALLBACK : 0);
        }
    }

    if(init_vo(sh, avctx->pix_fmt) < 0){
        avctx->release_buffer= avcodec_default_release_buffer;
        avctx->get_buffer= avcodec_default_get_buffer;
        return avctx->get_buffer(avctx, pic);
    }

    if (IMGFMT_IS_XVMC(ctx->best_csp) || IMGFMT_IS_VDPAU(ctx->best_csp)) {
        type =  MP_IMGTYPE_NUMBERED | (0xffff << 16);
    } else
    if (!pic->buffer_hints) {
        if(ctx->b_count>1 || ctx->ip_count>2){
            mp_msg(MSGT_DECVIDEO, MSGL_WARN, MSGTR_MPCODECS_DRIFailure);

            ctx->do_dr1=0; //FIXME
            avctx->get_buffer= avcodec_default_get_buffer;
            return avctx->get_buffer(avctx, pic);
        }

        if(avctx->has_b_frames){
            type= MP_IMGTYPE_IPB;
        }else{
            type= MP_IMGTYPE_IP;
        }
        mp_msg(MSGT_DECVIDEO, MSGL_DBG2, type== MP_IMGTYPE_IPB ? "using IPB\n" : "using IP\n");
    }

    if (ctx->best_csp == IMGFMT_RGB8 || ctx->best_csp == IMGFMT_BGR8)
        flags |= MP_IMGFLAG_RGB_PALETTE;
    mpi= mpcodecs_get_image(sh, type, flags, width, height);
    if (!mpi) return -1;

    // ok, let's see what did we get:
    if(mpi->flags&MP_IMGFLAG_DRAW_CALLBACK &&
       !(mpi->flags&MP_IMGFLAG_DIRECT)){
        // nice, filter/vo likes draw_callback :)
        avctx->draw_horiz_band= draw_slice;
    } else
        avctx->draw_horiz_band= NULL;
    if(IMGFMT_IS_VDPAU(mpi->imgfmt)) {
        avctx->draw_horiz_band= draw_slice;
    }
#if CONFIG_XVMC
    if(IMGFMT_IS_XVMC(mpi->imgfmt)) {
        struct xvmc_pix_fmt *render = mpi->priv; //same as data[2]
        avctx->draw_horiz_band= draw_slice;
        if(!avctx->xvmc_acceleration) {
            mp_msg(MSGT_DECVIDEO, MSGL_INFO, MSGTR_MPCODECS_McGetBufferShouldWorkOnlyWithXVMC);
            assert(0);
            exit(1);
//            return -1;//!!fixme check error conditions in ffmpeg
        }
        if(!(mpi->flags & MP_IMGFLAG_DIRECT)) {
            mp_msg(MSGT_DECVIDEO, MSGL_ERR, MSGTR_MPCODECS_OnlyBuffersAllocatedByVoXvmcAllowed);
            assert(0);
            exit(1);
//            return -1;//!!fixme check error conditions in ffmpeg
        }
        if(mp_msg_test(MSGT_DECVIDEO, MSGL_DBG5))
            mp_msg(MSGT_DECVIDEO, MSGL_DBG5, "vd_ffmpeg::get_buffer (xvmc render=%p)\n", render);
        assert(render != 0);
        assert(render->xvmc_id == AV_XVMC_ID);
        render->state |= AV_XVMC_STATE_PREDICTION;
    }
#endif

    pic->data[0]= mpi->planes[0];
    pic->data[1]= mpi->planes[1];
    pic->data[2]= mpi->planes[2];
    pic->data[3]= mpi->planes[3];

#if 0
    assert(mpi->width >= ((width +align)&(~align)));
    assert(mpi->height >= ((height+align)&(~align)));
    assert(mpi->stride[0] >= mpi->width);
    if(mpi->imgfmt==IMGFMT_I420 || mpi->imgfmt==IMGFMT_YV12 || mpi->imgfmt==IMGFMT_IYUV){
        const int y_size= mpi->stride[0] * (mpi->h-1) + mpi->w;
        const int c_size= mpi->stride[1] * ((mpi->h>>1)-1) + (mpi->w>>1);

        assert(mpi->planes[0] > mpi->planes[1] || mpi->planes[0] + y_size <= mpi->planes[1]);
        assert(mpi->planes[0] > mpi->planes[2] || mpi->planes[0] + y_size <= mpi->planes[2]);
        assert(mpi->planes[1] > mpi->planes[0] || mpi->planes[1] + c_size <= mpi->planes[0]);
        assert(mpi->planes[1] > mpi->planes[2] || mpi->planes[1] + c_size <= mpi->planes[2]);
        assert(mpi->planes[2] > mpi->planes[0] || mpi->planes[2] + c_size <= mpi->planes[0]);
        assert(mpi->planes[2] > mpi->planes[1] || mpi->planes[2] + c_size <= mpi->planes[1]);
    }
#endif

    /* Note, some (many) codecs in libavcodec must have stride1==stride2 && no changes between frames
     * lavc will check that and die with an error message, if its not true
     */
    pic->linesize[0]= mpi->stride[0];
    pic->linesize[1]= mpi->stride[1];
    pic->linesize[2]= mpi->stride[2];
    pic->linesize[3]= mpi->stride[3];

    pic->opaque = mpi;
//printf("%X\n", (int)mpi->planes[0]);
#if 0
if(mpi->flags&MP_IMGFLAG_DIRECT)
    printf("D");
else if(mpi->flags&MP_IMGFLAG_DRAW_CALLBACK)
    printf("S");
else
    printf(".");
#endif
    if(pic->reference){
        pic->age= ctx->ip_age[0];

        ctx->ip_age[0]= ctx->ip_age[1]+1;
        ctx->ip_age[1]= 1;
        ctx->b_age++;
    }else{
        pic->age= ctx->b_age;

        ctx->ip_age[0]++;
        ctx->ip_age[1]++;
        ctx->b_age=1;
    }
    pic->type= FF_BUFFER_TYPE_USER;
    return 0;
}

static void release_buffer(struct AVCodecContext *avctx, AVFrame *pic){
    mp_image_t *mpi= pic->opaque;
    sh_video_t *sh = avctx->opaque;
    vd_ffmpeg_ctx *ctx = sh->context;
    int i;

//printf("release buffer %d %d %d\n", mpi ? mpi->flags&MP_IMGFLAG_PRESERVE : -99, ctx->ip_count, ctx->b_count);

    if(ctx->ip_count <= 2 && ctx->b_count<=1){
        if(mpi->flags&MP_IMGFLAG_PRESERVE)
            ctx->ip_count--;
        else
            ctx->b_count--;
    }

    if (mpi) {
        // Palette support: free palette buffer allocated in get_buffer
        if (mpi->bpp == 8)
            av_freep(&mpi->planes[1]);
#if CONFIG_XVMC
        if (IMGFMT_IS_XVMC(mpi->imgfmt)) {
            struct xvmc_pix_fmt *render = (struct xvmc_pix_fmt*)pic->data[2]; //same as mpi->priv
            if(mp_msg_test(MSGT_DECVIDEO, MSGL_DBG5))
                mp_msg(MSGT_DECVIDEO, MSGL_DBG5, "vd_ffmpeg::release_buffer (xvmc render=%p)\n", render);
            assert(render!=NULL);
            assert(render->xvmc_id == AV_XVMC_ID);
            render->state&=~AV_XVMC_STATE_PREDICTION;
        }
#endif
        // release mpi (in case MPI_IMGTYPE_NUMBERED is used, e.g. for VDPAU)
        mpi->usage_count--;
    }

    if(pic->type!=FF_BUFFER_TYPE_USER){
        avcodec_default_release_buffer(avctx, pic);
        return;
    }

    for(i=0; i<4; i++){
        pic->data[i]= NULL;
    }
//printf("R%X %X\n", pic->linesize[0], pic->data[0]);
}

// copypaste from demux_real.c - it should match to get it working!
//FIXME put into some header
typedef struct dp_hdr_s {
    uint32_t chunks;        // number of chunks
    uint32_t timestamp; // timestamp from packet header
    uint32_t len;        // length of actual data
    uint32_t chunktab;        // offset to chunk offset array
} dp_hdr_t;

static av_unused void swap_palette(void *pal)
{
    int i;
    uint32_t *p = pal;
    for (i = 0; i < AVPALETTE_COUNT; i++)
        p[i] = le2me_32(p[i]);
}

// decode a frame
static mp_image_t *decode(sh_video_t *sh, void *data, int len, int flags){
    int got_picture=0;
    int ret;
    vd_ffmpeg_ctx *ctx = sh->context;
    AVFrame *pic= ctx->pic;
    AVCodecContext *avctx = ctx->avctx;
    mp_image_t *mpi=NULL;
    int dr1= ctx->do_dr1;
    AVPacket pkt;

    if(len<=0) return NULL; // skipped frame

//ffmpeg interlace (mpeg2) bug have been fixed. no need of -noslices
    if (!dr1)
    avctx->draw_horiz_band=NULL;
    if(ctx->vo_initialized && !(flags&3) && !dr1){
        mpi=mpcodecs_get_image(sh, MP_IMGTYPE_EXPORT, MP_IMGFLAG_PRESERVE |
            (ctx->do_slices?MP_IMGFLAG_DRAW_CALLBACK:0),
            sh->disp_w, sh->disp_h);
        if(mpi && mpi->flags&MP_IMGFLAG_DRAW_CALLBACK){
            // vd core likes slices!
            avctx->draw_horiz_band=draw_slice;
        }
    }

    avctx->hurry_up=(flags&3)?((flags&2)?2:1):0;

    mp_msg(MSGT_DECVIDEO, MSGL_DBG2, "vd_ffmpeg data: %04x, %04x, %04x, %04x\n",
           ((int *)data)[0], ((int *)data)[1], ((int *)data)[2], ((int *)data)[3]);
    av_init_packet(&pkt);
    pkt.data = data;
    pkt.size = len;
    // HACK: make PNGs decode normally instead of as CorePNG delta frames
    pkt.flags = PKT_FLAG_KEY;
    ret = avcodec_decode_video2(avctx, pic, &got_picture, &pkt);

    dr1= ctx->do_dr1;
    if(ret<0) mp_msg(MSGT_DECVIDEO, MSGL_WARN, "Error while decoding frame!\n");
//printf("repeat: %d\n", pic->repeat_pict);
//-- vstats generation
    while(lavc_param_vstats){ // always one time loop
        static FILE *fvstats=NULL;
        char filename[20];
        static long long int all_len=0;
        static int frame_number=0;
        static double all_frametime=0.0;
        AVFrame *pic= avctx->coded_frame;
        double quality=0.0;

        if(!fvstats) {
            time_t today2;
            struct tm *today;
            today2 = time(NULL);
            today = localtime(&today2);
            sprintf(filename, "vstats_%02d%02d%02d.log", today->tm_hour,
                today->tm_min, today->tm_sec);
            fvstats = fopen(filename, "w");
            if(!fvstats) {
                perror("fopen");
                lavc_param_vstats=0; // disable block
                break;
                /*exit(1);*/
            }
        }

        // average MB quantizer
        {
            int x, y;
            int w = ((avctx->width  << lavc_param_lowres)+15) >> 4;
            int h = ((avctx->height << lavc_param_lowres)+15) >> 4;
            int8_t *q = pic->qscale_table;
            for(y = 0; y < h; y++) {
                for(x = 0; x < w; x++)
                    quality += (double)*(q+x);
                q += pic->qstride;
            }
            quality /= w * h;
        }

        all_len+=len;
        all_frametime+=sh->frametime;
        fprintf(fvstats, "frame= %5d q= %2.2f f_size= %6d s_size= %8.0fkB ",
            ++frame_number, quality, len, (double)all_len/1024);
        fprintf(fvstats, "time= %0.3f br= %7.1fkbits/s avg_br= %7.1fkbits/s ",
           all_frametime, (double)(len*8)/sh->frametime/1000.0,
           (double)(all_len*8)/all_frametime/1000.0);
        switch(pic->pict_type){
        case FF_I_TYPE:
            fprintf(fvstats, "type= I\n");
            break;
        case FF_P_TYPE:
            fprintf(fvstats, "type= P\n");
            break;
        case FF_S_TYPE:
            fprintf(fvstats, "type= S\n");
            break;
        case FF_B_TYPE:
            fprintf(fvstats, "type= B\n");
            break;
        default:
            fprintf(fvstats, "type= ? (%d)\n", pic->pict_type);
            break;
        }

        ctx->qp_stat[(int)(quality+0.5)]++;
        ctx->qp_sum += quality;
        ctx->inv_qp_sum += 1.0/(double)quality;

        break;
    }
//--

    if(!got_picture) {
	if (avctx->codec->id == CODEC_ID_H264)
	    return &mpi_no_picture; // H.264 first field only
	else
	    return NULL;    // skipped image
    }

    if(init_vo(sh, avctx->pix_fmt) < 0) return NULL;

    if(dr1 && pic->opaque){
        mpi= (mp_image_t *)pic->opaque;
    }

    if(!mpi)
    mpi=mpcodecs_get_image(sh, MP_IMGTYPE_EXPORT, MP_IMGFLAG_PRESERVE,
        avctx->width, avctx->height);
    if(!mpi){        // temporary!
        mp_msg(MSGT_DECVIDEO, MSGL_WARN, MSGTR_MPCODECS_CouldntAllocateImageForCodec);
        return NULL;
    }

    if(!dr1){
        mpi->planes[0]=pic->data[0];
        mpi->planes[1]=pic->data[1];
        mpi->planes[2]=pic->data[2];
        mpi->planes[3]=pic->data[3];
        mpi->stride[0]=pic->linesize[0];
        mpi->stride[1]=pic->linesize[1];
        mpi->stride[2]=pic->linesize[2];
        mpi->stride[3]=pic->linesize[3];
    }

    if (!mpi->planes[0])
        return NULL;

    if(avctx->pix_fmt==PIX_FMT_YUV422P && mpi->chroma_y_shift==1){
        // we have 422p but user wants 420p
        mpi->stride[1]*=2;
        mpi->stride[2]*=2;
    }

#if HAVE_BIGENDIAN
    // FIXME: this might cause problems for buffers with FF_BUFFER_HINTS_PRESERVE
    if (mpi->bpp == 8)
        swap_palette(mpi->planes[1]);
#endif
/* to comfirm with newer lavc style */
    mpi->qscale =pic->qscale_table;
    mpi->qstride=pic->qstride;
    mpi->pict_type=pic->pict_type;
    mpi->qscale_type= pic->qscale_type;
    mpi->fields = MP_IMGFIELD_ORDERED;
    if(pic->interlaced_frame) mpi->fields |= MP_IMGFIELD_INTERLACED;
    if(pic->top_field_first ) mpi->fields |= MP_IMGFIELD_TOP_FIRST;
    if(pic->repeat_pict == 1) mpi->fields |= MP_IMGFIELD_REPEAT_FIRST;

    return mpi;
}

#if CONFIG_XVMC || CONFIG_VDPAU
static enum PixelFormat get_format(struct AVCodecContext *avctx,
                                    const enum PixelFormat *fmt){
    enum PixelFormat selected_format;
    int imgfmt;
    sh_video_t *sh = avctx->opaque;
    int i;

    for(i=0;fmt[i]!=PIX_FMT_NONE;i++){
        imgfmt = pixfmt2imgfmt(fmt[i]);
        if(!IMGFMT_IS_XVMC(imgfmt) && !IMGFMT_IS_VDPAU(imgfmt)) continue;
        mp_msg(MSGT_DECVIDEO, MSGL_INFO, MSGTR_MPCODECS_TryingPixfmt, i);
        if(init_vo(sh, fmt[i]) >= 0) {
            break;
        }
    }
    selected_format = fmt[i];
    set_format_params(avctx, selected_format);
    return selected_format;
}
#endif /* CONFIG_XVMC || CONFIG_VDPAU */
