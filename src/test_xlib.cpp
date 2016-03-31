//============================================================================
// Name        : hello_X11.cpp
// Author      : jun
// Version     :
// Copyright   :
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <X11/Xlib.h>
#include <unistd.h>
#include <cstdio>
#include <iostream>
#include "bd_test.h"
using namespace std;

struct MyXWindow{
	Display * dsp;
	GC gc;
	Window win;
	XEvent evt;
};
int my_x_window_create(MyXWindow & myxwin)
{
	Display *& dsp = myxwin.dsp = XOpenDisplay(0);
	if(!dsp) { return 1; }

	int n = DefaultScreen(dsp);
	auto white = WhitePixel(dsp, n), black = BlackPixel(dsp, n);
	auto & win = myxwin.win = XCreateSimpleWindow(dsp, DefaultRootWindow(dsp), 50, 50, 200, 200, 0, black, white);
	XMapWindow(dsp, win);
	long eventMask = StructureNotifyMask;
	XSelectInput(dsp, win, eventMask);
	XEvent evt;
	do {
		XNextEvent(dsp, &evt); // calls XFlush()
	} while (evt.type != MapNotify);

	GC & gc = myxwin.gc = XCreateGC(dsp, win, 0, NULL);

	XSetForeground(dsp, gc, black);

	return 0;
}
void my_x_window_draw_rectangle(MyXWindow & myxwin, int x, int y, int width, int height)
{
	auto & dsp = myxwin.dsp;
	auto & win = myxwin.win;
	GC & gc = myxwin.gc;
	XDrawLine(dsp, win, gc, x, y, x + width, y);
	XDrawLine(dsp, win, gc, x + width, y, x + width, y + height);
	XDrawLine(dsp, win, gc, x + width, y + height, x, y + height);
	XDrawLine(dsp, win, gc, x, y + height, x, y);

}
int test_xlib_main(int argc, char **argv) {
    cout << "hello_X11" << endl; // prints
    MyXWindow myxwin;
	auto r = my_x_window_create(myxwin);
	if(r != 0) {
		printf("%s", "my_x_window_create failed!\n");
	}
	auto & dsp =myxwin.dsp;
	auto & win = myxwin.win;
	auto & gc = myxwin.gc;
	auto & evt = myxwin.evt;

	my_x_window_draw_rectangle(myxwin, 0, 0, 30, 50);
//	XDrawLine(dsp, win, gc, 10, 10, 190, 190);
//	XFlush(dsp);
//	sleep(2);
//	XClearWindow(dsp, win);
//	XDrawLine(dsp, win, gc, 10, 10, 190, 20);

	long eventMask = ButtonPressMask | ButtonReleaseMask;
	XSelectInput(dsp, win, eventMask); // override prev
	do {
		XNextEvent(dsp, &evt);   // calls XFlush()
	} while (evt.type != ButtonRelease);
	XDestroyWindow(dsp, win);
	XCloseDisplay(dsp);

	return 0;
}
