#include "video_stream.hpp"
#include <print>
#include <cstddef>

namespace video_stream {
int VideoStream::setup(void) {
    int            ret = 0;
    enum AVCodecID codec_id;

    // Copy file_path string to local src_filename variable
    if (src_filename == NULL || strlen(src_filename) == 0) {
        fprintf(stderr, "\033[1;31mFile path string is NULL or empty.\033[0m\n");
        return -1;
    }

    // open input file, and allocate format context
    ret = avformat_open_input(&fmt_ctx, src_filename, NULL, NULL);
    if (ret < 0) {
        fprintf(stderr, "Could not open source file---> %s\n", src_filename);
        return -1;
    }

    // retrieve stream information
    ret = avformat_find_stream_info(fmt_ctx, NULL);
    if (ret < 0) {
        fprintf(stderr, "Could not find stream information\n");
        ret = -1;
        goto CLEANUP_INPUT_FILE;
    }

    ret = find_best_stream(&video_stream_idx, fmt_ctx, AVMEDIA_TYPE_VIDEO);
    if (ret < 0) {
        ret = -1;
        goto CLEANUP_INPUT_FILE;
    }

    // Add safety check for video stream index
    if (video_stream_idx < 0 || video_stream_idx >= fmt_ctx->nb_streams) {
        fprintf(stderr, "Invalid video stream index\n");
        ret = -1;
        goto CLEANUP_INPUT_FILE;
    }

    video_stream = fmt_ctx->streams[video_stream_idx];
    if (!video_stream || !video_stream->codecpar) {
        fprintf(stderr, "Could not find audio or video stream in the input, aborting\n");
        ret = -1;
        goto CLEANUP_INPUT_FILE;
    }

    codec_id = video_stream->codecpar->codec_id;

    switch (codec_id) {
    case AV_CODEC_ID_H264:
        // payload_type = PT_H264;
        std::println("[ffmpeg_convertor]: AV_CODEC_ID_H264");
        break;

    case AV_CODEC_ID_H265:
        // payload_type = PT_H265;
        std::println("[ffmpeg_convertor]: AV_CODEC_ID_H265");
        break;

    case AV_CODEC_ID_MJPEG:
        // payload_type = PT_MJPEG;
        std::println("[ffmpeg_convertor]: AV_CODEC_ID_MJPEG");
        break;

    default:
        // printf("\nImage or Video codec is not supported\n");
        std::println("[ffmpeg_convertor]: Image or Video codec is not supported");
        ret = -1;
        goto CLEANUP_INPUT_FILE;
    }

#if 1 // TODO: Check debug mode using global definition by the Makefile
    av_dump_format(fmt_ctx, 0, src_filename, 0);
#endif

    pkt = av_packet_alloc();
    if (!pkt) {
        fprintf(stderr, "Could not allocate packet\n");
        ret = AVERROR(ENOMEM);
        goto CLEANUP_INPUT_FILE;
    }

    if (codec_id == AV_CODEC_ID_H264) { // TODO: handle H265 codec bsf filter later
        // Setup mp4 to annexb filter
        const AVBitStreamFilter* pfilter = av_bsf_get_by_name("h264_mp4toannexb");
        if (pfilter == NULL) {
            fprintf(stderr, "Get bsf failed!\n");
            ret = -1;
            goto CLEANUP_PACKET;
        }

        ret = av_bsf_alloc(pfilter, &bsf_ctx);
        if (ret != 0) {
            fprintf(stderr, "Alloc bsf failed!\n");
            return -1;
        }

        ret =
            avcodec_parameters_copy(bsf_ctx->par_in, fmt_ctx->streams[video_stream_idx]->codecpar);
        if (ret < 0) {
            fprintf(stderr, "avcodec_parameters_copy failed!\n");
            ret = -1;
            goto CLEANUP_BSF;
        }

        ret = av_bsf_init(bsf_ctx);
        if (ret < 0) {
            fprintf(stderr, "av_bsf_init failed!\n");
            ret = -1;
            goto CLEANUP_BSF;
        }
    } else {
        goto CLEANUP_INPUT_FILE;
    }

    return 0; // Return here, because there has been no error and **the resources should be freed
              // later**

CLEANUP_BSF:
    if (bsf_ctx) {
        av_bsf_free(&bsf_ctx);
    }

CLEANUP_PACKET:
    if (pkt) {
        av_packet_free(&pkt);
    }

CLEANUP_INPUT_FILE:
    if (fmt_ctx) {
        avformat_close_input(&fmt_ctx);
    }

    bsf_ctx = NULL;
    pkt = NULL;
    fmt_ctx = NULL;
    return ret;
}

void VideoStream::close() {
    if (bsf_ctx) {
        av_bsf_free(&bsf_ctx);
        bsf_ctx = NULL;
    }

    if (pkt) {
        av_packet_free(&pkt);
        pkt = NULL;
    }

    if (fmt_ctx) {
        avformat_close_input(&fmt_ctx);
        fmt_ctx = NULL;
    }
}

VideoStreamLoopReturn VideoStream::loop(void) {
    int ret = 0;
    // FILE* f = NULL;
    std::println("Started");

    // VideoStreamData video_stream_data{};
    bool                                should_sleep{false};
    std::chrono::steady_clock::duration sleep_time{};

    bool     is_first_packet = true;
    auto     stream_start_time = std::chrono::steady_clock::now();
    uint32_t first_rtp_timestamp = 0;

    while (av_read_frame(fmt_ctx, pkt) >= 0) {
        if (pkt->stream_index == video_stream_idx) {

            ///////////////////////////// TIME CALCULATION /////////////////////////////

            // A. CALCULATE THE RTP TIMESTAMP (Your exact code)
            uint32_t current_rtp_ts{0U};
            if (pkt->pts != AV_NOPTS_VALUE) {
                AVRational    rtp_time_base = {1, 90000};
                const int64_t rtp_timestamp_64 =
                    av_rescale_q(pkt->pts, video_stream->time_base, rtp_time_base);
                current_rtp_ts = static_cast<uint32_t>(rtp_timestamp_64);
            } else if (pkt->dts != AV_NOPTS_VALUE) {
                // Pro-tip: Some poorly muxed files lack pts but have dts. Use it as a backup!
                AVRational    rtp_time_base = {1, 90000};
                const int64_t rtp_timestamp_64 =
                    av_rescale_q(pkt->dts, video_stream->time_base, rtp_time_base);
                current_rtp_ts = static_cast<uint32_t>(rtp_timestamp_64);
            } else {
                static uint32_t fallback_ts = 0;
                current_rtp_ts = fallback_ts;
                fallback_ts += 3000; // Assume 30fps
            }

            // B. ESTABLISH TIME ZERO
            if (is_first_packet) {
                first_rtp_timestamp = current_rtp_ts;
                stream_start_time = std::chrono::steady_clock::now();
                is_first_packet = false;
            }

            // C. CALCULATE PACING SLEEP
            uint32_t ts_diff = current_rtp_ts - first_rtp_timestamp;
            auto     expected_elapsed_us =
                std::chrono::microseconds((static_cast<int64_t>(ts_diff) * 1000000) / 90000);
            auto actual_elapsed = std::chrono::steady_clock::now() - stream_start_time;

            if (expected_elapsed_us > actual_elapsed) {
                // std::this_thread::sleep_for(expected_elapsed_us - actual_elapsed);
                should_sleep = true;
                sleep_time = expected_elapsed_us - actual_elapsed;
            } else {
                should_sleep = false;
                sleep_time = std::chrono::steady_clock::duration::zero();
            }

            ///////////////////////////// TIME CALCULATION /////////////////////////////

            // Send packet (MP4 AVCC) to bitstream filter
            ret = av_bsf_send_packet(bsf_ctx, pkt);
            if (ret < 0) {
                fprintf(stderr, "Error sending packet to bitstream filter\n");
                break;
            }

            // Receive filtered packets
            while (1) {
                // Recieve packet (ANNEXB) from bitstream filter
                ret = av_bsf_receive_packet(bsf_ctx, pkt);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    break;
                } else if (ret < 0) {
                    fprintf(stderr, "Error receiving packet from bitstream filter\n");
                    // goto cleanup;
                    av_packet_unref(pkt); // TODO: Ensure this logic is correct and safe
                    continue;             // TODO: Ensure this logic is correct and safe
                }

                // Safety check for packet data
                if (!pkt->data || pkt->size <= 0) {
                    fprintf(stderr, "Invalid packet data\n");
                    continue;
                }

                auto nal_units = parse_nalus(pkt);
                // co_yield nal_units;
                // video_stream_data.nal_units = nal_units;
                VideoStreamData video_stream_data{.finished = false,
                                                  .rtp_timestamp = current_rtp_ts,
                                                  .should_sleep = should_sleep,
                                                  .sleep_time = sleep_time,
                                                  .nal_units{nal_units}};
                co_yield video_stream_data;

                // for (const auto& nal_unit : nal_units) {
                //     co_yield nal_units;
                // }

                // ret = gsf_mpp_vo_vsend(VOLAYER_HD0, 1, 0, pkt->data, &attr);

#if FFMPEG_PRINT_DEBUG_INFO // TODO: Check debug mode using global definition by the Makefile
                printf("vsend ret:%d, video size:%d, pts:%llu\n", ret, attr.size, attr.pts);
#endif
            }
        }

        av_packet_unref(pkt);
    }

