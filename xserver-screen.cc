#include "led-matrix.h"

#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <string>
#include <vector>

#define DEFAULT_UPDATE_INTERVAL 10000
#define DEFAULT_SCREEENSHOT_X 0
#define DEFAULT_SCREEENSHOT_Y 0

using rgb_matrix::Canvas;
using rgb_matrix::FrameCanvas;
using rgb_matrix::RGBMatrix;

struct ColorComponentModifier
{
    unsigned long shift;
    unsigned long bits;
};

// Make sure we can exit gracefully when Ctrl-C is pressed.
volatile bool interrupt_received = false;
static void InterruptHandler(int signo)
{
    interrupt_received = true;
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
        if (interrupt_received)
            break;

        img = XGetImage(display, window, x, y, width, height, AllPlanes, XYPixmap);

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
    fprintf(stderr, "  -x, --x-offset <pixels>              Screenshot X offset (default: %d)\n", DEFAULT_SCREEENSHOT_X);
    fprintf(stderr, "  -y, --y-offset <pixels>              Screenshot Y offset (default: %d)\n", DEFAULT_SCREEENSHOT_Y);
    fprintf(stderr, "  -h, --help                           Show this help message\n\n");
    fprintf(stderr, "Screenshot dimensions are automatically calculated from LED matrix configuration.\n\n");
    fprintf(stderr, "LED Matrix options:\n");
    rgb_matrix::PrintMatrixFlags(stderr);
    return 1;
}

int main(int argc, char *argv[])
{
    int update_interval = DEFAULT_UPDATE_INTERVAL;
    int screenshot_x = DEFAULT_SCREEENSHOT_X;
    int screenshot_y = DEFAULT_SCREEENSHOT_Y;

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
                update_interval = atoi(argv[++i]);
                if (update_interval <= 0)
                {
                    fprintf(stderr, "Error: Update interval must be positive\n");
                    return 1;
                }
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
                screenshot_x = atoi(argv[++i]);
                if (screenshot_x < 0)
                {
                    fprintf(stderr, "Error: X offset must be non-negative\n");
                    return 1;
                }
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
                screenshot_y = atoi(argv[++i]);
                if (screenshot_y < 0)
                {
                    fprintf(stderr, "Error: Y offset must be non-negative\n");
                    return 1;
                }
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
    char *dpy_name = std::getenv("DISPLAY");

    if (!dpy_name)
    {
        fprintf(stderr, "No DISPLAY set\n");
        return 1;
    }

    fprintf(stdout, "DISPLAY is %s:\n", dpy_name);
    fprintf(stdout, "Update interval: %d microseconds (%.1f FPS)\n",
            update_interval, 1000000.0 / update_interval);
    fprintf(stdout, "Screenshot region: x=%d, y=%d, width=%d, height=%d\n",
            screenshot_x, screenshot_y, screenshot_width, screenshot_height);

    display = XOpenDisplay(dpy_name);
    if (display == NULL)
    {
        fprintf(stderr, "Display %s cannot be found, exiting", dpy_name);
        return 1;
    }

    // Put screnshot from display on RGB Matrix
    ShowScreen(display, screenshot_x, screenshot_y, screenshot_width, screenshot_height, matrix, update_interval);

    XCloseDisplay(display);
    matrix->Clear();
    delete matrix;

    return 0;
}