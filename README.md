X Server Screen - RGB LED Matrix Display
==========================================

This project was inspired by and uses code from [jenissimo/pico8-led](https://github.com/jenissimo/pico8-led).

Display any graphical application on an RGB LED matrix using Raspberry Pi GPIO. This program captures the output from a virtual X server (Xvfb) and renders it in real-time on an LED matrix display.

## How It Works

The application:
1. Creates a virtual X server display (192x128 pixels in the current run script configuration)
2. Continuously captures screenshots from the X display
3. Renders each frame to the RGB LED matrix via GPIO
4. Supports real-time display at up to 100 FPS

## Requirements

- Raspberry Pi with GPIO pins
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

## Usage

Use the included `run` script to start the virtual display and run your application:

```bash
./run your_application
```

For example:
```bash
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
  -w, --width <pixels>                 Screenshot width (default: 192)
  -t, --height <pixels>                Screenshot height (default: 128)
  -h, --help                           Show help message
```

Example:
```bash
sudo xserver-screen -u 20000 -w 128 -t 128 --led-rows=64 --led-cols=64
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