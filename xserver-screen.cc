#include "led-matrix.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <string>
#include <vector>

#define DEFAULT_UPDATE_INTERVAL 10000
#define DEFAULT_SCREENSHOT_X 0
#define DEFAULT_SCREENSHOT_Y 0

using rgb_matrix::Canvas;
using rgb_matrix::FrameCanvas;
using rgb_matrix::RGBMatrix;

struct ColorComponentModifier
{
    unsigned long shift;
    unsigned long bits;
};

// Make sure we can exit gracefully when Ctrl-C is pressed.
volatile sig_atomic_t interrupt_received = 0;
static void InterruptHandler(int signo)
{
    interrupt_received = 1;
}

ColorComponentModifier GetColorComponentModifier(unsigned long mask)
{
    ColorComponentModifier color_component_modifier;

    color_component_modifier.shift = 0;
    color_component_modifier.bits = 0;

    while (!(mask & 1))
    {
        color_component_modifier.shift++;
        mask >>= 1;
    }
    while (mask & 1)
    {
        color_component_modifier.bits++;
        mask >>= 1;
    }
    if (color_component_modifier.bits > 8)
    {
        color_component_modifier.shift += color_component_modifier.bits - 8;
        color_component_modifier.bits = 8;
    }

    return color_component_modifier;
}

int ShowScreen(Display *display, size_t x, size_t y, size_t width, size_t height, RGBMatrix *matrix, int update_interval)
{
    FrameCanvas *offscreen_canvas = matrix->CreateFrameCanvas();
    if (offscreen_canvas == NULL)
    {
        fprintf(stderr, "Error: Failed to create frame canvas\n");
        return 1;
    }

    XColor color;
    int screen = XDefaultScreen(display);
    XWindowAttributes attribs;
    Window window = XRootWindow(display, screen);
    XImage *img;
    ColorComponentModifier r_modifier, g_modifier, b_modifier;
    unsigned char color_channel[3];

    XGetWindowAttributes(display, window, &attribs);

    // based on original code from http://www.roard.com/docs/cookbook/cbsu19.html
    r_modifier = GetColorComponentModifier(attribs.visual->red_mask);
    g_modifier = GetColorComponentModifier(attribs.visual->green_mask);
    b_modifier = GetColorComponentModifier(attribs.visual->blue_mask);

    while (!interrupt_received)
    {
        img = XGetImage(display, window, x, y, width, height, AllPlanes, XYPixmap);
        if (img == NULL)
        {
            fprintf(stderr, "Error: Failed to capture screenshot\n");
            break;
        }

        for (int xPixel = 0; xPixel < img->width; xPixel++)
        {
            for (int yPixel = 0; yPixel < img->height; yPixel++)
            {
                color.pixel = XGetPixel(img, xPixel, yPixel);
                color_channel[0] = ((color.pixel >> b_modifier.shift) & ((1 << b_modifier.bits) - 1)) << (8 - b_modifier.bits);
                color_channel[1] = ((color.pixel >> g_modifier.shift) & ((1 << g_modifier.bits) - 1)) << (8 - g_modifier.bits);
                color_channel[2] = ((color.pixel >> r_modifier.shift) & ((1 << r_modifier.bits) - 1)) << (8 - r_modifier.bits);

                offscreen_canvas->SetPixel(xPixel, yPixel, color_channel[2], color_channel[1], color_channel[0]);
            }
        }

        offscreen_canvas = matrix->SwapOnVSync(offscreen_canvas);
        XDestroyImage(img);
        usleep(update_interval);
    }

    return 0;
}

int usage(const char *progname)
{
    fprintf(stderr, "Usage: %s [options] [led-matrix-options]\n", progname);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -u, --update-interval <microseconds>  Update interval in microseconds (default: %d)\n", DEFAULT_UPDATE_INTERVAL);
    fprintf(stderr, "  -x, --x-offset <pixels>              Screenshot X offset (default: %d)\n", DEFAULT_SCREENSHOT_X);
    fprintf(stderr, "  -y, --y-offset <pixels>              Screenshot Y offset (default: %d)\n", DEFAULT_SCREENSHOT_Y);
    fprintf(stderr, "  -h, --help                           Show this help message\n\n");
    fprintf(stderr, "Screenshot dimensions are automatically calculated from LED matrix configuration.\n\n");
    fprintf(stderr, "LED Matrix options:\n");
    rgb_matrix::PrintMatrixFlags(stderr);
    return 1;
}

