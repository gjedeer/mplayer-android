LOCAL_PATH := $(call my-dir)

# If targetting ARM CPUs, build the ARMv7 detection
# function. It will only be called by the main
# library if we detect ARMv7 through a system property.
#
include $(CLEAR_VARS)
LOCAL_MODULE := mplayer

include config.mak

###### variable declarations #######

SRCS_AUDIO_INPUT-$(ALSA1X)           += stream/ai_alsa1x.c
SRCS_AUDIO_INPUT-$(ALSA9)            += stream/ai_alsa.c
SRCS_AUDIO_INPUT-$(OSS)              += stream/ai_oss.c
SRCS_COMMON-$(AUDIO_INPUT)           += $(SRCS_AUDIO_INPUT-yes)
SRCS_COMMON-$(BITMAP_FONT)           += libvo/font_load.c
SRCS_COMMON-$(CDDA)                  += stream/stream_cdda.c \
                                        stream/cdinfo.c
SRCS_COMMON-$(CDDB)                  += stream/stream_cddb.c
SRCS_COMMON-$(DVBIN)                 += stream/dvb_tune.c \
                                        stream/stream_dvb.c
SRCS_COMMON-$(DVDNAV)                += stream/stream_dvdnav.c
SRCS_COMMON-$(DVDNAV_INTERNAL)       += libdvdnav/dvdnav.c \
                                        libdvdnav/highlight.c \
                                        libdvdnav/navigation.c \
                                        libdvdnav/read_cache.c \
                                        libdvdnav/remap.c \
                                        libdvdnav/searching.c \
                                        libdvdnav/settings.c \
                                        libdvdnav/vm/decoder.c \
                                        libdvdnav/vm/vm.c \
                                        libdvdnav/vm/vmcmd.c \

SRCS_COMMON-$(DVDREAD)               += stream/stream_dvd.c \
                                        stream/stream_dvd_common.c
SRCS_COMMON-$(DVDREAD_INTERNAL)      += libdvdread4/bitreader.c \
                                        libdvdread4/dvd_input.c \
                                        libdvdread4/dvd_reader.c \
                                        libdvdread4/dvd_udf.c \
                                        libdvdread4/ifo_print.c \
                                        libdvdread4/ifo_read.c \
                                        libdvdread4/md5.c \
                                        libdvdread4/nav_print.c \
                                        libdvdread4/nav_read.c \

SRCS_COMMON-$(FAAD)                  += libmpcodecs/ad_faad.c
SRCS_COMMON-$(FAAD_INTERNAL)         += libfaad2/bits.c \
                                        libfaad2/cfft.c \
                                        libfaad2/common.c \
                                        libfaad2/decoder.c \
                                        libfaad2/drc.c \
                                        libfaad2/drm_dec.c \
                                        libfaad2/error.c \
                                        libfaad2/filtbank.c \
                                        libfaad2/hcr.c \
                                        libfaad2/huffman.c \
                                        libfaad2/ic_predict.c \
                                        libfaad2/is.c \
                                        libfaad2/lt_predict.c \
                                        libfaad2/mdct.c \
                                        libfaad2/mp4.c \
                                        libfaad2/ms.c \
                                        libfaad2/output.c \
                                        libfaad2/pns.c \
                                        libfaad2/ps_dec.c \
                                        libfaad2/ps_syntax.c  \
                                        libfaad2/pulse.c \
                                        libfaad2/rvlc.c \
                                        libfaad2/sbr_dct.c \
                                        libfaad2/sbr_dec.c \
                                        libfaad2/sbr_e_nf.c \
                                        libfaad2/sbr_fbt.c \
                                        libfaad2/sbr_hfadj.c \
                                        libfaad2/sbr_hfgen.c \
                                        libfaad2/sbr_huff.c \
                                        libfaad2/sbr_qmf.c \
                                        libfaad2/sbr_syntax.c \
                                        libfaad2/sbr_tf_grid.c \
                                        libfaad2/specrec.c \
                                        libfaad2/ssr.c \
                                        libfaad2/ssr_fb.c \
                                        libfaad2/ssr_ipqf.c \
                                        libfaad2/syntax.c \
                                        libfaad2/tns.c \

SRCS_COMMON-$(FASTMEMCPY)            += libvo/aclib.c
SRCS_COMMON-$(FREETYPE)              += libvo/font_load_ft.c
SRCS_COMMON-$(FTP)                   += stream/stream_ftp.c
SRCS_COMMON-$(GIF)                   += libmpdemux/demux_gif.c
SRCS_COMMON-$(HAVE_POSIX_SELECT)     += libmpcodecs/vf_bmovl.c
SRCS_COMMON-$(HAVE_SYS_MMAN_H)       += libaf/af_export.c osdep/mmap_anon.c
SRCS_COMMON-$(JPEG)                  += libmpcodecs/vd_ijpg.c
SRCS_COMMON-$(LADSPA)                += libaf/af_ladspa.c
SRCS_COMMON-$(LIBA52)                += libmpcodecs/ad_liba52.c
SRCS_COMMON-$(LIBASS)                += libmpcodecs/vf_ass.c \
                                        libass/ass_mp.c \
                                        subassconvert.c \

SRCS_COMMON-$(LIBASS_INTERNAL)       += libass/ass.c \
                                        libass/ass_bitmap.c \
                                        libass/ass_cache.c \
                                        libass/ass_drawing.c \
                                        libass/ass_font.c \
                                        libass/ass_fontconfig.c \
                                        libass/ass_library.c \
                                        libass/ass_parse.c \
                                        libass/ass_render.c \
                                        libass/ass_render_api.c \
                                        libass/ass_strtod.c \
                                        libass/ass_utils.c \

