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

#include <stdlib.h>
#include "stream/stream.h"
#include "libmpdemux/demuxer.h"
#include "libmpdemux/stheader.h"
#include "mplayer.h"
#include "libvo/sub.h"
#include "libvo/video_out.h"
#include "cpudetect.h"
#include "help_mp.h"
#include "mp_msg.h"
#include "spudec.h"
#include "version.h"
#include "vobsub.h"
#include "av_sub.h"
#include "libmpcodecs/dec_teletext.h"
#include "libavutil/intreadwrite.h"
#include "m_option.h"
#include "mpcommon.h"

double sub_last_pts = -303;

#ifdef CONFIG_ASS
#include "libass/ass_mp.h"
ASS_Track* ass_track = 0; // current track to render
#endif

sub_data* subdata = NULL;
subtitle* vo_sub_last = NULL;

const char *mencoder_version = "MEncoder " VERSION;
const char *mplayer_version  = "MPlayer "  VERSION;

void print_version(const char* name)
{
    mp_msg(MSGT_CPLAYER, MSGL_INFO, MP_TITLE, name);

    /* Test for CPU capabilities (and corresponding OS support) for optimizing */
    GetCpuCaps(&gCpuCaps);
#if ARCH_X86
    mp_msg(MSGT_CPLAYER, MSGL_V,
	   "CPUflags:  MMX: %d MMX2: %d 3DNow: %d 3DNowExt: %d SSE: %d SSE2: %d SSSE3: %d\n",
	   gCpuCaps.hasMMX, gCpuCaps.hasMMX2,
	   gCpuCaps.has3DNow, gCpuCaps.has3DNowExt,
	   gCpuCaps.hasSSE, gCpuCaps.hasSSE2, gCpuCaps.hasSSSE3);
#if CONFIG_RUNTIME_CPUDETECT
    mp_msg(MSGT_CPLAYER,MSGL_V, MSGTR_CompiledWithRuntimeDetection);
#else
    mp_msg(MSGT_CPLAYER,MSGL_V, MSGTR_CompiledWithCPUExtensions);
if (HAVE_MMX)
    mp_msg(MSGT_CPLAYER,MSGL_V," MMX");
if (HAVE_MMX2)
    mp_msg(MSGT_CPLAYER,MSGL_V," MMX2");
if (HAVE_AMD3DNOW)
    mp_msg(MSGT_CPLAYER,MSGL_V," 3DNow");
if (HAVE_AMD3DNOWEXT)
    mp_msg(MSGT_CPLAYER,MSGL_V," 3DNowExt");
if (HAVE_SSE)
    mp_msg(MSGT_CPLAYER,MSGL_V," SSE");
if (HAVE_SSE2)
    mp_msg(MSGT_CPLAYER,MSGL_V," SSE2");
if (HAVE_SSSE3)
    mp_msg(MSGT_CPLAYER,MSGL_V," SSSE3");
if (HAVE_CMOV)
    mp_msg(MSGT_CPLAYER,MSGL_V," CMOV");
    mp_msg(MSGT_CPLAYER,MSGL_V,"\n");
#endif /* CONFIG_RUNTIME_CPUDETECT */
#endif /* ARCH_X86 */
}

static int is_text_sub(int type)
{
    return type == 't' || type == 'm' || type == 'a';
}

static int is_av_sub(int type)
{
    return type == 'b' || type == 'p' || type == 'x';
}

