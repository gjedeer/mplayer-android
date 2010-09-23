/*
 * PCM audio output driver (mplayer's ao_pcm.c modified by overdose for android support)
 *
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

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libavutil/common.h"
#include "mpbswap.h"
#include "subopt-helper.h"
#include "libaf/af_format.h"
#include "libaf/reorder_ch.h"
#include "audio_out.h"
#include "audio_out_internal.h"
#include "mp_msg.h"
#include "help_mp.h"

#include <sys/ioctl.h>
#include <linux/soundcard.h>
#include <linux/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#ifdef __MINGW32__
// for GetFileType to detect pipes
#include <windows.h>
#endif

#if 0
#include <linux/msm_audio.h>
#else
/* ---------- linux/msm_audio.h -------- */

#define AUDIO_IOCTL_MAGIC 'a'

#define AUDIO_START        _IOW(AUDIO_IOCTL_MAGIC, 0, unsigned)
#define AUDIO_STOP         _IOW(AUDIO_IOCTL_MAGIC, 1, unsigned)
#define AUDIO_FLUSH        _IOW(AUDIO_IOCTL_MAGIC, 2, unsigned)
#define AUDIO_GET_CONFIG   _IOR(AUDIO_IOCTL_MAGIC, 3, unsigned)
#define AUDIO_SET_CONFIG   _IOW(AUDIO_IOCTL_MAGIC, 4, unsigned)
#define AUDIO_GET_STATS    _IOR(AUDIO_IOCTL_MAGIC, 5, unsigned)
#define AUDIO_ENABLE_AUDPP _IOW(AUDIO_IOCTL_MAGIC, 6, unsigned)
#define AUDIO_SET_ADRC     _IOW(AUDIO_IOCTL_MAGIC, 7, unsigned)
#define AUDIO_SET_EQ       _IOW(AUDIO_IOCTL_MAGIC, 8, unsigned)
#define AUDIO_SET_RX_IIR   _IOW(AUDIO_IOCTL_MAGIC, 9, unsigned)

#define EQ_MAX_BAND_NUM	12

#define ADRC_ENABLE  0x0001
#define ADRC_DISABLE 0x0000
#define EQ_ENABLE    0x0002
#define EQ_DISABLE   0x0000
#define IIR_ENABLE   0x0004
#define IIR_DISABLE  0x0000

struct eq_filter_type
{
  int16_t gain;
  uint16_t freq;
  uint16_t type;
  uint16_t qf;
};

struct eqalizer
{
  uint16_t bands;
  uint16_t params[132];
};

struct rx_iir_filter
{
  uint16_t num_bands;
  uint16_t iir_params[48];
};


struct msm_audio_config
{
  uint32_t buffer_size;
  uint32_t buffer_count;
  uint32_t channel_count;
  uint32_t sample_rate;
  uint32_t codec_type;
  uint32_t unused[3];
};

struct msm_audio_stats
{
	 uint32_t byte_count;
	 uint32_t sample_count;
	 uint32_t unused[2];
};

/* Audio routing */

#define SND_IOCTL_MAGIC 's'

#define SND_MUTE_UNMUTED 0
#define SND_MUTE_MUTED   1

struct msm_snd_device_config
{
  uint32_t device;
  uint32_t ear_mute;
  uint32_t mic_mute;
};

#define SND_SET_DEVICE _IOW(SND_IOCTL_MAGIC, 2, struct msm_device_config *)

#define SND_METHOD_VOICE 0

#define SND_METHOD_VOICE_1 1

struct msm_snd_volume_config
{
  uint32_t device;
  uint32_t method;
  uint32_t volume;
};

#define SND_SET_VOLUME _IOW(SND_IOCTL_MAGIC, 3, struct msm_snd_volume_config *)

/* Returns the number of SND endpoints supported. */

#define SND_GET_NUM_ENDPOINTS _IOR(SND_IOCTL_MAGIC, 4, unsigned *)

struct msm_snd_endpoint
{
  int id;			/* input and output */
  char name[64];		/* output only */
};