SRCS_COMMON-$(LIBAVCODEC)            += av_opts.c \
                                        av_sub.c \
                                        libaf/af_lavcresample.c \
                                        libmpcodecs/ad_ffmpeg.c \
                                        libmpcodecs/vd_ffmpeg.c \
                                        libmpcodecs/vf_lavc.c \
                                        libmpcodecs/vf_lavcdeint.c \
                                        libmpcodecs/vf_screenshot.c \

# These filters use private headers and do not work with shared libavcodec.
SRCS_COMMON-$(LIBAVCODEC_A)          += libaf/af_lavcac3enc.c \
                                        libmpcodecs/vf_fspp.c \
                                        libmpcodecs/vf_geq.c \
                                        libmpcodecs/vf_mcdeint.c \
                                        libmpcodecs/vf_qp.c \
                                        libmpcodecs/vf_spp.c \
                                        libmpcodecs/vf_uspp.c \

SRCS_COMMON-$(LIBAVFORMAT)           += libmpdemux/demux_lavf.c \
                                        stream/stream_ffmpeg.c \

SRCS_COMMON-$(LIBBLURAY)             += stream/stream_bluray.c
SRCS_COMMON-$(LIBBS2B)               += libaf/af_bs2b.c
SRCS_COMMON-$(LIBDCA)                += libmpcodecs/ad_libdca.c
SRCS_COMMON-$(LIBDV)                 += libmpcodecs/ad_libdv.c \
                                        libmpcodecs/vd_libdv.c \
                                        libmpdemux/demux_rawdv.c
SRCS_COMMON-$(LIBDVDCSS_INTERNAL)    += libdvdcss/css.c \
                                        libdvdcss/device.c \
                                        libdvdcss/error.c \
                                        libdvdcss/ioctl.c \
                                        libdvdcss/libdvdcss.c \

SRCS_COMMON-$(LIBMAD)                += libmpcodecs/ad_libmad.c

SRCS_COMMON-$(LIBMPEG2)              += libmpcodecs/vd_libmpeg2.c
SRCS_LIBMPEG2-INTERNAL-$(ARCH_ALPHA) += libmpeg2/idct_alpha.c \
                                        libmpeg2/motion_comp_alpha.c
SRCS_LIBMPEG2-INTERNAL-$(ARCH_ARM)   += libmpeg2/motion_comp_arm.c \
                                        libmpeg2/motion_comp_arm_s.S
SRCS_LIBMPEG2-INTERNAL-$(HAVE_ALTIVEC) += libmpeg2/idct_altivec.c \
                                        libmpeg2/motion_comp_altivec.c
SRCS_LIBMPEG2-INTERNAL-$(HAVE_MMX)   += libmpeg2/idct_mmx.c \
                                        libmpeg2/motion_comp_mmx.c
SRCS_LIBMPEG2-INTERNAL-$(HAVE_VIS)   += libmpeg2/motion_comp_vis.c
SRCS_COMMON-$(LIBMPEG2_INTERNAL)     += libmpeg2/alloc.c \
                                        libmpeg2/cpu_accel.c\
                                        libmpeg2/cpu_state.c \
                                        libmpeg2/decode.c \
                                        libmpeg2/header.c \
                                        libmpeg2/idct.c \
                                        libmpeg2/motion_comp.c \
                                        libmpeg2/slice.c \
                                        $(SRCS_LIBMPEG2-INTERNAL-yes)

SRCS_COMMON-$(LIBNEMESI)             += libmpdemux/demux_nemesi.c \
                                        stream/stream_nemesi.c
SRCS_COMMON-$(LIBNUT)                += libmpdemux/demux_nut.c
SRCS_COMMON-$(LIBPOSTPROC)           += libmpcodecs/vf_pp.c
SRCS_COMMON-$(LIBSMBCLIENT)          += stream/stream_smb.c
SRCS_COMMON-$(LIBTHEORA)             += libmpcodecs/vd_theora.c
SRCS_COMMON-$(LIVE555)               += libmpdemux/demux_rtp.cpp \
                                        libmpdemux/demux_rtp_codec.cpp \
                                        stream/stream_live555.c
SRCS_COMMON-$(MACOSX_FINDER)         += osdep/macosx_finder_args.c
SRCS_COMMON-$(MNG)                   += libmpdemux/demux_mng.c
SRCS_COMMON-$(MPG123)                += libmpcodecs/ad_mpg123.c

SRCS_MP3LIB-X86-$(HAVE_AMD3DNOW)     += mp3lib/dct36_3dnow.c \
                                        mp3lib/dct64_3dnow.c
SRCS_MP3LIB-X86-$(HAVE_AMD3DNOWEXT)  += mp3lib/dct36_k7.c \
                                        mp3lib/dct64_k7.c
SRCS_MP3LIB-X86-$(HAVE_MMX)          += mp3lib/dct64_mmx.c
SRCS_MP3LIB-$(ARCH_X86_32)           += mp3lib/decode_i586.c \
                                        $(SRCS_MP3LIB-X86-yes)
SRCS_MP3LIB-$(HAVE_ALTIVEC)          += mp3lib/dct64_altivec.c
SRCS_MP3LIB-$(HAVE_MMX)              += mp3lib/decode_mmx.c
SRCS_MP3LIB-$(HAVE_SSE)              += mp3lib/dct64_sse.c
SRCS_MP3LIB                          += mp3lib/sr1.c \
                                        $(SRCS_MP3LIB-yes)
