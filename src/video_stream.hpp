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

    int  setup(void);
    void close();

    /**
     * @brief Coroutine generator that reads, timestamps, and parses video frames from an FFmpeg
     * context.
     *
     * @details
     * This function runs as a C++20 coroutine, continuously reading packets from the format
     * context. It processes only packets belonging to the target video stream.
     *
     * Processing steps per frame:
     * 1. Timestamping: Converts the packet PTS (or DTS) to a 90kHz RTP timestamp using
     * av_rescale_q. If neither PTS nor DTS is available, it assumes a constant 30fps (3000 ticks
     * per frame).
     * 2. Pacing: Calculates the required sleep time to maintain real-time playback speed. It
     * compares the expected elapsed time (based on RTP timestamps) against the actual wall-clock
     * elapsed time.
     * 3. Bitstream Filtering: Sends the packet to a bitstream filter context (typically converting
     * MP4/AVCC to Annex B).
     * 4. Parsing: Receives the filtered packet, parses it into individual H.264 NAL units, and
     * yields a VideoStreamData struct containing the payload and pacing instructions.
     *
     * The coroutine terminates when av_read_frame reaches EOF or encounters an error. It performs a
     * final co_yield with VideoStreamData::finished set to true to signal the consumer thread to
     * stop.
     *
     * @return VideoStreamLoopReturn A coroutine generator yielding VideoStreamData structs.
     *
     * @note The pacing logic sets should_sleep and sleep_time but does not sleep internally.
     *       The consumer thread is responsible for executing the sleep before sending the RTP
     * packets.
     */
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
    static constexpr int        H264_RTP_CLOCK_RATE{90000}; // 90 kHz standard for H.264
    static constexpr int64_t    MICROSECONDS_PER_SEC{1000000};
    static constexpr AVRational RTP_TIME_BASE = {1, H264_RTP_CLOCK_RATE};

    static std::vector<std::vector<uint8_t>> parse_nalus(const AVPacket* pkt);
};
} // namespace video_stream