    // std::vector<std::vector<uint8_t>> final_ret{{}};
    // std::vector<std::vector<uint8_t>> final_ret{}; // empty vector
    // co_yield final_ret;

    VideoStreamData video_stream_data{.finished = true};
    co_yield video_stream_data;
}

/**
 * @brief Find best video or audio strwm from the file
 *
 * @param stream_idx
 * @param fmt_ctx
 * @param type
 * @return int
 */
int VideoStream::find_best_stream(int* stream_idx, AVFormatContext* fmt_ctx,
                                  enum AVMediaType type) {
    int            ret, stream_index;
    const AVCodec* dec = NULL;

    ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
    if (ret < 0) {
        fprintf(stderr, "Could not find %s stream in input file '%s'\n",
                av_get_media_type_string(type), src_filename);
        return ret;
    }

    stream_index = ret;
    *stream_idx = stream_index;

    return 0;
}

// H.264 NAL Unit Types
enum H264NalUnitType {
    NAL_UNIT_UNSPECIFIED = 0,
    NAL_UNIT_SLICE = 1, // Non-IDR slice
    NAL_UNIT_SLICE_DPA = 2,
    NAL_UNIT_SLICE_DPB = 3,
    NAL_UNIT_SLICE_DPC = 4,
    NAL_UNIT_IDR = 5, // IDR slice (Keyframe)
    NAL_UNIT_SEI = 6,
    NAL_UNIT_SPS = 7, // Sequence Parameter Set
    NAL_UNIT_PPS = 8, // Picture Parameter Set
    NAL_UNIT_AUD = 9, // Access Unit Delimiter
    NAL_UNIT_END_SEQUENCE = 10,
    NAL_UNIT_END_STREAM = 11,
    NAL_UNIT_FILLER = 12
};

