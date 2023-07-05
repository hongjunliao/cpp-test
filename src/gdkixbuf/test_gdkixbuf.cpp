#ifdef CPP_TEST_WITH_GDKIXBUF

#ifdef __CYGWIN_GCC__
#include "bd_test.h"
#include <gtk/gtk.h>
#include <SDL/SDL.h>

void set_pixel_value(GdkPixbuf *pixbuf, int x, int y, int color) {
	guchar *pixel;
	guchar *p;
	gint channel;
	gint rowstride;

	if (!pixbuf)
		return;

	channel = gdk_pixbuf_get_n_channels(pixbuf);
	rowstride = gdk_pixbuf_get_rowstride(pixbuf);
	pixel = gdk_pixbuf_get_pixels(pixbuf);
	p = pixel + y * rowstride + x * channel;

	p[0] = (color & 0xFF000000) >> 24;
	p[1] = (color & 0x00FF0000) >> 16;
	p[2] = (color & 0x0000FF00) >> 8;
}

//output a rectangel to pixbuf
void PutRect(GdkPixbuf*pixbuf, SDL_Rect rect, gint color) {
	gint i;
	gint j;
	gint x;
	gint y;
	gint w;
	gint h;

	if (pixbuf == NULL)
		return;
	w = gdk_pixbuf_get_width(pixbuf);
	h = gdk_pixbuf_get_height(pixbuf);

// draw top, buttom line
	for (i = 0; i < 2; ++i) {
		if (i == 0)
			y = rect.y;
		else
			y += rect.h;

		x = rect.x;
		for (j = 0; j < rect.w; ++j) {
			if (x + j < w && y < h)
				set_pixel_value(pixbuf, x + j, y, color);
		}
	}

// draw left, right line
	for (i = 0; i < 2; ++i) {
		if (i == 0)
			x = rect.x;
		else
			x += rect.w;

		y = rect.y;
		for (j = 0; j < rect.h; ++j) {
			if (x < w && y + j < h)
				set_pixel_value(pixbuf, x, y + j, color);
		}
	}
}

int test_gdkpixbuf_main(int argc, char *argv[]) {
	GtkWidget *win = NULL;
	GtkWidget *vbox = NULL;
	GtkWidget *image;
	GdkPixbuf *pixbuf;
	SDL_Rect rect;

	/* Initialize GTK+ */
	gtk_init(&argc, &argv);

	/* Create the main window */
	win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(win), 8);
	gtk_window_set_default_size(GTK_WINDOW(win), 800, 600);
	gtk_window_set_title(GTK_WINDOW(win), "Test draw rectangle to pixbuf");
	gtk_window_set_position(GTK_WINDOW(win), GTK_WIN_POS_CENTER);
	g_signal_connect(win, "destroy", gtk_main_quit, NULL);

	vbox = gtk_vbox_new(FALSE, 50);
	gtk_container_add(GTK_CONTAINER(win), vbox);

	pixbuf = gdk_pixbuf_new_from_file("christmas.jpg", NULL);

	rect.x = 20;
	rect.y = 20;
	rect.w = 70;
	rect.h = 30;

	//draw a yellow rectangle to pixbuf
	PutRect(pixbuf, rect, 0xFFFF0000);
	image = gtk_image_new_from_pixbuf(pixbuf);
	g_object_unref(pixbuf);
	gtk_box_pack_start(GTK_BOX(vbox), image, TRUE, TRUE, 10);

	gtk_widget_show_all(win);

	gtk_main();
	return 0;
}
#endif /*__CYGWIN_GCC__*/

#endif