void update_subtitles(sh_video_t *sh_video, double refpts, demux_stream_t *d_dvdsub, int reset)
{
    double curpts = refpts - sub_delay;
    unsigned char *packet=NULL;
    int len;
    int type = d_dvdsub->sh ? ((sh_sub_t *)d_dvdsub->sh)->type : 'v';
    static subtitle subs;
    if (reset) {
        sub_clear_text(&subs, MP_NOPTS_VALUE);
        if (vo_sub) {
            set_osd_subtitle(NULL);
        }
        if (vo_spudec) {
            spudec_reset(vo_spudec);
            vo_osd_changed(OSDTYPE_SPU);
        }
#ifdef CONFIG_LIBAVCODEC
        if (is_av_sub(type))
            reset_avsub(d_dvdsub->sh);
#endif
    }
    // find sub
    if (subdata) {
        if (sub_fps==0) sub_fps = sh_video ? sh_video->fps : 25;
        current_module = "find_sub";
        if (refpts > sub_last_pts || refpts < sub_last_pts-1.0) {
            find_sub(subdata, curpts *
                     (subdata->sub_uses_time ? 100. : sub_fps));
            if (vo_sub) vo_sub_last = vo_sub;
            // FIXME! frame counter...
            sub_last_pts = refpts;
        }
    }

    // DVD sub:
    if (vo_config_count &&
        (vobsub_id >= 0 || type == 'v')) {
        int timestamp;
        current_module = "spudec";
        /* Get a sub packet from the DVD or a vobsub */
        while(1) {
            // Vobsub
            len = 0;
            if (vo_vobsub) {
                if (curpts >= 0) {
                    len = vobsub_get_packet(vo_vobsub, curpts,
                                            (void**)&packet, &timestamp);
                    if (len > 0) {
                        mp_dbg(MSGT_CPLAYER,MSGL_V,"\rVOB sub: len=%d v_pts=%5.3f v_timer=%5.3f sub=%5.3f ts=%d \n",len,refpts,sh_video->timer,timestamp / 90000.0,timestamp);
                    }
                }
            } else {
                // DVD sub
                len = ds_get_packet_sub(d_dvdsub, (unsigned char**)&packet, NULL, NULL);
                if (len > 0) {
                    // XXX This is wrong, sh_video->pts can be arbitrarily
                    // much behind demuxing position. Unfortunately using
                    // d_video->pts which would have been the simplest
                    // improvement doesn't work because mpeg specific hacks
                    // in video.c set d_video->pts to 0.
                    float x = d_dvdsub->pts - refpts;
                    if (x > -20 && x < 20) // prevent missing subs on pts reset
                        timestamp = 90000*d_dvdsub->pts;
                    else timestamp = 90000*curpts;
                    mp_dbg(MSGT_CPLAYER, MSGL_V, "\rDVD sub: len=%d  "
                           "v_pts=%5.3f  s_pts=%5.3f  ts=%d \n", len,
                           refpts, d_dvdsub->pts, timestamp);
                }
            }
            if (len<=0 || !packet) break;
            // create it only here, since with some broken demuxers we might
            // type = v but no DVD sub and we currently do not change the
            // "original frame size" ever after init, leading to wrong-sized
            // PGS subtitles.
            if (!vo_spudec)
                vo_spudec = spudec_new(NULL);
            if (vo_vobsub || timestamp >= 0)
                spudec_assemble(vo_spudec, packet, len, timestamp);
        }
    } else if (is_text_sub(type) || is_av_sub(type) || type == 'd') {
        int orig_type = type;
        double endpts;
        if (type == 'd' && !d_dvdsub->demuxer->teletext) {
            tt_stream_props tsp = {0};
            void *ptr = &tsp;
            if (teletext_control(NULL, TV_VBI_CONTROL_START, &ptr) == VBI_CONTROL_TRUE)
                d_dvdsub->demuxer->teletext = ptr;
        }
        if (d_dvdsub->non_interleaved)
            ds_get_next_pts(d_dvdsub);
        while (1) {
            double subpts = curpts;
            type = orig_type;
            len = ds_get_packet_sub(d_dvdsub, &packet, &subpts, &endpts);
            if (len < 0)
                break;
            if (is_av_sub(type)) {
#ifdef CONFIG_LIBAVCODEC
                type = decode_avsub(d_dvdsub->sh, &packet, &len, &subpts, &endpts);
                if (type <= 0)
#endif
                    continue;
            }
            if (type == 'm') {
                if (len < 2) continue;
                len = FFMIN(len - 2, AV_RB16(packet));
                packet += 2;
            }
            if (type == 'd') {
                if (d_dvdsub->demuxer->teletext) {
                    uint8_t *p = packet;
                    p++;
                    len--;
                    while (len >= 46) {
                        int sublen = p[1];
                        if (p[0] == 2 || p[0] == 3)
                            teletext_control(d_dvdsub->demuxer->teletext,
                                TV_VBI_CONTROL_DECODE_DVB, p + 2);
                        p   += sublen + 2;
                        len -= sublen + 2;
                    }
                }
                continue;
            }
#ifdef CONFIG_ASS
            if (ass_enabled) {
                sh_sub_t* sh = d_dvdsub->sh;
                ass_track = sh ? sh->ass_track : NULL;
                if (!ass_track) continue;
                if (type == 'a') { // ssa/ass subs with libass
                    if (len > 10 && memcmp(packet, "Dialogue: ", 10) == 0)
                        ass_process_data(ass_track, packet, len);
                    else
                        ass_process_chunk(ass_track, packet, len,
                                          (long long)(subpts*1000 + 0.5),
                                          (long long)((endpts-subpts)*1000 + 0.5));
                } else { // plaintext subs with libass
                    if (subpts != MP_NOPTS_VALUE) {
                        subtitle tmp_subs = {0};
                        if (endpts == MP_NOPTS_VALUE) endpts = subpts + 3;
                        sub_add_text(&tmp_subs, packet, len, endpts, 0);
                        tmp_subs.start = subpts * 100;
                        tmp_subs.end = endpts * 100;
                        ass_process_subtitle(ass_track, &tmp_subs);
                        sub_clear_text(&tmp_subs, MP_NOPTS_VALUE);
                    }
                }
                continue;
            }
#endif
            if (subpts != MP_NOPTS_VALUE) {
                if (endpts == MP_NOPTS_VALUE)
                    sub_clear_text(&subs, MP_NOPTS_VALUE);
                if (type == 'a') { // ssa/ass subs without libass => convert to plaintext
                    int i;
                    unsigned char* p = packet;
                    int skip_commas = 8;
                    if (len > 10 && memcmp(packet, "Dialogue: ", 10) == 0)
                        skip_commas = 9;
                    for (i=0; i < skip_commas && *p != '\0'; p++)
                        if (*p == ',')
                            i++;
                    if (*p == '\0')  /* Broken line? */
                        continue;
                    len -= p - packet;
                    packet = p;
                }
                sub_add_text(&subs, packet, len, endpts, 1);
                set_osd_subtitle(&subs);
            }
            if (d_dvdsub->non_interleaved)
                ds_get_next_pts(d_dvdsub);
        }
        if (sub_clear_text(&subs, curpts))
            set_osd_subtitle(&subs);
    }
    if (vo_spudec) {
        spudec_heartbeat(vo_spudec, 90000*curpts);
        if (spudec_changed(vo_spudec))
            vo_osd_changed(OSDTYPE_SPU);
    }

    current_module=NULL;
}

