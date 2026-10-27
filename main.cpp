
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <string>
#include <cstdio>
#include <chrono>
#include <thread>

constexpr int W = 240;
constexpr int H = 240;
constexpr int FPS = 24;

const std::string ramp = " .`',:;~+<>!iIl?tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";

void resize(const unsigned char* src, int sw, int sh, int channels, unsigned char* dst, int dw, int dh) {
    for (int y = 0; y < dh; ++y) {
        for (int x = 0; x < dw; ++x) {
            int sy = y * sh / dh;
            int sx = x * sw / dw;
            for (int c = 0; c < channels; ++c) dst[(y * dw + x) * channels + c] = src[(sy * sw + sx) * channels + c];
        }
    }
}

void sobel(const unsigned char* src, int w, int h, int channels, unsigned char* dst) {
    const int gx[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };

    const int gy[3][3] = {
        {-1, -2, -1},
        { 0,  0,  0},
        { 1,  2,  1}
    };

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int pos = y * w + x;

            if (x == 0 || y == 0 || x == w - 1 || y == h - 1) {
                dst[pos] = ' ';
                continue;
            }

            int sx = 0, sy = 0;

            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    int i = ((y + dy) * w + x + dx) * channels;

                    int gray = (299 * src[i] + 587 * src[i + 1] + 114 * src[i + 2]) / 1000;

                    sx += gx[dy + 1][dx + 1] * gray;
                    sy += gy[dy + 1][dx + 1] * gray;
                }
            }

            int magn = sx * sx + sy * sy;

            if (magn > 300 * 300) {
                double angle = atan2(sy, sx) * 180.0 / M_PI;
                angle = fmod(angle + 180.0, 180.0);

                if (angle < 22.5 || angle >= 157.5) dst[pos] = '|';
                else if (angle < 67.5) dst[pos] = '\\';
                else if (angle < 112.5) dst[pos] = '_';
                else dst[pos] = '/';
            }
            else dst[pos] = ' ';
        }
    }
}

void render(const unsigned char* rgb, int w, int h) {
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int i = (y * w + x) * 3;

            int r = rgb[i];
            int g = rgb[i + 1];
            int b = rgb[i + 2];

            int gray = (299 * r + 587 * g + 114 * b) / 1000;
            char c = ramp[gray * (ramp.size() - 1) / 255];

            std::cout << "\033[38;2;" << r << ';' << g << ';' << b << 'm' << c;
        }

        std::cout << "\033[0m\n";
    }
}

void playVideo(const char* path) {
    std::string quoted = "'";
    for (const char* p = path; *p; ++p) {
        if (*p == '\'') quoted += "'\\''";
        else quoted += *p;
    }
    quoted += "'";

    std::string cmd =
        "ffmpeg -v error -i " + quoted +
        " -an -vf \"fps=24,scale=480:240:force_original_aspect_ratio=decrease,"
        "pad=480:240:(ow-iw)/2:(oh-ih)/2:black\""
        " -f rawvideo -pix_fmt rgb24 -";

    FILE* pipe = popen(cmd.c_str(), "r");

    if (!pipe) {
        std::cerr << "Failed to start FFmpeg\n";
        return;
    }

    std::vector<unsigned char> frame(W * H * 3);
    std::vector<unsigned char> terminalFrame(W * (H / 2) * 3);

    using clock = std::chrono::steady_clock;
    auto nextFrame = clock::now();
    const auto frameDuration = std::chrono::microseconds(1000000 / FPS);

    std::cout << "\033[2J\033[H";

    while (true) {
        size_t bytes = fread(frame.data(), 1, frame.size(), pipe);

        if (bytes != frame.size()) break;

        resize(frame.data(), W, H, 3, terminalFrame.data(), W, H / 2);

        std::cout << "\033[H";
        render(terminalFrame.data(), W, H / 2);
        std::cout.flush();

        nextFrame += frameDuration;
        std::this_thread::sleep_until(nextFrame);
    }

    pclose(pipe);
    std::cout << "\033[0m\n";
}

int main(int argc, char** argv) {
    if (argc == 3 && std::string(argv[1]) == "video") {
        playVideo(argv[2]);
        return 0;
    }

    if (argc != 2) {
        std::cerr << "Usage:\n  ./build/meowscii image.png\n  ./build/meowscii video video.mp4\n";
        return 0;
    }

    int w, h, channels;
    unsigned char* img = stbi_load(argv[1], &w, &h, &channels, 3);

    if (!img) {
        std::cerr << stbi_failure_reason() << '\n';
        return 0;
    }

    channels = 3;

    int nw = std::min(W, w);
    int nh = std::max(1, h * nw / w / 2);

    std::vector<unsigned char> resized(nw * nh * 3);

    resize(img, w, h, channels, resized.data(), nw, nh);
    render(resized.data(), nw, nh);

    stbi_image_free(img);
    return 0;
}