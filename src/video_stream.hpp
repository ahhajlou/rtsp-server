#pragma once

#include <cstdint>
#include <vector>
#include <generator>
#include <expected>
#include <chrono>

extern "C" {
#include <libavutil/mem.h>
#include <libavutil/timestamp.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavcodec/bsf.h>
#include <libswscale/swscale.h>
}

namespace video_stream {

struct VideoStreamData {
    bool                                finished{false};
    uint32_t                            rtp_timestamp{};
    bool                                should_sleep{false};
    std::chrono::steady_clock::duration sleep_time{};
    std::vector<std::vector<uint8_t>>   nal_units;
};

// using VideoStreamLoopReturn = std::generator<std::vector<std::vector<uint8_t>>>;
using VideoStreamLoopReturn = std::generator<VideoStreamData>;

class VideoStream {
  public:
    VideoStream() = default;
    ~VideoStream() { close(); };

    int                   setup(void);
    void                  close();
    VideoStreamLoopReturn loop(void);
    int find_best_stream(int* stream_idx, AVFormatContext* fmt_ctx, enum AVMediaType type);

  private:
    AVPacket*                    pkt = NULL;
    AVFormatContext*             fmt_ctx = NULL;
    AVBSFContext*                bsf_ctx = NULL;
    AVFormatContext*             ofmt_ctx = NULL;
    AVStream*                    video_stream = NULL;
    int                          video_stream_idx = -1;
    static constexpr const char* src_filename =
        "/home/amirhossein/Github/rtsp-server/assets/out.mp4";

    static std::vector<std::vector<uint8_t>> parse_nalus(const AVPacket* pkt);
};
} // namespace video_stream
