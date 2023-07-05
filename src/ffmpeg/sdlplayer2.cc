//Based on the standard version, SDL Event is introduced.
//The effect is as follows:
//1. The window popped up by SDL can be moved
//2. The screen display is strictly 40ms per frame
/**
 * The simplest FFmpeg-based video player 2 (SDL upgraded version)
 * Simplest FFmpeg Player
 * Version 2 use SDL 2.0 instead of SDL 1.2 in version 1.
 **
 This program realizes the decoding and display of video files (supports
 HEVC, H.264, MPEG2, etc.)
 * It is the simplest tutorial on FFmpeg video decoding.
 * You can understand the decoding process of FFmpeg by studying this
 example.
 * In this version, the SDL message mechanism is used to refresh the video
 screen.
 * This software is a simplest video player based on FFmpeg.
 * Suitable for beginner of FFmpeg.
 **
 Remarks:
 * When the standard version is playing video, the screen display uses a
 40ms delay. This has two consequences:
 * ��1) The window popped up by SDL cannot be moved, and it always
 shows that it is busy
 * ��2��The picture display is not strictly a 40ms frame, because the decoding
 time has not been considered.
 * SU��SDL Update��In the process of video decoding, the version no longer
 uses the 40ms delay method, but is created
 * A thread sends a custom message every 40ms to inform the main
 function to decode and display. After doing so:
 * (1) The window popped up by SDL can be moved
 * ��2��Screen display is strictly 40ms a frame
 * Remark:
 * Standard Version use's SDL_Delay() to control video's frame rate, it has2*
 disadvantages:
 * (1)SDL's Screen can't be moved and always "Busy".
 * (2)Frame rate can't be accurate because it doesn't consider the time
 consumed
 * by avcodec_decode_video2()
 * SU��SDL Update��Version solved 2 problems above. It create a thread to
 send SDL
 * Event every 40ms to tell the main loop to decode and show video frames.
 */
#ifdef CPP_TEST_WITH_FFMPEG
#include <stdio.h>
#define __STDC_CONSTANT_MACROS
#ifdef _WIN32
//Windows
extern "C"
{
#
include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "SDL2/SDL.h"
};
#else
//Linux...
#ifdef __cplusplus
extern "C"
{
#
endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <SDL2/SDL.h>
#ifdef __cplusplus
};
#endif
#endif
//Refresh Event
#define SFM_REFRESH_EVENT (SDL_USEREVENT + 1)
#define SFM_BREAK_EVENT (SDL_USEREVENT + 2)
int thread_exit = 0;
int thread_pause = 0;
int sfp_refresh_thread(void *opaque) {
	thread_exit = 0;
	thread_pause = 0;
	while (!thread_exit) {
		if (!thread_pause) {
			SDL_Event event;
			event.type = SFM_REFRESH_EVENT;
			SDL_PushEvent(&event);
		}
		S
		DL_Delay(40);
	}
	th read_exit = 0;
	thread_pause = 0;
//Break
	SDL_Event event;
	event.type = SFM_BREAK_EVENT;
	SDL_PushEvent(&event);
	return 0;
}
int main(int argc, char *argv[]) {
	AVFormatContext *pFormatCtx;
	int i, videoindex;
	AVCodecContext *pCodecCtx;
	AVCodec *pCodec;
	AVFrame *pFrame, *pFrameYUV;
	unsigned char *out_buffer;
	AVPacket *packet;
	int ret, got_picture;
//------------SDL----------------
	int screen_w, screen_h;
	SDL_Window *screen;
	SDL_Renderer *sdlRenderer;
	SDL_Texture *sdlTexture;
	SDL_Rect sdlRect;
	SDL_Thread *video_tid;
	SDL_Event event;
	struct SwsContext *img_convert_ctx;
	char filepath[] = "Titanic.ts";
	av_register_all();
	avformat_network_init();
	pFormatCtx = avformat_alloc_context();
	if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0) {
		printf("Couldn't open input stream.\n");
		return -1;
	}
	if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
		printf("Couldn't find stream information.\n");
		return -1;
	}
	v ideoindex = -1;
	for (i = 0; i < pFormatCtx->nb_streams; i++)
		if (pFormatCtx->streams[i]->codec-
		>codec_type==AVMEDIA_TYPE_VIDEO) {
			videoindex = i;
			break;
		}
	if (videoindex == -1) {
		printf("Didn't find a video stream.\n");
		return -1;
	}
	pCodecCtx = pFormatCtx->streams[videoindex]->codec;
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL) {
		printf("Codec not found.\n");
		return -1;
	}
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
		printf("Could not open codec.\n");
		return -1;
	}
	p Frame = av_frame_alloc();
	pFrameYUV = av_frame_alloc();
	out_buffer = (unsigned char*) av_malloc(
			av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width,
					pCodecCtx->height, 1));
	av_image_fill_arrays(pFrameYUV->data, pFrameYUV-
			>linesize,out_buffer,
			AV_PIX_FMT_YUV420P,pCodecCtx->width, pCodecCtx-
			>height,1);