SRCS_COMMON-$(MP3LIB)                += libmpcodecs/ad_mp3lib.c \
                                        $(SRCS_MP3LIB)

SRCS_COMMON-$(MUSEPACK)              += libmpcodecs/ad_mpc.c \
                                        libmpdemux/demux_mpc.c
SRCS_COMMON-$(NATIVE_RTSP)           += stream/stream_rtsp.c \
                                        stream/freesdp/common.c \
                                        stream/freesdp/errorlist.c \
                                        stream/freesdp/parser.c \
                                        stream/librtsp/rtsp.c \
                                        stream/librtsp/rtsp_rtp.c \
                                        stream/librtsp/rtsp_session.c \

SRCS_COMMON-$(NEED_GETTIMEOFDAY)     += osdep/gettimeofday.c
SRCS_COMMON-$(NEED_GLOB)             += osdep/glob-win.c
SRCS_COMMON-$(NEED_MMAP)             += osdep/mmap-os2.c
SRCS_COMMON-$(NEED_SETENV)           += osdep/setenv.c
SRCS_COMMON-$(NEED_SHMEM)            += osdep/shmem.c
SRCS_COMMON-$(NEED_STRSEP)           += osdep/strsep.c
SRCS_COMMON-$(NEED_SWAB)             += osdep/swab.c
SRCS_COMMON-$(NEED_VSSCANF)          += osdep/vsscanf.c
SRCS_COMMON-$(NETWORKING)            += stream/stream_netstream.c \
                                        stream/asf_mmst_streaming.c \
                                        stream/asf_streaming.c \
                                        stream/cookies.c \
                                        stream/http.c \
                                        stream/network.c \
                                        stream/pnm.c \
                                        stream/rtp.c \
                                        stream/udp.c \
                                        stream/tcp.c \
                                        stream/stream_rtp.c \
                                        stream/stream_udp.c \
                                        stream/librtsp/rtsp.c \
                                        stream/realrtsp/asmrp.c \
                                        stream/realrtsp/real.c \
                                        stream/realrtsp/rmff.c \
                                        stream/realrtsp/sdpplin.c \
                                        stream/realrtsp/xbuffer.c \

SRCS_COMMON-$(PNG)                   += libmpcodecs/vd_mpng.c
SRCS_COMMON-$(PRIORITY)              += osdep/priority.c
SRCS_COMMON-$(PVR)                   += stream/stream_pvr.c
SRCS_COMMON-$(QTX_CODECS)            += libmpcodecs/ad_qtaudio.c \
                                        libmpcodecs/vd_qtvideo.c
SRCS_COMMON-$(RADIO)                 += stream/stream_radio.c
SRCS_COMMON-$(RADIO_CAPTURE)         += stream/audio_in.c
SRCS_COMMON-$(REAL_CODECS)           += libmpcodecs/ad_realaud.c \
                                        libmpcodecs/vd_realvid.c
SRCS_COMMON-$(SPEEX)                 += libmpcodecs/ad_speex.c
SRCS_COMMON-$(STREAM_CACHE)          += stream/cache2.c

SRCS_COMMON-$(TREMOR_INTERNAL)       += tremor/bitwise.c \
                                        tremor/block.c \
                                        tremor/codebook.c \
                                        tremor/floor0.c \
                                        tremor/floor1.c \
                                        tremor/framing.c \
                                        tremor/info.c \
                                        tremor/mapping0.c \
                                        tremor/mdct.c \
                                        tremor/registry.c \
                                        tremor/res012.c \
                                        tremor/sharedbook.c \
                                        tremor/synthesis.c \
                                        tremor/window.c \

SRCS_COMMON-$(TV)                    += stream/stream_tv.c stream/tv.c \
                                        stream/frequencies.c stream/tvi_dummy.c
SRCS_COMMON-$(TV_BSDBT848)           += stream/tvi_bsdbt848.c
SRCS_COMMON-$(TV_DSHOW)              += stream/tvi_dshow.c \
                                        loader/dshow/guids.c \
                                        loader/dshow/mediatype.c \

SRCS_COMMON-$(TV_V4L1)               += stream/tvi_v4l.c  stream/audio_in.c
SRCS_COMMON-$(TV_V4L2)               += stream/tvi_v4l2.c stream/audio_in.c
SRCS_COMMON-$(UNRAR_EXEC)            += unrar_exec.c
SRCS_COMMON-$(VCD)                   += stream/stream_vcd.c
SRCS_COMMON-$(VORBIS)                += libmpcodecs/ad_libvorbis.c \
                                        libmpdemux/demux_ogg.c
SRCS_COMMON-$(VSTREAM)               += stream/stream_vstream.c
SRCS_QTX_EMULATION                   += loader/wrapper.S
SRCS_COMMON-$(QTX_EMULATION)         += $(SRCS_QTX_EMULATION)
SRCS_WIN32_EMULATION                 += loader/elfdll.c \
                                        loader/ext.c \
                                        loader/ldt_keeper.c \
                                        loader/module.c \
                                        loader/pe_image.c \
                                        loader/pe_resource.c \
                                        loader/registry.c \
                                        loader/resource.c \
                                        loader/win32.c \

SRCS_COMMON-$(WIN32_EMULATION)       += $(SRCS_WIN32_EMULATION)