std::vector<std::vector<uint8_t>> VideoStream::parse_nalus(const AVPacket* pkt) {
    const uint8_t* data = pkt->data;
    size_t         size = pkt->size;

    int nal_unit_in_pkt_count = 0;

    std::vector<std::vector<uint8_t>> nal_units{};

    size_t i = 0;
    while (i < size) {
        int    start_code_size = 0;
        size_t nal_start = 0;

        // 1. Find the start code (00 00 01 or 00 00 00 01)
        if (i + 2 < size && data[i] == 0x00 && data[i + 1] == 0x00) {
            if (data[i + 2] == 0x01) {
                start_code_size = 3;
                nal_start = i + 3;
            } else if (i + 3 < size && data[i + 2] == 0x00 && data[i + 3] == 0x01) {
                start_code_size = 4;
                nal_start = i + 4;
            }
        }

        if (start_code_size > 0 && nal_start < size) {
            // 2. Extract the NAL Header (the first byte after the start code)
            uint8_t nal_header = data[nal_start];

            // 3. Extract the nal_unit_type (lower 5 bits)
            uint8_t nal_unit_type = nal_header & 0x1F;

            // std::println("/// NAL UNIT TYPE {:02x} ///", nal_unit_type);

            // (Optional) Extract nal_ref_idc (bits 5 and 6)
            // uint8_t nal_ref_idc = (nal_header >> 5) & 0x03;

            // --- DO SOMETHING WITH THE NAL UNIT ---
            // e.g., if (nal_unit_type == NAL_UNIT_SPS) { ... }

            // Find the end of this NAL unit (the next start code or end of packet)
            size_t nal_end = size;
            for (size_t j = nal_start + 1; j + 2 < size; ++j) {
                if (data[j] == 0x00 && data[j + 1] == 0x00 &&
                    (data[j + 2] == 0x01 ||
                     (j + 3 < size && data[j + 2] == 0x00 && data[j + 3] == 0x01))) {
                    nal_end = j;
                    break;
                }
            }

            size_t         nal_size = nal_end - nal_start;
            const uint8_t* nal_payload = &data[nal_start];

            // Now you have:
            // - nal_unit_type
            // - nal_payload (pointer to the raw NAL data WITHOUT the start code)
            // - nal_size (size of the NAL data)
            // You can now build your RTP packets!
            nal_unit_in_pkt_count++;
            // std::vector<uint8_t> my(nal_payload, nal_payload + nal_size);
            nal_units.emplace_back(nal_payload, nal_payload + nal_size);

            // Advance the loop to the next NAL unit
            i = nal_end;
        } else {
            // No more start codes found, break to avoid infinite loop
            break;
        }
    }

    // std::println("/// [ nal_unit_in_pkt_count={} ] ///", nal_unit_in_pkt_count);
    return nal_units;
}
} // namespace video_stream