//Output Info-----------------------------
	printf("---------------- File Information ---------------\n");
	av_dump_format(pFormatCtx, 0, filepath, 0);
	printf("-------------------------------------------------\n");
	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx-
			>height, pCodecCtx->pix_fmt,
			pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P,
			SWS_BICUBIC, NULL, NULL, NULL);
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
		printf("Could not initialize SDL - %s\n", SDL_GetError());
		return -1;
	} //
	SDL
	2.0
	Support
	for multiple windows
	screen_w = pCodecCtx->width;
	screen_h = pCodecCtx->height;
	screen = SDL_CreateWindow("Simplest ffmpeg player's Window",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screen_w,
			screen_h, SDL_WINDOW_OPENGL);
	if (!screen) {
	printf("SDL: could not create window -
			exiting:%s\n",SDL_GetError());
			return -1;
		}
		s dlRenderer = SDL_CreateRenderer(screen, -1, 0);
//IYUV: Y + U + V (3 planes)
//YV12: Y + V + U (3 planes)
		sdlTexture = SDL_CreateTexture(sdlRenderer,
				SDL_PIXELFORMAT_IYUV,
				SDL_TEXTUREACCESS_STREAMING,pCodecCtx->width,pCodecCtx-
				>height);
		sdlRect.x = 0;
		sdlRect.y = 0;
		sdlRect.w = screen_w;
		sdlRect.h = screen_h;
		packet = (AVPacket*) av_malloc(sizeof(AVPacket));
		video_tid = SDL_CreateThread(sfp_refresh_thread, NULL, NULL);
//------------SDL End------------
//Event Loop
		for (;;) {
//Wait
			SDL_WaitEvent(&event);
			if (event.type == SFM_REFRESH_EVENT) {
				while (1) {
					if (av_read_frame(pFormatCtx, packet) < 0)
						thread_exit = 1;
					if (packet->stream_index == videoindex)
						break;
				}
				ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture,
						packet);
				if (ret < 0) {
					printf("Decode Error.\n");
					return -1;
				}
				if (got_picture) {
					sws_scale(img_convert_ctx, (const unsigned char* const*)pFrame-
							>data, pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data,
							pFrameYUV->linesize);
//SDL---------------------------
					SDL_UpdateTexture(sdlTexture, NULL, pFrameYUV->data[0],
							pFrameYUV->linesize[0]);
					SDL_RenderClear(sdlRenderer);
					SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
					SDL_RenderPresent(sdlRenderer);
//SDL End-----------------------
				}
				a
				v_free_packet( packet);
			} else if (event.type == SDL_KEYDOWN) {
//Pause
				if (event.key.keysym.sym == SDLK_SPACE)
					thread_pause = !thread_pause;
			} else if (event.type == SDL_QUIT) {
				thread_exit = 1;
			} else if (event.type == SFM_BREAK_EVENT) {
				break;
			}
		}
		s
		ws_freeContext( img_convert_ctx);
		SDL_Quit();
		av_frame_free(&pFrameYUV);
		av_frame_free(&pFrame);
		avcodec_close(pCodecCtx);
		avformat_close_input(&pFormatCtx);
		return 0;
	}
#endif
#endif