SRCS_COMMON-$(WIN32DLL)              += libmpcodecs/ad_acm.c \
                                        libmpcodecs/ad_dmo.c \
                                        libmpcodecs/ad_dshow.c \
                                        libmpcodecs/ad_twin.c \
                                        libmpcodecs/vd_dmo.c \
                                        libmpcodecs/vd_dshow.c \
                                        libmpcodecs/vd_vfw.c \
                                        libmpcodecs/vd_vfwex.c \
                                        libmpdemux/demux_avs.c \
                                        loader/afl.c \
                                        loader/drv.c \
                                        loader/vfl.c \
                                        loader/dshow/DS_AudioDecoder.c \
                                        loader/dshow/DS_Filter.c \
                                        loader/dshow/DS_VideoDecoder.c \
                                        loader/dshow/allocator.c \
                                        loader/dshow/cmediasample.c \
                                        loader/dshow/graph.c \
                                        loader/dshow/guids.c \
                                        loader/dshow/inputpin.c \
                                        loader/dshow/mediatype.c \
                                        loader/dshow/outputpin.c \
                                        loader/dmo/DMO_AudioDecoder.c \
                                        loader/dmo/DMO_VideoDecoder.c   \
                                        loader/dmo/buffer.c   \
                                        loader/dmo/dmo.c  \
                                        loader/dmo/dmo_guids.c \

SRCS_COMMON-$(XANIM_CODECS)          += libmpcodecs/vd_xanim.c
SRCS_COMMON-$(XMMS_PLUGINS)          += libmpdemux/demux_xmms.c
SRCS_COMMON-$(XVID4)                 += libmpcodecs/vd_xvid4.c
SRCS_COMMON-$(ZR)                    += libmpcodecs/vd_zrmjpeg.c \
                                        libmpcodecs/vf_zrmjpeg.c
