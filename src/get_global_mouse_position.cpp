#if defined(_WIN32)
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#elif defined(__APPLE__)
    #include <ApplicationServices/ApplicationServices.h>
#else // Linux (X11)
    #define X_DONT_DEFINE_FONT
    #include <X11/Xlib.h>
    #include <stdlib.h>
#endif

// 获取全局屏幕坐标
void getGlobalMousePosition(float& x, float& y) {
    x = -1.0, y = -1.0;

#if defined(_WIN32)
    POINT p;
    if (GetCursorPos(&p)) {
        x = static_cast<float>(p.x);
        y = static_cast<float>(p.y);
    }
#elif defined(__APPLE__)
    CGEventRef event = CGEventCreate(NULL);
    CGPoint p = CGEventGetLocation(event);
    CFRelease(event);
    x = static_cast<float>(p.x);
    y = static_cast<float>(p.y);
#else // Linux (X11)
    Display *dpy = XOpenDisplay(NULL);
    if (dpy) {
        int screen = DefaultScreen(dpy);
        Window root = RootWindow(dpy, screen);
        Window root_return, child_return;
        int root_x, root_y, win_x, win_y;
        unsigned int mask_return;

        if (XQueryPointer(dpy, root, &root_return, &child_return,
                         &root_x, &root_y, &win_x, &win_y, &mask_return)) {
            x = static_cast<float>(root_x);
            y = static_cast<float>(root_y);
        }
        XCloseDisplay(dpy);
    }
#endif
}
