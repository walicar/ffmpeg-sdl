/* Resources used:
@ffmpeg
https://github.com/leandromoreira/ffmpeg-libav-tutorial/blob/master/0_hello_world.c
https://github.com/FFmpeg/FFmpeg/blob/master/doc/examples/decode_video.c
https://github.com/FFmpeg/FFmpeg/blob/master/doc/examples/scale_video.c
http://www.dranger.com/ffmpeg/tutorial01.html

@SDL
https://lazyfoo.net/tutorials/SDL/01_hello_SDL/index2.php
*/

#include "SDL2/SDL_events.h"
#include "libavutil/frame.h"
#include "libavutil/pixfmt.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <libavcodec/avcodec.h>
#include <libavcodec/codec.h>
#include <libavcodec/codec_id.h>
#include <libavcodec/codec_par.h>
#include <libavcodec/defs.h>
#include <libavcodec/packet.h>
#include <libavformat/avformat.h> // for containers
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <stdio.h>
#include <stdlib.h>

const int SCR_WIDTH = 470;
const int SCR_HEIGHT = 280;
const enum AVPixelFormat PIX_FMT = AV_PIX_FMT_YUV420P;

static int decode_packet(AVPacket *packet, AVCodecContext *codec_ctx,
                         AVFrame *frame, AVFrame *dst_frame,
                         struct SwsContext *sws_ctx, SDL_Renderer *renderer,
                         SDL_Texture *texture);
static void display_frame(AVFrame *frame, SDL_Renderer *renderer,
                          SDL_Texture *texture);

int main(int argc, char **argv) {
  const char *infile;

  if (argc <= 1) {
    fprintf(stderr, "Usage: %s <input file>\n", argv[0]);
    exit(0);
  }

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    fprintf(stderr, "Could not initialize SDL\n");
    exit(1);
  }

  SDL_Window *window = SDL_CreateWindow(
      "ffmpeg-sdl", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCR_WIDTH,
      SCR_HEIGHT, SDL_WINDOW_SHOWN);

  if (!window) {
    fprintf(stderr, "Could not create SDL window, SDL_ERROR: %s\n",
            SDL_GetError());
    exit(1);
  }

  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  if (!renderer) {
    fprintf(stderr, "Could not create SDL renderer, SDL_ERROR: %s\n",
            SDL_GetError());
    exit(1);
  }

  SDL_Texture *texture =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV,
                        SDL_TEXTUREACCESS_STREAMING, SCR_WIDTH, SCR_HEIGHT);

  if (!texture) {
    fprintf(stderr, "Could not create SDL texture, SDL_ERROR: %s\n",
            SDL_GetError());
    exit(1);
  }

  AVFormatContext *format_ctx = avformat_alloc_context();
  if (!format_ctx) {
    fprintf(stderr, "Could not allocate format context\n");
    exit(1);
  };

  infile = argv[1];
  if (avformat_open_input(&format_ctx, infile, NULL, NULL) < 0) {
    fprintf(stderr, "Could not open file to get format\n");
    exit(1);
  }

  if (avformat_find_stream_info(format_ctx, NULL) < 0) {
    fprintf(stderr, "Could not find stream info\n");
    exit(1);
  }

  AVCodecParameters *params = NULL;
  int vid_index = -1;
  int codec_id = -1;
  for (int i = 0; i < format_ctx->nb_streams; i++) {
    if (format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      vid_index = i;
      params = format_ctx->streams[i]->codecpar;
    }
  }

  const AVCodec *codec = avcodec_find_decoder(params->codec_id);
  if (!codec) {
    fprintf(stderr, "Could not find codec H264\n");
    exit(1);
  }

  AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
  if (!codec_ctx) {
    fprintf(stderr, "Could not allocate codec context\n");
    exit(1);
  }

  // adding this resolved h264 NAL error
  if (avcodec_parameters_to_context(codec_ctx, params) < 0) {
    fprintf(stderr, "Could not copy codec params to codec context\n");
    exit(1);
  }

  if (avcodec_open2(codec_ctx, codec, NULL) < 0) {
    fprintf(stderr, "Could not open codec\n");
    exit(1);
  }

  struct SwsContext *sws_ctx =
      sws_getContext(codec_ctx->width, codec_ctx->height, PIX_FMT, SCR_WIDTH,
                     SCR_HEIGHT, PIX_FMT, SWS_BILINEAR, NULL, NULL, NULL);

  if (!sws_ctx) {
    fprintf(stderr, "Could not get sws context\n");
    exit(1);
  }

  AVPacket *packet = av_packet_alloc();
  if (!packet) {
    fprintf(stderr, "Could not allocate packet\n");
    exit(1);
  }

  AVFrame *frame = av_frame_alloc();
  if (!frame) {
    fprintf(stderr, "Could not allocate frame\n");
    exit(1);
  }

  AVFrame *dst_frame = av_frame_alloc();
  if (!dst_frame) {
    fprintf(stderr, "Could not allocate destination frame\n");
    exit(1);
  }

  dst_frame->format = PIX_FMT;
  dst_frame->width = SCR_WIDTH;
  dst_frame->height = SCR_HEIGHT;

  // fill the Packet with data from the Stream
  // https://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html#ga4fdb3084415a82e3810de6ee60e46a61
  // first two packets are NAL

  SDL_Event event;
  int running = 1;
  while (av_read_frame(format_ctx, packet) >= 0 && running) {
    if (packet->stream_index == vid_index) {
      if (decode_packet(packet, codec_ctx, frame, dst_frame, sws_ctx, renderer,
                        texture) < 0)
        break;
    }
    av_packet_unref(packet);
    // required to get screen to show up
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_KEYDOWN) {
        running = 0;
      }
    }
  }

  // free resources
  avformat_close_input(&format_ctx);
  av_packet_free(&packet);
  av_frame_free(&frame);
  av_frame_free(&dst_frame);
  avcodec_free_context(&codec_ctx);
  sws_freeContext(sws_ctx);
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}

static int decode_packet(AVPacket *packet, AVCodecContext *codec_ctx,
                         AVFrame *frame, AVFrame *dst_frame,
                         struct SwsContext *sws_ctx, SDL_Renderer *renderer,
                         SDL_Texture *texture) {
  if (avcodec_send_packet(codec_ctx, packet) < 0) {
    fprintf(stderr, "Could not send packet to decoder\n");
    return -1;
  }

  int response = 0;
  // https://ffmpeg.org/doxygen/trunk/group__lavc__encdec.html#ga4c1691163d2b0616f21af5fed28b6de3
  while (response >= 0) {
    int response = avcodec_receive_frame(codec_ctx, frame);
    if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
      // https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html#ga11e6542c4e66d3028668788a1a74217c
      break;
    } else if (response < 0) {
      fprintf(stderr, "Could not send packet to decoder\n");
      return response;
    }

    if (av_image_alloc(dst_frame->data, dst_frame->linesize, SCR_WIDTH,
                       SCR_HEIGHT, PIX_FMT, 32) < 0) {
      fprintf(stderr, "Could not alloc image\n");
      return -1;
    }

    sws_scale(sws_ctx, (const uint8_t *const *)frame->data, frame->linesize, 0,
              frame->height, dst_frame->data, dst_frame->linesize);
    display_frame(dst_frame, renderer, texture);
  }
  return 0;
}

static void display_frame(AVFrame *frame, SDL_Renderer *renderer,
                          SDL_Texture *texture) {
  SDL_UpdateYUVTexture(texture, NULL, frame->data[0], frame->linesize[0],
                       frame->data[1], frame->linesize[1], frame->data[2],
                       frame->linesize[2]);
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}