SRCS_COMMON = asxparser.c \
              bstr.c \
              codec-cfg.c \
              cpudetect.c \
              edl.c \
              eosd.c \
              find_sub.c \
              fmt-conversion.c \
              m_config.c \
              m_option.c \
              m_struct.c \
              mp_msg.c \
              mpcommon.c \
              parser-cfg.c \
              path.c \
              playtree.c \
              playtreeparser.c \
              spudec.c \
              sub_cc.c \
              subopt-helper.c \
              subreader.c \
              vobsub.c \
              libaf/af.c \
              libaf/af_center.c \
              libaf/af_channels.c \
              libaf/af_comp.c \
              libaf/af_delay.c \
              libaf/af_dummy.c \
              libaf/af_equalizer.c \
              libaf/af_extrastereo.c \
              libaf/af_format.c \
              libaf/af_gate.c \
              libaf/af_hrtf.c \
              libaf/af_karaoke.c \
              libaf/af_pan.c \
              libaf/af_resample.c \
              libaf/af_scaletempo.c \
              libaf/af_sinesuppress.c \
              libaf/af_stats.c \
              libaf/af_sub.c \
              libaf/af_surround.c \
              libaf/af_sweep.c \
              libaf/af_tools.c \
              libaf/af_volnorm.c \
              libaf/af_volume.c \
              libaf/filter.c \
              libaf/format.c \
              libaf/reorder_ch.c \
              libaf/window.c \
              libmpcodecs/ad.c \
              libmpcodecs/ad_alaw.c \
              libmpcodecs/ad_dk3adpcm.c \
              libmpcodecs/ad_dvdpcm.c \
              libmpcodecs/ad_hwac3.c \
              libmpcodecs/ad_hwmpa.c \
              libmpcodecs/ad_imaadpcm.c \
              libmpcodecs/ad_msadpcm.c \
              libmpcodecs/ad_pcm.c \
              libmpcodecs/dec_audio.c \
              libmpcodecs/dec_teletext.c \
              libmpcodecs/dec_video.c \
              libmpcodecs/img_format.c \
              libmpcodecs/mp_image.c \
              libmpcodecs/pullup.c \
              libmpcodecs/vd.c \
              libmpcodecs/vd_hmblck.c \
              libmpcodecs/vd_lzo.c \
              libmpcodecs/vd_mpegpes.c \
              libmpcodecs/vd_mtga.c \
              libmpcodecs/vd_null.c \
              libmpcodecs/vd_raw.c \
              libmpcodecs/vd_sgi.c \
              libmpcodecs/vf.c \
              libmpcodecs/vf_1bpp.c \
              libmpcodecs/vf_2xsai.c \
              libmpcodecs/vf_blackframe.c \
              libmpcodecs/vf_boxblur.c \
              libmpcodecs/vf_crop.c \
              libmpcodecs/vf_cropdetect.c \
              libmpcodecs/vf_decimate.c \
              libmpcodecs/vf_delogo.c \
              libmpcodecs/vf_denoise3d.c \
              libmpcodecs/vf_detc.c \
              libmpcodecs/vf_dint.c \
              libmpcodecs/vf_divtc.c \
              libmpcodecs/vf_down3dright.c \
              libmpcodecs/vf_dsize.c \
              libmpcodecs/vf_dvbscale.c \
              libmpcodecs/vf_eq.c \
              libmpcodecs/vf_eq2.c \
              libmpcodecs/vf_expand.c \
              libmpcodecs/vf_field.c \
              libmpcodecs/vf_fil.c \
              libmpcodecs/vf_filmdint.c \
              libmpcodecs/vf_fixpts.c \
              libmpcodecs/vf_flip.c \
              libmpcodecs/vf_format.c \
              libmpcodecs/vf_framestep.c \
              libmpcodecs/vf_gradfun.c \
              libmpcodecs/vf_halfpack.c \
              libmpcodecs/vf_harddup.c \
              libmpcodecs/vf_hqdn3d.c \
              libmpcodecs/vf_hue.c \
              libmpcodecs/vf_il.c \
              libmpcodecs/vf_ilpack.c \
              libmpcodecs/vf_ivtc.c \
              libmpcodecs/vf_kerndeint.c \
              libmpcodecs/vf_mirror.c \
              libmpcodecs/vf_noformat.c \
              libmpcodecs/vf_noise.c \
              libmpcodecs/vf_ow.c \
              libmpcodecs/vf_palette.c \
              libmpcodecs/vf_perspective.c \
              libmpcodecs/vf_phase.c \
              libmpcodecs/vf_pp7.c \
              libmpcodecs/vf_pullup.c \
              libmpcodecs/vf_rectangle.c \
              libmpcodecs/vf_remove_logo.c \
              libmpcodecs/vf_rgbtest.c \
              libmpcodecs/vf_rotate.c \
              libmpcodecs/vf_sab.c \
              libmpcodecs/vf_scale.c \
              libmpcodecs/vf_smartblur.c \
              libmpcodecs/vf_softpulldown.c \
              libmpcodecs/vf_softskip.c \
              libmpcodecs/vf_swapuv.c \
              libmpcodecs/vf_telecine.c \
              libmpcodecs/vf_test.c \
              libmpcodecs/vf_tfields.c \
              libmpcodecs/vf_tile.c \
              libmpcodecs/vf_tinterlace.c \
              libmpcodecs/vf_unsharp.c \
              libmpcodecs/vf_vo.c \
              libmpcodecs/vf_yadif.c \
              libmpcodecs/vf_yuvcsp.c \
              libmpcodecs/vf_yvu9.c \
              libmpdemux/aac_hdr.c \
              libmpdemux/asfheader.c \
              libmpdemux/aviheader.c \
              libmpdemux/aviprint.c \
              libmpdemux/demuxer.c \
              libmpdemux/demux_aac.c \
              libmpdemux/demux_asf.c \
              libmpdemux/demux_audio.c \
              libmpdemux/demux_avi.c \
              libmpdemux/demux_demuxers.c \
              libmpdemux/demux_film.c \
              libmpdemux/demux_fli.c \
              libmpdemux/demux_lmlm4.c \
              libmpdemux/demux_mf.c \
              libmpdemux/demux_mkv.c \
              libmpdemux/demux_mov.c \
              libmpdemux/demux_mpg.c \
              libmpdemux/demux_nsv.c \
              libmpdemux/demux_pva.c \
              libmpdemux/demux_rawaudio.c \
              libmpdemux/demux_rawvideo.c \
              libmpdemux/demux_realaud.c \
              libmpdemux/demux_real.c \
              libmpdemux/demux_roq.c \
              libmpdemux/demux_smjpeg.c \
              libmpdemux/demux_ts.c \
              libmpdemux/demux_ty.c \
              libmpdemux/demux_ty_osd.c \
              libmpdemux/demux_viv.c \
              libmpdemux/demux_vqf.c \
              libmpdemux/demux_y4m.c \
              libmpdemux/ebml.c \
              libmpdemux/extension.c \
              libmpdemux/mf.c \
              libmpdemux/mp3_hdr.c \
              libmpdemux/mp_taglists.c \
              libmpdemux/mpeg_hdr.c \
              libmpdemux/mpeg_packetizer.c \
              libmpdemux/parse_es.c \
              libmpdemux/parse_mp4.c \
              libmpdemux/video.c \
              libmpdemux/yuv4mpeg.c \
              libmpdemux/yuv4mpeg_ratio.c \
              libvo/osd.c \
              libvo/sub.c \
              osdep/$(GETCH) \
              osdep/$(TIMER) \
              stream/open.c \
              stream/stream.c \
              stream/stream_bd.c \
              stream/stream_cue.c \
              stream/stream_file.c \
              stream/stream_mf.c \
              stream/stream_null.c \
              stream/url.c \
              $(SRCS_COMMON-yes)


SRCS_MPLAYER-$(3DFX)         += libvo/vo_3dfx.c
SRCS_MPLAYER-$(AA)           += libvo/vo_aa.c
SRCS_MPLAYER-$(ALSA1X)       += libao2/ao_alsa.c
SRCS_MPLAYER-$(ALSA5)        += libao2/ao_alsa5.c
SRCS_MPLAYER-$(ALSA9)        += libao2/ao_alsa.c
SRCS_MPLAYER-$(APPLE_IR)     += input/appleir.c
SRCS_MPLAYER-$(APPLE_REMOTE) += input/ar.c
SRCS_MPLAYER-$(ARTS)         += libao2/ao_arts.c
SRCS_MPLAYER-$(BL)           += libvo/vo_bl.c
SRCS_MPLAYER-$(CACA)         += libvo/vo_caca.c
SRCS_MPLAYER-$(COREAUDIO)    += libao2/ao_coreaudio.c
SRCS_MPLAYER-$(COREVIDEO)    += libvo/vo_corevideo.m libvo/osx_common.c
SRCS_MPLAYER-$(DART)         += libao2/ao_dart.c
SRCS_MPLAYER-$(DFBMGA)       += libvo/vo_dfbmga.c
SRCS_MPLAYER-$(DGA)          += libvo/vo_dga.c
SRCS_MPLAYER-$(DIRECT3D)     += libvo/vo_direct3d.c libvo/w32_common.c
SRCS_MPLAYER-$(DIRECTFB)     += libvo/vo_directfb2.c
SRCS_MPLAYER-$(DIRECTX)      += libao2/ao_dsound.c libvo/vo_directx.c
SRCS_MPLAYER-$(DXR2)         += libao2/ao_dxr2.c libvo/vo_dxr2.c
SRCS_MPLAYER-$(DXR3)         += libvo/vo_dxr3.c
SRCS_MPLAYER-$(ESD)          += libao2/ao_esd.c
SRCS_MPLAYER-$(FBDEV)        += libvo/vo_fbdev.c libvo/vo_fbdev2.c
SRCS_MPLAYER-$(GGI)          += libvo/vo_ggi.c
SRCS_MPLAYER-$(GIF)          += libvo/vo_gif89a.c
SRCS_MPLAYER-$(GL)           += libvo/gl_common.c libvo/vo_gl.c \
                                libvo/vo_gl2.c libvo/csputils.c
