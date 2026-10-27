# meowscii 🐈

**A true-color ASCII image and video renderer for your terminal.**

`meowscii` transforms images and videos into colored ASCII art, rendering every character directly in your terminal using ANSI true-color escape sequences.

Built with C++, [stb_image](https://github.com/nothings/stb) for image loading, and FFmpeg for video decoding.

## Preview

![meowscii preview](preview.png)

*Best experienced in a black terminal with a tiny font.*

## Features

- **True-color rendering** — preserves original pixel colors using 24-bit ANSI RGB.
- **ASCII shading** — maps pixel brightness to a character ramp, from dark to bright.
- **Image rendering** — loads images using `stb_image`.
- **Video playback** — decodes video frames through FFmpeg and renders them in the terminal.
- **Lightweight processing** — works with raw RGB pixel buffers without a graphical interface.
- **Terminal-native output** — uses ANSI escape sequences to control colors and the cursor.

## Requirements

- A C++17-compatible compiler.
- CMake 3.15 or later.
- FFmpeg for video playback.
- A terminal supporting ANSI escape sequences and 24-bit true color.
- `stb_image.h` for image loading.

## Setup

### 1. Clone the repository

```bash
git clone https://github.com/YOUR_USERNAME/meowscii.git
cd meowscii
```

Replace `YOUR_USERNAME` with your GitHub username.

### 2. Install dependencies

**Arch Linux**

```bash
sudo pacman -S cmake base-devel ffmpeg
```

**Ubuntu / Debian**

```bash
sudo apt update
sudo apt install cmake build-essential ffmpeg
```

### 3. Add stb_image

Place `stb_image.h` in the project root, alongside `main.cpp` and `CMakeLists.txt`.

You can get it from the [official stb repository](https://github.com/nothings/stb/blob/master/stb_image.h).

### 4. Build

Configure the project and compile it using CMake:

```bash
cmake -S . -B build
cmake --build build -j$(nproc)
```

The executable will be generated at:

```text
build/meowscii
```

## Usage

### Images

```bash
./build/meowscii image.png
```

`meowscii` loads the image, scales it to fit within a maximum width of 240 characters, and renders each pixel as a colored ASCII character.

Supported image formats depend on the formats enabled in `stb_image`.

### Videos

```bash
./build/meowscii video video.mp4
```

Video decoding is handled by FFmpeg. Frames are converted to raw RGB24 data and streamed to the program through a pipe.

The current video configuration targets:

- **Frame rate:** 24 FPS
- **FFmpeg output:** 480 × 240 pixels
- **Terminal output:** 240 × 120 characters
- **Color format:** RGB24
- **Audio:** Disabled

The video output is resized vertically to half its pixel height to better match the proportions of terminal characters.

## Recommended Terminal Settings

`meowscii` is designed to look best with an unusually small font and a completely black background.

| Setting | Value |
|---|---|
| Background | `#000000` |
| Font size | `2` |
| Color support | 24-bit true color |
| Terminal | Any terminal supporting ANSI RGB escape sequences |
| Window size | Large enough to display the rendered output |

### Why a font size of 2?

The renderer outputs hundreds of colored characters per frame. With a tiny font, individual characters blend together, creating the appearance of a continuous image rather than a grid of symbols.

The black background also allows darker areas to blend into the terminal, enhancing contrast.

These settings are recommendations for the intended visual effect, not hard requirements.

## How It Works

The rendering pipeline consists of several stages.

### 1. Input decoding

For images, `stb_image` loads the input into an RGB pixel buffer.

For videos, FFmpeg decodes the source and streams raw RGB24 frames through a pipe opened with `popen()`.

### 2. Resizing

The program uses nearest-neighbor sampling to resize images. Each destination pixel copies a corresponding source pixel without interpolation.

For video, FFmpeg scales and pads the frame to fit a 480 × 240 output canvas. The program then resizes the frame to the dimensions used for terminal rendering.

### 3. Brightness calculation

Each RGB pixel is converted to grayscale using a weighted brightness approximation:

```cpp
gray = (299 * r + 587 * g + 114 * b) / 1000;
```

Green contributes the most to the result, followed by red and blue.

### 4. ASCII mapping

The grayscale value determines which character is selected from a predefined character ramp:

```cpp
const std::string ramp = " .`',:;~+<>!iIl?tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";
```

Characters near the beginning represent darker pixels, while characters near the end represent brighter pixels.

### 5. True-color rendering

Each character is printed using an ANSI 24-bit foreground color sequence:

```text
ESC[38;2;R;G;Bm
```

The character provides the brightness and shape, while the original pixel's RGB values provide the color.

This preserves the source image's colors instead of converting the entire image into monochrome ASCII art.

### 6. Video playback

During playback, the program:

1. Reads a frame from FFmpeg.
2. Resizes the frame for terminal output.
3. Moves the cursor back to the beginning of the terminal.
4. Renders the frame using colored ASCII characters.
5. Flushes stdout.
6. Waits until the next frame deadline.

Reusing the terminal screen avoids clearing the entire display between frames and helps reduce flicker.

## Project Structure

```text
meowscii/
├── main.cpp
├── CMakeLists.txt
├── stb_image.h
├── preview.png
├── README.md
└── LICENSE
```

The `build/` directory is generated by CMake and should generally be excluded from version control.

Add the following to `.gitignore`:

```gitignore
/build/
```

## Limitations

- Video playback has no audio.
- Video frame dimensions and frame rate are currently fixed in the FFmpeg command.
- Nearest-neighbor resizing can introduce aliasing.
- Playback timing depends on decoding speed, rendering overhead, and terminal performance.
- True-color output requires terminal support for 24-bit ANSI colors.
- The visual result depends on terminal font, character dimensions, and background color.
- Video playback requires FFmpeg to be installed and accessible through `PATH`.

## License

This project is released under the **CC0 1.0 Universal** license.

See [LICENSE](https://github.com/let-def/stb_image/blob/master/LICENSE) for details.