/* Takes an index between 0 and one less than the number returned by
 * SND_GET_NUM_ENDPOINTS, and returns the SND index and name of a
 * SND endpoint.  On input, the .id field contains the number of the
 * endpoint, and on exit it contains the SND index, while .name contains
 * the description of the endpoint.
 */

#define SND_GET_ENDPOINT _IOWR(SND_IOCTL_MAGIC, 5, struct msm_snd_endpoint *)

#endif
/* ----------  -------- */
/*
static int
msm72xx_enable_audpp (uint16_t enable_mask)
{
  int fd;

//  if (!audpp_filter_inited)
//    return -1;

  fd = open ("/dev/msm_pcm_ctl", O_RDWR);
  if (fd < 0)
    {
      perror ("Cannot open audio device");
      return -1;
    }

  if (enable_mask & ADRC_ENABLE)
    enable_mask &= ~ADRC_ENABLE;
  if (enable_mask & EQ_ENABLE)
    enable_mask &= ~EQ_ENABLE;
  if (enable_mask & IIR_ENABLE)
    enable_mask &= ~IIR_ENABLE;

  printf ("msm72xx_enable_audpp: 0x%04x", enable_mask);
  if (ioctl (fd, AUDIO_ENABLE_AUDPP, &enable_mask) < 0)
    {
      perror ("enable audpp error");
      close (fd);
      return -1;
    }

  close (fd);
  return 0;
}

static int
do_route_audio_rpc (uint32_t device, int ear_mute, int mic_mute)
{
  if (device == -1UL)
    return 0;

  int fd;

  printf ("rpc_snd_set_device(%d, %d, %d)\n", device, ear_mute, mic_mute);

  fd = open ("/dev/msm_snd", O_RDWR);
  if (fd < 0)
    {
      perror ("Can not open snd device");
      return -1;
    }
  struct msm_snd_device_config args;
  args.device = device;
  args.ear_mute = ear_mute ? SND_MUTE_MUTED : SND_MUTE_UNMUTED;
  args.mic_mute = mic_mute ? SND_MUTE_MUTED : SND_MUTE_UNMUTED;

  if (ioctl (fd, SND_SET_DEVICE, &args) < 0)
    {
      perror ("snd_set_device error.");
      close (fd);
      return -1;
    }

  close (fd);
  return 0;
}

static int
set_volume_rpc (uint32_t device, uint32_t method, uint32_t volume)
{
  int fd;

  printf ("rpc_snd_set_volume(%d, %d, %d)\n", device, method, volume);

  if (device == -1UL)
    return 0;

  fd = open ("/dev/msm_snd", O_RDWR);
  if (fd < 0)
    {
      perror ("Can not open snd device");
      return -1;
    }
  struct msm_snd_volume_config args;
  args.device = device;
  args.method = method;
  args.volume = volume;

  if (ioctl (fd, SND_SET_VOLUME, &args) < 0)
    {
      perror ("snd_set_volume error.");
      close (fd);
      return -1;
    }
  close (fd);
  return 0;
}
*/


static const ao_info_t info =
{
    "/dev/msm_pcm_out android output",
    "android",
    "overdose",
    "original pcm by atmosfear"
};

LIBAO_EXTERN(android)
/*
extern int vo_pts;

static char *ao_outputfilename = NULL;
static int ao_pcm_waveheader = 1;
static int fast = 0;
*/

/* init with default values */
static struct msm_audio_stats stats;
static struct msm_audio_config config;
static int afd = NULL;
static int started = 0;
static uint32_t bytestobeused;
static uint32_t bytesused;
static uint32_t bytesinbuffer;
static char * sndbuffer = 0;

// to set/get/query special features/parameters
static int control(int cmd,void *arg){
    return -1;
}