SRCS_MPLAYER-$(GL_SDL)       += libvo/sdl_common.c
SRCS_MPLAYER-$(GL_WIN32)     += libvo/w32_common.c
SRCS_MPLAYER-$(GL_X11)       += libvo/x11_common.c
SRCS_MPLAYER-$(MATRIXVIEW)   += libvo/vo_matrixview.c libvo/matrixview.c
SRCS_MPLAYER-$(GUI)          += gui/bitmap.c
SRCS_MPLAYER-$(GUI_GTK)      += gui/app.c \
                                gui/cfg.c \
                                gui/interface.c \
                                gui/mplayer/gui_common.c \
                                gui/mplayer/menu.c \
                                gui/mplayer/mw.c \
                                gui/mplayer/pb.c \
                                gui/mplayer/play.c \
                                gui/mplayer/sw.c \
                                gui/mplayer/widgets.c \
                                gui/mplayer/gtk/about.c \
                                gui/mplayer/gtk/eq.c \
                                gui/mplayer/gtk/fs.c \
                                gui/mplayer/gtk/gtk_common.c \
                                gui/mplayer/gtk/gtk_menu.c \
                                gui/mplayer/gtk/gtk_url.c \
                                gui/mplayer/gtk/mb.c \
                                gui/mplayer/gtk/opts.c \
                                gui/mplayer/gtk/pl.c \
                                gui/mplayer/gtk/sb.c \
                                gui/skin/cut.c \
                                gui/skin/font.c \
                                gui/skin/skin.c \
                                gui/wm/ws.c \
                                gui/wm/wsxdnd.c \

SRCS_MPLAYER-$(GUI_WIN32)    += gui/win32/dialogs.c \
                                gui/win32/gui.c \
                                gui/win32/interface.c \
                                gui/win32/playlist.c \
                                gui/win32/preferences.c \
                                gui/win32/skinload.c \
                                gui/win32/widgetrender.c \
                                gui/win32/wincfg.c \

SRCS_MPLAYER-$(IVTV)         += libao2/ao_ivtv.c libvo/vo_ivtv.c
SRCS_MPLAYER-$(JACK)         += libao2/ao_jack.c
SRCS_MPLAYER-$(JOYSTICK)     += input/joystick.c
SRCS_MPLAYER-$(JPEG)         += libvo/vo_jpeg.c
SRCS_MPLAYER-$(KAI)          += libao2/ao_kai.c
SRCS_MPLAYER-$(KVA)          += libvo/vo_kva.c
SRCS_MPLAYER-$(LIBAVCODEC)   += libvo/vo_png.c
SRCS_MPLAYER-$(LIBMENU)      += libmenu/menu.c \
                                libmenu/menu_chapsel.c \
                                libmenu/menu_cmdlist.c  \
                                libmenu/menu_console.c \
                                libmenu/menu_filesel.c \
                                libmenu/menu_list.c  \
                                libmenu/menu_param.c \
                                libmenu/menu_pt.c \
                                libmenu/menu_txt.c \
                                libmenu/vf_menu.c \

SRCS_MPLAYER-$(LIBMENU_DVBIN) += libmenu/menu_dvbin.c
SRCS_MPLAYER-$(LIRC)          += input/lirc.c
SRCS_MPLAYER-$(MD5SUM)        += libvo/vo_md5sum.c
SRCS_MPLAYER-$(MGA)           += libvo/vo_mga.c
SRCS_MPLAYER-$(NAS)           += libao2/ao_nas.c
SRCS_MPLAYER-$(NETWORKING)    += udp_sync.c
SRCS_MPLAYER-$(OPENAL)        += libao2/ao_openal.c
SRCS_MPLAYER-$(OSS)           += libao2/ao_oss.c
SRCS_MPLAYER-$(PNM)           += libvo/vo_pnm.c
SRCS_MPLAYER-$(PULSE)         += libao2/ao_pulse.c
SRCS_MPLAYER-$(QUARTZ)        += libvo/vo_quartz.c libvo/osx_common.c
SRCS_MPLAYER-$(S3FB)          += libvo/vo_s3fb.c
SRCS_MPLAYER-$(SDL)           += libao2/ao_sdl.c libvo/vo_sdl.c libvo/sdl_common.c
SRCS_MPLAYER-$(SGIAUDIO)      += libao2/ao_sgi.c
SRCS_MPLAYER-$(SUNAUDIO)      += libao2/ao_sun.c
SRCS_MPLAYER-$(SVGA)          += libvo/vo_svga.c
SRCS_MPLAYER-$(TDFXFB)        += libvo/vo_tdfxfb.c
SRCS_MPLAYER-$(TDFXVID)       += libvo/vo_tdfx_vid.c
SRCS_MPLAYER-$(TGA)           += libvo/vo_tga.c
SRCS_MPLAYER-$(V4L2)          += libvo/vo_v4l2.c
SRCS_MPLAYER-$(V4L2)          += libao2/ao_v4l2.c
SRCS_MPLAYER-$(VDPAU)         += libvo/vo_vdpau.c
SRCS_MPLAYER-$(VESA)          += libvo/gtf.c libvo/vo_vesa.c libvo/vesa_lvo.c
SRCS_MPLAYER-$(VIDIX)         += libvo/vo_cvidix.c \
                                 libvo/vosub_vidix.c \
                                 vidix/vidix.c \
                                 vidix/drivers.c \
                                 vidix/dha.c \
                                 vidix/mtrr.c \
                                 vidix/pci.c \
                                 vidix/pci_names.c \
                                 vidix/pci_dev_ids.c\

