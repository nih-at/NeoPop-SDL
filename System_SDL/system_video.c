/* $NiH$ */
/*
  system_video.c -- record video
  Copyright (C) 2004 Thomas Klausner and Dieter Baron

  This file is part of NeoPop-SDL, a NeoGeo Pocket emulator
  The author can be contacted at <wiz@danbala.tuwien.ac.at>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "config.h"

#ifdef HAVE_FFMPEG

#include <ffmpeg/avformat.h>

#include "NeoPop-SDL.h"

#define DO_AUDIO

#define VIDEO_BITRATE	1200000
#define AUDIO_BITRATE	128000

static int x_off, y_off;
static AVFormatContext *oc;
static AVStream *st_v, *st_a;
static AVFrame *picyuv, *picrgb;
static unsigned char buf_v[200000];
static unsigned char buf_ai[10000];
static unsigned char buf_ao[10000];
static int buf_aoff;

static AVFrame *alloc_pic(int, int, int);
static void fill_pic(AVFrame *);




int
system_video_close(void)
{
    avcodec_close(&st_v->codec);
#ifdef DO_AUDIO
    avcodec_close(&st_a->codec);
#endif

    av_write_trailer(oc);
    url_fclose(&oc->pb);

    /* XXX: free oc */

    return 0;
}



void
system_video_init(void)
{
    av_register_all();
}



int
system_video_open(const char *fname)
{
    AVOutputFormat *fmt;
    AVCodecContext *c;
    AVCodec *cd;

    if ((fmt=guess_format(NULL, fname, NULL)) == NULL)
	if ((fmt=guess_format("mpeg", NULL, NULL)) == NULL)
	    return -1;

    if ((oc=malloc(sizeof(*oc))) == NULL)
	return -1;
    memset(oc, 0, sizeof(*oc));
    oc->oformat = fmt;
    strlcpy(oc->filename, fname, sizeof(oc->filename));

    if ((st_v=av_new_stream(oc, 0)) == NULL)
	return -1;
    c = &st_v->codec;
    c->codec_id = fmt->video_codec;
    c->codec_type = CODEC_TYPE_VIDEO;
    c->bit_rate = VIDEO_BITRATE;
    c->width = (SCREEN_WIDTH+15) & ~0xf;
    c->height = (SCREEN_HEIGHT+15) & ~0xf;
    c->frame_rate = 60;
    c->frame_rate_base = 1;
    c->gop_size = 12;
    x_off = (c->width - SCREEN_WIDTH) / 2;
    y_off = (c->height - SCREEN_HEIGHT) / 2;

#ifdef DO_AUDIO
    if ((st_a=av_new_stream(oc, 1)) == NULL)
	return -1;
    c = &st_a->codec;
    c->codec_id = fmt->audio_codec;
    c->codec_type = CODEC_TYPE_AUDIO;
    c->bit_rate = AUDIO_BITRATE;
    c->sample_rate = samplerate;
    c->channels = 1;
#endif

    if (av_set_parameters(oc, NULL) < 0)
	return -1;
#if 0
    dump_format(oc, 0, oc->filename, 1);
#endif

    c = &st_v->codec;
    if ((cd=avcodec_find_encoder(c->codec_id)) < 0)
	return -1;
    if (avcodec_open(c, cd) < 0)
	return -1;
    if ((picrgb=alloc_pic(PIX_FMT_RGB24, c->width, c->height)) == NULL)
	return -1;
    if (c->pix_fmt != PIX_FMT_RGB24) {
	if ((picyuv=alloc_pic(c->pix_fmt, c->width, c->height)) == NULL)
	    return -1;
    }

#ifdef DO_AUDIO
    c = &st_a->codec;
    if ((cd=avcodec_find_encoder(c->codec_id)) < 0)
	return -1;
    if (avcodec_open(c, cd) < 0)
	return -1;
    buf_aoff = 0;
#endif

    if (url_fopen(&oc->pb, oc->filename, URL_WRONLY) < 0)
	return -1;

    av_write_header(oc);

    return 0;
}



void
system_video_write_aframe(unsigned char *samples, int bpf)
{
#ifdef DO_AUDIO
    AVCodecContext *c;
    int size, len;

    c = &st_a->codec;

    len = bpf;

    while (len > 0) {
	if (buf_aoff+len > c->frame_size*2)
	    size = c->frame_size*2 - buf_aoff;
	else
	    size = len;

	memcpy(buf_ai+buf_aoff, samples, size);
	buf_aoff += size;
	samples += size;
	len -= size;

	if (buf_aoff >= c->frame_size*2) {
	    size = avcodec_encode_audio(c, buf_ao, sizeof(buf_ao),
					(unsigned short *)buf_ai);
	    av_write_frame(oc, st_a->index, buf_ao, size);
	    buf_aoff = 0;
	}
    }
#endif
}



void
system_video_write_vframe(void)
{
    AVCodecContext *c;
    AVFrame *pic;
    int size;

    fill_pic(picrgb);

    c = &st_v->codec;

    if (c->pix_fmt == PIX_FMT_RGB24)
	pic = picrgb;
    else {
	img_convert((AVPicture *)picyuv, c->pix_fmt,
		    (AVPicture *)picrgb, PIX_FMT_RGB24,
		    c->width, c->height);
	pic = picyuv;
    }

    /* XXX: handle RAWPICTURE */

    size = avcodec_encode_video(c, buf_v, sizeof(buf_v), pic);
    if (size > 0)
	av_write_frame(oc, st_v->index, buf_v, size);
}



static AVFrame *
alloc_pic(int fmt, int width, int height)
{
    AVFrame *pic;
    unsigned char *dat;
    int size;

    if ((pic=malloc(sizeof(*pic))) == NULL)
	return NULL;
    memset(pic, 0, sizeof(*pic));

    size = avpicture_get_size(fmt, width, height);
    if ((dat=malloc(size)) == NULL) {
	free(pic);
	return NULL;
    }
    memset(dat, 0, size);

    avpicture_fill((AVPicture *)pic, dat, fmt, width, height);

    return pic;
}



static void
fill_pic(AVFrame *pic)
{
    int x, y;
    int pitch;
    unsigned char *p;
    _u16 pix;

    pitch = pic->linesize[0];

    for (y=0; y<SCREEN_HEIGHT; y++) {
	p = pic->data[0] + pitch*(y_off+y) + x_off*3;
	for (x=0; x<SCREEN_WIDTH; x++) {
	    pix = cfb[y*SCREEN_WIDTH+x];

	    *(p++) = CONV4TO8(pix&0xf);
	    *(p++) = CONV4TO8((pix>>4)&0xf);
	    *(p++) = CONV4TO8(pix>>8);
	}
    }
}

#endif
