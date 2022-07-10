#include <stdbool.h>
#include <stdio.h>
#include <GL/gl.h>
#include <gtk/gtk.h>

#include "background.h"
#include "program.h"
#include "util.h"

// struct that holds relevant data for gtk signals
struct signal
{
	const gchar *signal;
	GCallback handler;
	GdkEventMask mask;
};

static void on_resize(GtkGLArea *area, gint width, gint height)
{
	// TODO: write
	//view_set_window(width, height);
	background_set_window(width, height);
}

static void on_realize(GtkGLArea *glarea)
{
	gtk_gl_area_make_current(glarea); /* associates / verifies association of GdkGLContect w/ glarea */
	
	// print out version info
	const GLubyte* renderer = glGetString(GL_RENDERER); 
	const GLubyte* version = glGetString(GL_VERSION);
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported: %s\n", version);

	// enable depth buffer, for perspective rendering
	gtk_gl_area_set_has_depth_buffer(glarea, TRUE);

	programs_init(); /* only inits the background program so far */

	background_init();

	// Get frame clock
	GdkGLContext *glcontext = gtk_gl_area_get_context(glarea);
	GdkWindow *glwindow = gdk_gl_context_get_window(glcontext);
	GdkFrameClock *frame_clock = gdk_window_get_frame_clock(glwindow);

	// connect update signal
	g_signal_connect_swapped
			(frame_clock,
			 "update",
			 G_CALLBACK(gtk_gl_area_queue_render),
			 glarea
			);

	// start updating the clock
	gdk_frame_clock_begin_updating(frame_clock);
}

static gboolean on_render(GtkGLArea *glarea, GdkGLContext *context)
{
	// clear canvas
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// draw background
	background_draw();

	// don't progagate signal (???)
	return TRUE;
}

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

static void connect_glarea_signals(GtkWidget *glarea)
{
	// TODO: add more signals as needed
	struct signal signals[] = {
		{ "realize", G_CALLBACK(on_realize), 0},
		{ "render",  G_CALLBACK(on_render),  0},
		{ "resize",  G_CALLBACK(on_resize),  0}
	};
	
	connect_signals(glarea, signals, NELEM(signals));
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
	connect_glarea_signals(glarea);
	
	gtk_widget_show_all(window);

	// enter gtk event loop
	gtk_main();

	return true;

}

