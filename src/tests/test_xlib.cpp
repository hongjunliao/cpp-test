//============================================================================
// Name        : hello_X11.cpp
// Author      : jun
// Version     :
// Copyright   :
// Description : Hello World in C++, Ansi-style
//============================================================================

#ifdef __CYGWIN_GCC__
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
	auto & win = myxwin.win = XCreateSimpleWindow(dsp, DefaultRootWindow(dsp), 50, 50, 800, 600, 0, black, white);
	XMapWindow(dsp, win);

	GC & gc = myxwin.gc = XCreateGC(dsp, win, 0, NULL);
	XSetForeground(dsp, gc, black);
	return 0;
}

int my_x_window_create(MyXWindow & myxwin, MyXWindow & parent,
		int x = 0, int y = 0, int width = 800, int height = 600, int border = 20)
{
	Display *& dsp = parent.dsp;
	if(!dsp) { return 1; }

	int n = DefaultScreen(dsp);
	auto white = WhitePixel(dsp, n), black = BlackPixel(dsp, n);
	auto & win = myxwin.win = XCreateSimpleWindow(dsp, parent.win, x, y, width, height, border, black, white);
	XMapWindow(dsp, win);

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

		XFlush(dsp);
}

int do_test_xlib_main(int argc, char **argv)
{
	printf("%s\n", __FUNCTION__);
	MyXWindow myxwin;
	auto r = my_x_window_create(myxwin);
	if (r != 0) {
		printf("%s", "my_x_window_create failed!\n");
	}
	auto & dsp = myxwin.dsp;
	auto & win = myxwin.win;
	auto & gc = myxwin.gc;
	auto & evt = myxwin.evt;

	my_x_window_draw_rectangle(myxwin, 0, 0, 30, 50);
	my_x_window_draw_rectangle(myxwin, 0, 0, 234, 67);
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
	return 0;
}

/*XReparentWindow 测试通过!!*/
int do_test_xlib_main_2(int argc, char **argv)
{
	if(argc < 2){
		printf("%s: test X11 XReparentWindow\n\t%s %s %s\n", __FUNCTION__, argv[0], TEST_XLIB, "<X Window ID>");
		return 0;
	}
	MyXWindow myxwin, subwin;
	auto r = my_x_window_create(myxwin);
	r = my_x_window_create(subwin, myxwin, 0, 0, 400, 200);
	if (r != 0) {
		printf("%s", "my_x_window_create failed!\n");
	}
	auto & dsp = myxwin.dsp;
	auto & win = myxwin.win;
	auto & evt = myxwin.evt;

	my_x_window_draw_rectangle(myxwin, 0, 0, 30, 50);
	my_x_window_draw_rectangle(myxwin, 0, 0, 234, 67);


	long eventMask = ButtonPressMask | ButtonReleaseMask;
	XSelectInput(dsp, win, eventMask); // override prev
	do {
		XNextEvent(dsp, &evt);   // calls XFlush()
	} while (evt.type != ButtonRelease);
	XReparentWindow(myxwin.dsp, subwin.win, DefaultRootWindow(myxwin.dsp), 10, 10);
	do {
		XNextEvent(dsp, &evt);   // calls XFlush()
	} while (evt.type != ButtonRelease);

	Window xid = atol(argv[1]);
	printf("window to attach %u\n", xid);
	XReparentWindow(myxwin.dsp, subwin.win, xid/*myxwin.win*/, 10, 10);
	XSetWindowBorderWidth(myxwin.dsp, subwin.win, 10);
	do {
		XNextEvent(dsp, &evt);   // calls XFlush()
	} while (evt.type != ButtonRelease);
	XReparentWindow(myxwin.dsp, subwin.win, myxwin.win, 10, 10);
	XSetWindowBorderWidth(myxwin.dsp, subwin.win, 10);

	do {
		XNextEvent(dsp, &evt);   // calls XFlush()
	} while (evt.type != ButtonRelease);
	XReparentWindow(myxwin.dsp, subwin.win, DefaultRootWindow(myxwin.dsp), 10, 10);
	do {
		XNextEvent(dsp, &evt);   // calls XFlush()
	} while (evt.type != ButtonRelease);
	return 0;
}
int test_xlib_main(int argc, char **argv)
{
    cout << "hello_X11" << endl; // prints

//    xlib_draw_rectangle(0, 0, 30, 50);
//	xlib_draw_rectangle(20, 23, 214, 89);
//	sleep(300);

    /*ok*/
//	do_test_xlib_main(argc, argv);


	do_test_xlib_main_2(argc, argv);
	return 0;
}

void * do_x_event(void * arg)
{
	MyXWindow & myxwin = *(MyXWindow * )arg;
	auto & dsp = myxwin.dsp;
	auto & win = myxwin.win;
	auto & gc = myxwin.gc;
	auto & evt = myxwin.evt;

	long eventMask = KeyPressMask;
	XSelectInput(dsp, win, eventMask); // override prev

	for(int is_run = 1; is_run;){
		XNextEvent(dsp, &evt);   // calls XFlush()
		if(evt.type == KeyPress){
//			printf("keycode = %d\n", evt.xkey.keycode);
			int keycode = evt.xkey.keycode;
			if(keycode == 54 /*c*/){
				XClearWindow(dsp, win);
			}
		}
		is_run = !(evt.type == KeyPress && evt.xkey.keycode == 9 /*ESC*/);
	} ;
//	XDestroyWindow(dsp, win);
//	XCloseDisplay(dsp);

	return arg;
}
int xlib_draw_rectangle(int x, int y, int w, int h)
{
	static bool is_done = 0;
    static MyXWindow myxwin;
	if(!is_done){
		pthread_t tid;
		auto r = my_x_window_create(myxwin);
		int result = pthread_create(&tid, NULL, do_x_event, &myxwin);
		if(r != 0 || result) {
			printf("%s\n", __FUNCTION__);
			return -1;
		}
		is_done = 1;
	}
	my_x_window_draw_rectangle(myxwin, x, y, w, h);
	return 0;
}

#endif /*__CYGWIN_GCC__*/
