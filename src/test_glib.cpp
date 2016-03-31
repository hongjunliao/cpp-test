/*
 * test_glib.cpp

 *
 *  Created on: Jan 5, 2016
 *      Author: root
 */
#include "bd_test.h"
#include "test_glib.h"
#include <gtk/gtk.h>
G_DEFINE_TYPE(Student, student, G_TYPE_OBJECT)
static void student_class_init(StudentClass*)
{

}

static void student_init(Student*)
{
}
void btn_clicked(GtkWidget * obj, gpointer data){
	static int count  = 0;
	printf("data = %s, count = %d\n", (char *)data, ++count);
}
int test_gtk_main(int argc, char ** argv)
{
	gtk_init(&argc, &argv);	//required!
	GtkWidget * wnd = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(wnd, "destroy",gtk_main_quit, 0);
	GtkWidget * btn = gtk_button_new_with_label("hello");
	gtk_container_add(GTK_CONTAINER(wnd), btn);
	g_signal_connect(btn, "clicked", G_CALLBACK(btn_clicked), (void *)"hello gtk!");
	gtk_widget_show_all(wnd);
	gtk_main();
	return 0;
}

//int test_gtk_main(int argc, char **	argv){
//	GtkWidget *window;
//	gtk_init(&argc, &argv);
//
//	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
////	g_object_set(window, "hide-titlebar-when-maximized", 1);
//	gtk_window_set_decorated (GTK_WINDOW(window), FALSE);
////	gtk_window_set_hide_titlebar_when_maximized(GTK_WINDOW(window), TRUE);
//	gtk_window_maximize(GTK_WINDOW(window));
//	gtk_widget_show(window);
//
//	gtk_main();
//
//	return 0;
//}
