/*
 * test_glib.cpp

 *
 *  Created on: Jan 5, 2016
 *      Author: root
 */
#ifdef TEST_GTK
#include "bd_test.h"
#include "test_glib.h"
#include <gtk/gtk.h>
#include "gtk_supper_container.h"
#include <gdk/gdkx.h>	/*GDK_WINDOW_XID(win)*/
/* Pixmap for scribble area, to store current scribbles */
static cairo_surface_t *surface = NULL;
/* Draw a rectangle on the screen */
static void draw_brush (GtkWidget *widget,
            gdouble    x,
            gdouble    y);

static void btn_clicked(GtkWidget * obj, gpointer data){
	static int count  = 0;
	printf("data = %s, count = %d\n", (char *)data, ++count);
}

static int test_gtk_main_3(int argc, char ** argv);

int test_gtk_main_2(int argc, char ** argv)
{
	gtk_init(&argc, &argv);	//required!
	GtkWidget * window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (window),
	                                   GTK_DIALOG_DESTROY_WITH_PARENT,
	                                   GTK_MESSAGE_INFO,
	                                   GTK_BUTTONS_CLOSE,
	                                   "You activated action: \"%s\" of type \"%s\"",
	                                    "name", "test");


	  /* Close dialog on user response */
	g_signal_connect (dialog,
	                    "response",
	                    G_CALLBACK (gtk_widget_destroy),
	                    NULL);

	g_signal_connect(window, "destroy",gtk_main_quit, 0);

	gtk_widget_show_all(window);

	gtk_widget_show (dialog);
	gtk_main();
	return 0;
}

int test_gtk_main(int argc, char ** argv)
{
//	return test_glib_timeout_main(argc, argv);

//	return test_gtk_main_2(argc, argv);

//	return test_gtk_main_3(argc, argv);

	gtk_init(&argc, &argv);	//required!
	GtkWidget * wnd = gtk_window_new(GTK_WINDOW_TOPLEVEL);

//	GtkWidgetClass * gtkwidget_class = GTK_WIDGET_CLASS(wnd);

	g_signal_connect(wnd, "destroy",gtk_main_quit, 0);
	GtkWidget * btn = gtk_button_new_with_label("hello");
//	gtk_container_add(GTK_CONTAINER(wnd), btn);
	g_signal_connect(btn, "clicked", G_CALLBACK(btn_clicked), (void *)"hello gtk!");
	gtk_widget_show_all(wnd);
//	draw_brush(wnd, 5, 5);
	GdkWindow * dwnd = gtk_widget_get_window(wnd);
	if(!dwnd){
		printf("dwnd NULL\n");
	}
	else{
		Window xid = GDK_WINDOW_XID(dwnd);
		printf("%u\n", xid);

	}
	gtk_main();
	return 0;
}

/* Draw a rectangle on the screen */
static void draw_brush (GtkWidget *widget,
            gdouble    x,
            gdouble    y)
{
  GdkRectangle update_rect;
  cairo_t *cr;

  update_rect.x = x - 3;
  update_rect.y = y - 3;
  update_rect.width = 6;
  update_rect.height = 6;

  /* Paint to the surface, where we store our state */
  cr = cairo_create (surface);

  gdk_cairo_rectangle (cr, &update_rect);
  cairo_fill (cr);

  cairo_destroy (cr);

  /* Now invalidate the affected region of the drawing area. */
  gdk_window_invalidate_rect (gtk_widget_get_window (widget),
                              &update_rect,
                              FALSE);
}

/*Gtk椤剁骇绐楀彛宓屽叆鎶�鏈�------glade鎶�鏈垎鏋愪箣涓�*/
static int test_gtk_main_3(int argc, char ** argv)
{
	GtkWidget *window, *embedded_window;
	GtkWidget *supper;
	GtkWidget *label_embedded;
	GtkWidget *label_embedder;
	GtkWidget *vbox;

	gtk_init(&argc, &argv);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_set_size_request(window, 400, 300);

	vbox = gtk_vbox_new(FALSE, 2);
	gtk_widget_show(vbox);

	label_embedder = gtk_label_new("I'am a label in embedder.");
	gtk_widget_show(label_embedder);

	gtk_box_pack_start(GTK_BOX(vbox), label_embedder, FALSE, FALSE, 5);

	supper = supper_container_new();
	gtk_widget_show(supper);

	gtk_box_pack_start(GTK_BOX(vbox), supper, TRUE, TRUE, 5);

	gtk_container_add(GTK_CONTAINER(window), vbox);

	gtk_widget_show(window);

	embedded_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	label_embedded = gtk_label_new("I'am a label in embedded.");
	gtk_widget_show(label_embedded);
	gtk_container_add(GTK_CONTAINER(embedded_window), label_embedded);

	gtk_container_add(GTK_CONTAINER(supper), embedded_window);

	gtk_widget_show_all(window);

	gtk_main();

	return 0;
}
#endif /*TEST_GTK*/