SRCS_MPLAYER-$(VIDIX_CYBERBLADE)    += vidix/cyberblade_vid.c
SRCS_MPLAYER-$(VIDIX_IVTV)          += vidix/ivtv_vid.c
SRCS_MPLAYER-$(VIDIX_MACH64)        += vidix/mach64_vid.c
SRCS_MPLAYER-$(VIDIX_MGA)           += vidix/mga_vid.c
SRCS_MPLAYER-$(VIDIX_MGA_CRTC2)     += vidix/mga_crtc2_vid.c
SRCS_MPLAYER-$(VIDIX_NVIDIA)        += vidix/nvidia_vid.c
SRCS_MPLAYER-$(VIDIX_PM2)           += vidix/pm2_vid.c
SRCS_MPLAYER-$(VIDIX_PM3)           += vidix/pm3_vid.c
SRCS_MPLAYER-$(VIDIX_RADEON)        += vidix/radeon_vid.c
SRCS_MPLAYER-$(VIDIX_RAGE128)       += vidix/rage128_vid.c
SRCS_MPLAYER-$(VIDIX_S3)            += vidix/s3_vid.c
SRCS_MPLAYER-$(VIDIX_SH_VEU)        += vidix/sh_veu_vid.c
SRCS_MPLAYER-$(VIDIX_SIS)           += vidix/sis_vid.c vidix/sis_bridge.c
SRCS_MPLAYER-$(VIDIX_UNICHROME)     += vidix/unichrome_vid.c
SRCS_MPLAYER-$(WII)           += libvo/vo_wii.c
SRCS_MPLAYER-$(WIN32WAVEOUT)  += libao2/ao_win32.c
SRCS_MPLAYER-$(WINVIDIX)      += libvo/vo_winvidix.c
SRCS_MPLAYER-$(X11)           += libvo/vo_x11.c libvo/vo_xover.c \
                                 libvo/x11_common.c
SRCS_MPLAYER-$(XMGA)          += libvo/vo_xmga.c
SRCS_MPLAYER-$(XV)            += libvo/vo_xv.c
SRCS_MPLAYER-$(XVIDIX)        += libvo/vo_xvidix.c
SRCS_MPLAYER-$(XVMC)          += libvo/vo_xvmc.c
SRCS_MPLAYER-$(XVR100)        += libvo/vo_xvr100.c
SRCS_MPLAYER-$(YUV4MPEG)      += libvo/vo_yuv4mpeg.c
SRCS_MPLAYER-$(ZR)            += libvo/jpeg_enc.c libvo/vo_zr.c libvo/vo_zr2.c

SRCS_MPLAYER = command.c \
               m_property.c \
               mixer.c \
               mp_fifo.c \
               mplayer.c \
               parser-mpcmd.c \
               input/input.c \
               libao2/ao_mpegpes.c \
               libao2/ao_null.c \
               libao2/ao_pcm.c \
               libao2/ao_android.c \
               libao2/audio_out.c \
               libvo/aspect.c \
               libvo/geometry.c \
               libvo/spuenc.c \
               libvo/video_out.c \
               libvo/vo_mpegpes.c \
               libvo/vo_null.c \
               $(SRCS_MPLAYER-yes)


SRCS_MENCODER-$(FAAC)             += libmpcodecs/ae_faac.c
SRCS_MENCODER-$(LIBAVCODEC)       += libmpcodecs/ae_lavc.c libmpcodecs/ve_lavc.c
SRCS_MENCODER-$(LIBAVFORMAT)      += libmpdemux/muxer_lavf.c
SRCS_MENCODER-$(LIBDV)            += libmpcodecs/ve_libdv.c
SRCS_MENCODER-$(LIBLZO)           += libmpcodecs/ve_nuv.c \
                                     libmpcodecs/native/rtjpegn.c
SRCS_MENCODER-$(MP3LAME)          += libmpcodecs/ae_lame.c
SRCS_MENCODER-$(QTX_CODECS_WIN32) += libmpcodecs/ve_qtvideo.c
SRCS_MENCODER-$(TOOLAME)          += libmpcodecs/ae_toolame.c
SRCS_MENCODER-$(TWOLAME)          += libmpcodecs/ae_twolame.c
SRCS_MENCODER-$(WIN32DLL)         += libmpcodecs/ve_vfw.c
SRCS_MENCODER-$(X264)             += libmpcodecs/ve_x264.c
SRCS_MENCODER-$(XVID4)            += libmpcodecs/ve_xvid4.c

