// Nanami deskpet
// Copyright (C) 2026  CorvusCinereus
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

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
