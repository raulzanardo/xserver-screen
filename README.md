Displaying Any Software on RGB LED Display with Raspberry Pi GPIO
==================================================
This program runs any graphical application in a virtual X-server (Xvfb), captures screenshots at regular intervals, and displays them on an LED matrix connected via Raspberry Pi GPIO.

Instructions
--------
Clone repo recursively with submodules
```
git clone --recurse-submodules https://github.com/jenissimo/pico8-led.git
```

Install dependencies:
```
sudo apt-get install xvfb libx11-dev
```

Adjust settings of your LED matrix in `run_led.sh` using `rpi-rgb-led-matrix` documentation: https://github.com/hzeller/rpi-rgb-led-matrix/blob/master/README.md

Running
--------
Run any graphical application:
```
DISPLAY=:1 xvfb-run -s "-screen 0 128x128x24" your_application &
./led_display
```

Or modify the included shell scripts to run your specific application.

Example uses:
- Games and gaming engines
- Visualization tools
- Custom graphical applications
- Terminal applications with GUI
- Media players
- Web browsers in kiosk mode

Configuration
--------
You can adjust the screenshot capture interval and display settings by modifying the source code. The default capture rate is every 10000 microseconds (100 FPS).

For input device configuration, refer to your specific application's documentation.