void update_teletext(sh_video_t *sh_video, demuxer_t *demuxer, int reset)
{
    int page_changed;

    if (!demuxer->teletext)
        return;

    //Also forcing page update when such ioctl is not supported or call error occured
    if(teletext_control(demuxer->teletext,TV_VBI_CONTROL_IS_CHANGED,&page_changed)!=VBI_CONTROL_TRUE)
        page_changed=1;

    if(!page_changed)
        return;

    if(teletext_control(demuxer->teletext,TV_VBI_CONTROL_GET_VBIPAGE,&vo_osd_teletext_page)!=VBI_CONTROL_TRUE)
        vo_osd_teletext_page=NULL;
    if(teletext_control(demuxer->teletext,TV_VBI_CONTROL_GET_HALF_PAGE,&vo_osd_teletext_half)!=VBI_CONTROL_TRUE)
        vo_osd_teletext_half=0;
    if(teletext_control(demuxer->teletext,TV_VBI_CONTROL_GET_MODE,&vo_osd_teletext_mode)!=VBI_CONTROL_TRUE)
        vo_osd_teletext_mode=0;
    if(teletext_control(demuxer->teletext,TV_VBI_CONTROL_GET_FORMAT,&vo_osd_teletext_format)!=VBI_CONTROL_TRUE)
        vo_osd_teletext_format=0;
    vo_osd_changed(OSDTYPE_TELETEXT);

    teletext_control(demuxer->teletext,TV_VBI_CONTROL_MARK_UNCHANGED,NULL);
}

int select_audio(demuxer_t* demuxer, int audio_id, char* audio_lang)
{
    if (audio_id == -1 && audio_lang)
        audio_id = demuxer_audio_track_by_lang(demuxer, audio_lang);
    if (audio_id == -1)
        audio_id = demuxer_default_audio_track(demuxer);
    if (audio_id != -1) // -1 (automatic) is the default behaviour of demuxers
        demuxer_switch_audio(demuxer, audio_id);
    if (audio_id == -2) { // some demuxers don't yet know how to switch to no sound
        demuxer->audio->id = -2;
        demuxer->audio->sh = NULL;
    }
    return demuxer->audio->id;
}

/* Parse -noconfig common to both programs */
int disable_system_conf=0;
int disable_user_conf=0;
#ifdef CONFIG_GUI
int disable_gui_conf=0;
#endif /* CONFIG_GUI */

/* Disable all configuration files */
static void noconfig_all(void)
{
    disable_system_conf = 1;
    disable_user_conf = 1;
#ifdef CONFIG_GUI
    disable_gui_conf = 1;
#endif /* CONFIG_GUI */
}

const m_option_t noconfig_opts[] = {
    {"all", noconfig_all, CONF_TYPE_FUNC, CONF_GLOBAL|CONF_NOCFG|CONF_PRE_PARSE, 0, 0, NULL},
    {"system", &disable_system_conf, CONF_TYPE_FLAG, CONF_GLOBAL|CONF_NOCFG|CONF_PRE_PARSE, 0, 1, NULL},
    {"user", &disable_user_conf, CONF_TYPE_FLAG, CONF_GLOBAL|CONF_NOCFG|CONF_PRE_PARSE, 0, 1, NULL},
#ifdef CONFIG_GUI
    {"gui", &disable_gui_conf, CONF_TYPE_FLAG, CONF_GLOBAL|CONF_NOCFG|CONF_PRE_PARSE, 0, 1, NULL},
#endif /* CONFIG_GUI */
    {NULL, NULL, 0, 0, 0, 0, NULL}
};

