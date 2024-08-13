/*
Resources Used:
https://github.com/leandromoreira/ffmpeg-libav-tutorial/blob/master/0_hello_world.c
https://github.com/FFmpeg/FFmpeg/blob/master/doc/examples/decode_video.c
http://www.dranger.com/ffmpeg/tutorial01.html
@TODO:
https://stackoverflow.com/questions/39105571/decoding-mp4-mkv-using-ffmpeg-fails
*/

#include <libavcodec/avcodec.h> // codecs
#include <libavcodec/codec.h>
#include <libavcodec/codec_id.h>
#include <libavcodec/codec_par.h>
#include <libavcodec/defs.h>
#include <libavcodec/packet.h>
#include <libavformat/avformat.h> // for containers
#include <libavutil/avutil.h>
#include <stdio.h>
#include <stdlib.h>

static void save_frame(unsigned char *buf, int wrap, int height, int width,
                       char *filename);
static int decode_packet(AVPacket *packet, AVCodecContext *codec_ctx,
                         AVFrame *frame);

int main(int argc, char **argv) {
  const char *infile;

  if (argc <= 1) {
    fprintf(stderr, "Usage: %s <input file>\n", argv[0]);
    exit(0);
  }

  infile = argv[1];

  AVFormatContext *format_ctx;
  format_ctx = avformat_alloc_context();
  if (!format_ctx) {
    fprintf(stderr, "Could not allocate format context\n");
    exit(1);
  };

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
  if (!codec) {
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

  AVFrame *frame = av_frame_alloc();
  if (!frame) {
    fprintf(stderr, "Could not allocate frame\n");
    exit(1);
  }

  AVPacket *packet = av_packet_alloc();
  if (!packet) {
    fprintf(stderr, "Could not allocate packet\n");
    exit(1);
  }

  // fill the Packet with data from the Stream
  // https://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html#ga4fdb3084415a82e3810de6ee60e46a61
  int amt_packets = 8;
  while (av_read_frame(format_ctx, packet) >= 0) {
    if (packet->stream_index == vid_index) {
      if (decode_packet(packet, codec_ctx, frame) < 0)
        break;
      if (--amt_packets <= 0)
        break;
    }
    av_packet_unref(packet);
  }

  // free resources
  avformat_close_input(&format_ctx);
  av_packet_free(&packet);
  av_frame_free(&frame);
  avcodec_free_context(&codec_ctx);
  return 0;
}

static int decode_packet(AVPacket *packet, AVCodecContext *codec_ctx,
                         AVFrame *frame) {
  if (avcodec_send_packet(codec_ctx, packet) < 0) {
    fprintf(stderr, "Could not send packet to decoder\n");
    return -1;
  }
  int response = 0;

  while (response >= 0) {
    int response = avcodec_receive_frame(codec_ctx, frame);
    if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
      // https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html#ga11e6542c4e66d3028668788a1a74217c
      break;
    } else if (response < 0) {
      fprintf(stderr, "Could not send packet to decoder\n");
      return response;
    }

    char frame_filename[1024];
    snprintf(frame_filename, sizeof(frame_filename), "%s-%d.pgm", "frame",
             (int)codec_ctx->frame_num);
    save_frame(frame->data[0], frame->linesize[0], frame->height, frame->width,
               frame_filename);
  }

  return 0;
}

static void save_frame(unsigned char *buf, int wrap, int height, int width,
                       char *filename) {
  printf("Saving file %s\n", filename);
  FILE *f;
  f = fopen(filename, "w");
  fprintf(f, "P5\n%d %d\n%d\n", width, height, 255);

  for (int i = 0; i < height; i++)
    fwrite(buf + i * wrap, 1, width, f);
  fclose(f);
}