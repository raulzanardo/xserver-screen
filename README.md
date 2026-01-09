X Server Screen - RGB LED Matrix Display
==========================================

This project was inspired by and uses code from [jenissimo/pico8-led](https://github.com/jenissimo/pico8-led).

Display any graphical application on an RGB LED matrix using Raspberry Pi GPIO. This program captures the output from a virtual X server (Xvfb) and renders it in real-time on an LED matrix display.

**⚠️ Compatible with Raspberry Pi 1-4 and Zero only. NOT compatible with Raspberry Pi 5.**

## How It Works

The application:
1. Creates a virtual X server display (192x128 pixels in the current run script configuration)
2. Continuously captures screenshots from the X display
3. Renders each frame to the RGB LED matrix via GPIO
4. Supports real-time display at up to 100 FPS

## Requirements

- Raspberry Pi with GPIO pins (Pi 1, 2, 3, 4, or Zero - **NOT Pi 5**)
- RGB LED Matrix panels (connected via GPIO)
- rpi-rgb-led-matrix library (included as submodule)

## Installation

Clone the repository with submodules:
```bash
git clone --recurse-submodules https://github.com/raulzanardo/xserver-screen.git
cd xserver-screen
```

Install dependencies:
```bash
sudo apt-get install xvfb libx11-dev
```

Build the project:
```bash
make
```

Optionally, install system-wide:
```bash
sudo make install
```
This installs `xserver-screen` and `run` to `/usr/local/bin`.

## Usage

Use the included `run` script to start the virtual display and run your application:

```bash
./run your_application
```

For example:
```bash
./run xclock -geometry 192x128
./run firefox
./run /usr/games/your-game
```

## Configuration

### Command-Line Options

The `xserver-screen` binary supports several command-line options:

```bash
sudo xserver-screen [options] [led-matrix-options]

Options:
  -u, --update-interval <microseconds>  Update interval in microseconds (default: 10000)
  -x, --x-offset <pixels>              Screenshot X offset (default: 0)
  -y, --y-offset <pixels>              Screenshot Y offset (default: 0)
  -h, --help                           Show help message
```

Screenshot dimensions are automatically calculated from the LED matrix configuration.

Example:
```bash
sudo xserver-screen -u 20000 -x 0 -y 0 --led-rows=64 --led-cols=64
```

### LED Matrix Settings

Edit the `run` script to adjust your LED matrix configuration:
- `--led-rows`: Number of rows in each panel (default: 64)
- `--led-cols`: Number of columns in each panel (default: 64)
- `--led-chain`: Number of panels chained horizontally (default: 3)
- `--led-parallel`: Number of parallel chains (default: 2)
- `--led-brightness`: Display brightness 0-100 (default: 60)
- `--led-panel-type`: Panel driver chip type (default: FM6126A)

For more options, see the [rpi-rgb-led-matrix documentation](https://github.com/hzeller/rpi-rgb-led-matrix).

### Display Resolution

The virtual X server resolution is set to 192x128 pixels to match the LED matrix configuration (3 panels × 64 columns = 192 width, 2 panels × 64 rows = 128 height). Adjust the Xvfb resolution in the `run` script to match your matrix setup.

You can also adjust the screenshot region using the command-line options to capture a specific portion of the virtual display.

## Use Cases

- Retro gaming displays
- Information dashboards
- Digital signage
- Visual effects and animations
- Media players
- Any X11 application

## Changes from pico8-led

This project is based on [jenissimo/pico8-led](https://github.com/jenissimo/pico8-led) with the following enhancements:

- **Generalized for any application**: Works with any X11 application, not just PICO-8
- **Automatic dimension detection**: Screenshot size automatically matches LED matrix configuration
- **Flexible capture region**: Added `-x` and `-y` options to capture from any screen position (default: 0,0 instead of 128,128)
- **Simplified run script**: Single `run` script accepts any application as argument
- **System installation**: Added `make install` target for system-wide installation