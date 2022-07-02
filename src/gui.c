#include <stdio.h>
#include <GL/gl.h>
#include <gtk/gtk.h>
#include "gui.h"
#include "util.h"

// struct that holds relevant data for gtk signals
struct signal
{
	const gchar *signal;
	GCallback handler;
	GdkEventMask mask;
};

static void connect_signals(GtkWidget *widget, struct signal *signals, size_t members)
{
	FOREACH_NELEM(signals, members, s)
	{
		gtk_widget_add_events(widget, s->mask);
		g_signal_connect(widget, s->signal, s->handler, NULL);
	}
}

static void connect_window_signals(GtkWidget *window)
{
	struct signal signals[] = {{"destroy", G_CALLBACK(gtk_main_quit), 0}};
	connect_signals(window, signals, NELEM(signals));
}

bool gui_init(int *argc, char ***argv)
{
	// init gtk
	if (!gtk_init_check(argc, argv))
	{
		fputs("Could not initialize GTK", stderr);	
		return false;
	}
	return true;
}

bool gui_run(void)
{
	// create the toplevel window, add in GtkGLArea
	GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	GtkWidget *glarea = gtk_gl_area_new();
	gtk_container_add(GTK_CONTAINER(window), glarea);

	// connect gtk signals
	connect_window_signals(window);	
	
	gtk_widget_show_all(window);

	// enter gtk event loop
	gtk_main();

	return true;

}
