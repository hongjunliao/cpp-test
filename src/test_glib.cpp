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
static GMainLoop *loop;
static int timeout_source_id = 0;
static gboolean timeout_func   (gpointer user_data);
static int test_glib_timeout_main(int argc, char ** argv);

static void student_class_init(StudentClass*)
{

}

static void student_init(Student*)
{
}
static void btn_clicked(GtkWidget * obj, gpointer data){
	static int count  = 0;
	printf("data = %s, count = %d\n", (char *)data, ++count);
}


static gboolean timeout_func   (gpointer user_data)
{
	static int count = 5;
	int argc = *(int *)user_data;
	printf("%s/%s: argc = %d, left_count = %d\n", __FILE__, __FUNCTION__, argc, count);
	if(count <= 0){
		printf("%s/%s: return FALSE, timer stop\n", __FILE__, __FUNCTION__);
		return FALSE;
	}
	--count;
	return TRUE;
}
static int test_glib_timeout_main(int argc, char ** argv)
{
	gtk_init(&argc, &argv);	//required!
	g_timeout_add(1000, timeout_func, (void *)&argc);
    loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);
	g_main_loop_quit(loop);
	return 0;
}