SRCS_MENCODER = mencoder.c \
                parser-mecmd.c \
                xvid_vbr.c \
                libmpcodecs/ae.c \
                libmpcodecs/ae_pcm.c \
                libmpcodecs/ve.c \
                libmpcodecs/ve_raw.c \
                libmpdemux/muxer.c \
                libmpdemux/muxer_avi.c \
                libmpdemux/muxer_mpeg.c \
                libmpdemux/muxer_rawaudio.c \
                libmpdemux/muxer_rawvideo.c \
                $(SRCS_MENCODER-yes)


COMMON_LIBS-$(LIBAVFORMAT_A)      += libavformat/libavformat.a
COMMON_LIBS-$(LIBAVCODEC_A)       += libavcodec/libavcodec.a
COMMON_LIBS-$(LIBAVCORE_A)        += libavcore/libavcore.a
COMMON_LIBS-$(LIBAVUTIL_A)        += libavutil/libavutil.a
COMMON_LIBS-$(LIBPOSTPROC_A)      += libpostproc/libpostproc.a
COMMON_LIBS-$(LIBSWSCALE_A)       += libswscale/libswscale.a
COMMON_LIBS += $(COMMON_LIBS-yes)

OBJS_COMMON    += $(addsuffix .o, $(basename $(SRCS_COMMON)))
OBJS_MENCODER  += $(addsuffix .o, $(basename $(SRCS_MENCODER)))
OBJS_MPLAYER   += $(addsuffix .o, $(basename $(SRCS_MPLAYER)))
OBJS_MPLAYER-$(PE_EXECUTABLE) += osdep/mplayer-rc.o
OBJS_MPLAYER   += $(OBJS_MPLAYER-yes)

MENCODER_DEPS = $(OBJS_MENCODER) $(OBJS_COMMON) $(COMMON_LIBS)
MPLAYER_DEPS  = $(OBJS_MPLAYER)  $(OBJS_COMMON) $(COMMON_LIBS)
DEPS = $(filter-out %.S,$(patsubst %.cpp,%.d,$(patsubst %.c,%.d,$(SRCS_COMMON) $(SRCS_MPLAYER:.m=.d) $(SRCS_MENCODER))))

ALL_PRG-$(MPLAYER)  += mplayer$(EXESUF)
ALL_PRG-$(MENCODER) += mencoder$(EXESUF)

INSTALL_TARGETS-$(GUI)      += install-gui
INSTALL_TARGETS-$(MENCODER) += install-mencoder install-mencoder-man
INSTALL_TARGETS-$(MPLAYER)  += install-mplayer  install-mplayer-man

DIRS =  . \
        gui \
        gui/mplayer \
        gui/mplayer/gtk \
        gui/skin \
        gui/wm \
        gui/win32 \
        input \
        libaf \
        libao2 \
        libass \
        libavcodec \
        libavcodec/alpha \
        libavcodec/arm \
        libavcodec/bfin \
        libavcodec/mlib \
        libavcodec/ppc \
        libavcodec/sh4 \
        libavcodec/sparc \
        libavcodec/x86 \
        libavcore \
        libavformat \
        libavutil \
        libavutil/arm \
        libavutil/bfin \
        libavutil/ppc \
        libavutil/sh4 \
        libavutil/tomi \
        libavutil/x86 \
        libdvdcss \
        libdvdnav \
        libdvdnav/vm \
        libdvdread4 \
        libfaad2 \
        libmenu \
        libmpcodecs \
        libmpcodecs/native \
        libmpdemux \
        libmpeg2 \
        libpostproc \
        libswscale \
        libswscale/bfin \
        libswscale/mlib \
        libswscale/ppc \
        libswscale/sparc \
        libswscale/x86 \
        libvo \
        loader \
        loader/dshow \
        loader/dmo \
        loader/wine \
        mp3lib \
        osdep \
        stream \
        stream/freesdp \
        stream/librtsp \
        stream/realrtsp \
        tremor \
        TOOLS \
        vidix \

ALLHEADERS = $(foreach dir,$(DIRS),$(wildcard $(dir)/*.h))

ADDSUFFIXES     = $(foreach suf,$(1),$(addsuffix $(suf),$(2)))
ADD_ALL_DIRS    = $(call ADDSUFFIXES,$(1),$(DIRS))
ADD_ALL_EXESUFS = $(1) $(call ADDSUFFIXES,$(EXESUFS_ALL),$(1))

FFMPEGPARTS = libavcodec \
              libavcore \
              libavformat \
              libavutil \
              libpostproc \
              libswscale \

FFMPEGLIBS  = $(foreach part, $(FFMPEGPARTS), $(part)/$(part).a)
FFMPEGFILES = $(foreach part, $(FFMPEGPARTS), $(wildcard $(part)/*.[chS] $(part)/*/*.[chS]))



LOCAL_SRC_FILES := $(SRCS_MPLAYER)
LOCAL_LDLIBS := -Wl,-z,noexecstack  -ffast-math -ldl -rdynamic -lm 
LOCAL_CFLAGS += -D_ISOC99_SOURCE -D_POSIX_C_SOURCE=200112 -O3 -std=c99 -D__linux__ -DCONFIG_ANDROID
include $(BUILD_EXECUTABLE)