static int init(int rate,int channels,int format,int flags){

	if(format != AFMT_S16_LE && 0)
	{
		printf("mauvais format :\\ ");
		return -1;
	}
	afd = open ("/dev/msm_pcm_out", O_WRONLY | O_NONBLOCK );
	if (afd < 0)
	{
		printf ("pcm_play: cannot open audio device");
	  return -1;
	}
	if (ioctl (afd, AUDIO_GET_CONFIG, &config))
	{
		printf ("could not get config");
		return -1;
	}
	config.channel_count = channels;
	config.sample_rate = rate;
	if (ioctl (afd, AUDIO_SET_CONFIG, &config))
	{
		printf ("could not set config");
	  return -1;
	}
	//if buffer size change ^^
	/*if (ioctl (afd, AUDIO_GET_CONFIG, &config))
	{
		printf ("could not get config");
		return -1;
	}*/
	sndbuffer = malloc(config.buffer_size);
	//ioctl (afd, AUDIO_START, 0);
	started = 1;
	ioctl (afd, AUDIO_GET_STATS, &stats);
	bytestobeused = bytesused = stats.byte_count;
	bytesinbuffer = 0;
	//printf("initialisation ok %5d - %5d, type: %x\n",config.buffer_size,config.buffer_count,config.codec_type);
	return 1;
}

// close audio device
static void uninit(int immed){
    if(afd)
    {
    	ioctl (afd, AUDIO_STOP, 0);
    	close(afd);
    }
    afd = 0;

    if(sndbuffer)
    {
    	free(sndbuffer);
    	sndbuffer = 0;
    }
    started = 0;
    bytestobeused = bytesused = 0;
    bytesinbuffer =0;
}

// stop playing and empty buffers (for seeking/pause)
static void reset(void){
	//
	ioctl (afd, AUDIO_STOP, 0);
	ioctl (afd, AUDIO_FLUSH, 0);
	ioctl (afd, AUDIO_START, 0);
	ioctl (afd, AUDIO_GET_STATS, &stats);
	bytestobeused = bytesused = stats.byte_count;
}

// stop playing, keep buffers (for pause)
static void audio_pause(void)
{
    // for now, just call reset();
    reset();
}

// resume playing, after audio_pause()
static void audio_resume(void)
{
}

static int internal_get_space(void){
	//stats seem to just count whole buffer we write in :\ not the number of samples actually played
	//so this func always return a (x * config.buffer_size) shit;
	ioctl (afd, AUDIO_GET_STATS, &stats);
	bytesused = stats.byte_count;
	if(stats.byte_count > bytestobeused)
		bytestobeused = stats.byte_count;
	//in case of int overflow
	if(bytestobeused < bytesused)
		bytesinbuffer = ( (0xffffffff - ( bytesused-1))+bytestobeused);
	else
		bytesinbuffer = (bytestobeused - bytesused);
	if(bytesinbuffer <= (config.buffer_size * config.buffer_count))
		return (config.buffer_size * config.buffer_count) - bytesinbuffer;
	else
		return 0;
}
// return: how many bytes can be played without blocking
static int get_space(void){
	//a minimum of a buffer is return else mplayer crash
	int nbuff;
	nbuff = internal_get_space();
	return (nbuff?nbuff:config.buffer_size*config.buffer_count);
}

// plays 'len' bytes of 'data'
// it should round it down to outburst*n
// return: number of bytes played
static int play(void* data,int len,int flags){

    unsigned int res;
    unsigned char *cdata;
	//printf("PCM: Writing chunk! %5d\n",len);
	internal_get_space();
	cdata = data;
	bytestobeused += len;
	for( int n=(len/config.buffer_size)+1;n>0;n--)
	{
		memcpy(sndbuffer,cdata,(n>1? config.buffer_size : (len%config.buffer_size)));
		res = write(afd,sndbuffer,(n>1? config.buffer_size : (len%config.buffer_size)));

		//res = write(afd,cdata,(n>1? config.buffer_size : (len%config.buffer_size)));

		cdata += config.buffer_size;
	}
	if(started)
	{
		started = 0;
		ioctl (afd, AUDIO_START, 0);
	}
	return len;
}

// return: delay in seconds between first and last sample in buffer
static float get_delay(void){
	internal_get_space();
	//printf("demande de delay %f\n",((float) (bytesinbuffer) ) / ( (float) (config.sample_rate*2*config.channel_count)));
	//only 16 bit seem supported by msm_pcm_out
    return ((float) (bytesinbuffer) ) / ( (float) (config.sample_rate*2*config.channel_count));
}