int main(int argc, char *argv[])
{
    int update_interval = DEFAULT_UPDATE_INTERVAL;
    int screenshot_x = DEFAULT_SCREENSHOT_X;
    int screenshot_y = DEFAULT_SCREENSHOT_Y;

    // Manual parsing of custom options
    std::vector<char *> new_argv;
    new_argv.push_back(argv[0]);
    for (int i = 1; i < argc; ++i)
    {
        const std::string arg = argv[i];
        if (arg == "-u" || arg == "--update-interval")
        {
            if (i + 1 < argc)
            {
                char *endptr;
                long val = strtol(argv[++i], &endptr, 10);
                if (*endptr != '\0' || val <= 0 || val > INT_MAX)
                {
                    fprintf(stderr, "Error: Update interval must be a positive integer\n");
                    return 1;
                }
                update_interval = (int)val;
            }
            else
            {
                fprintf(stderr, "Error: %s requires an argument\n", arg.c_str());
                return usage(argv[0]);
            }
        }
        else if (arg == "-x" || arg == "--x-offset")
        {
            if (i + 1 < argc)
            {
                char *endptr;
                long val = strtol(argv[++i], &endptr, 10);
                if (*endptr != '\0' || val < 0 || val > INT_MAX)
                {
                    fprintf(stderr, "Error: X offset must be a non-negative integer\n");
                    return 1;
                }
                screenshot_x = (int)val;
            }
            else
            {
                fprintf(stderr, "Error: %s requires an argument\n", arg.c_str());
                return usage(argv[0]);
            }
        }
        else if (arg == "-y" || arg == "--y-offset")
        {
            if (i + 1 < argc)
            {
                char *endptr;
                long val = strtol(argv[++i], &endptr, 10);
                if (*endptr != '\0' || val < 0 || val > INT_MAX)
                {
                    fprintf(stderr, "Error: Y offset must be a non-negative integer\n");
                    return 1;
                }
                screenshot_y = (int)val;
            }
            else
            {
                fprintf(stderr, "Error: %s requires an argument\n", arg.c_str());
                return usage(argv[0]);
            }
        }
        else if (arg == "-h" || arg == "--help")
        {
            return usage(argv[0]);
        }
        else
        {
            new_argv.push_back(argv[i]);
        }
    }

    int new_argc = new_argv.size();
    char **new_argv_ptr = new_argv.data();

    // Initialize the RGB matrix
    RGBMatrix::Options matrix_options;
    rgb_matrix::RuntimeOptions runtime_opt;
    if (!rgb_matrix::ParseOptionsFromFlags(&new_argc, &new_argv_ptr,
                                           &matrix_options, &runtime_opt))
    {
        return usage(argv[0]);
    }

    signal(SIGTERM, InterruptHandler);
    signal(SIGINT, InterruptHandler);

    RGBMatrix *matrix = RGBMatrix::CreateFromOptions(matrix_options, runtime_opt);
    if (matrix == NULL)
        return 1;

    // Calculate screenshot dimensions from matrix configuration
    int screenshot_width = matrix->width();
    int screenshot_height = matrix->height();

    Display *display;
    const char *dpy_name = getenv("DISPLAY");

    if (!dpy_name || dpy_name[0] == '\0')
    {
        fprintf(stderr, "Error: DISPLAY environment variable not set\n");
        return 1;
    }

    fprintf(stdout, "DISPLAY is %s\n", dpy_name);
    fprintf(stdout, "Update interval: %d microseconds (%.1f FPS)\n",
            update_interval, 1000000.0 / update_interval);
    fprintf(stdout, "Screenshot region: x=%d, y=%d, width=%d, height=%d\n",
            screenshot_x, screenshot_y, screenshot_width, screenshot_height);

    display = XOpenDisplay(dpy_name);
    if (display == NULL)
    {
        fprintf(stderr, "Error: Display %s cannot be opened\n", dpy_name);
        return 1;
    }

    // Put screenshot from display on RGB Matrix
    ShowScreen(display, screenshot_x, screenshot_y, screenshot_width, screenshot_height, matrix, update_interval);

    matrix->Clear();
    delete matrix;
    XCloseDisplay(display);

    return 0;
}