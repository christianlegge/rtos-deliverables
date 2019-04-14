/*
 * Copyright 2018 Jonathan Anderson
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>

#include "ui.h"
#include "rusage.h"


/**
 * Show an informational dialog, right now, from the GUI thread.
 *
 * @warning This will fail spectacularly if called from a thread other than the
 *          Gtk GUI thread!
 */
static void	dialog_show(const char *title, const char *text,
	const char *data);

static gint	dialog_queue_check(gpointer);

// GTK+ callbacks for GUI events (e.g., button clicks):
static void	on_click_bye(GtkWidget *, gpointer);
static void	on_click_print(GtkWidget *, gpointer);
static void	on_click_run_cc(GtkWidget *, gpointer);
static void	on_click_run_grep_large(GtkWidget *, gpointer);
static void	on_click_run_grep_small(GtkWidget *, gpointer);
static void	on_confirm_exit(GtkDialog *, gint, gpointer);
static void	on_destroy(GtkWidget *, gpointer);


struct DialogInfo {
	char *title;
	char *text;
	char *data;
};


GAsyncQueue	*dialog_queue;
GtkWidget	*main_window;


bool
ui_init(int *argcp, char **argvp[])
{
	gtk_init(argcp, argvp);

	// Create an async queue to receive dialog requests from non-GUI
	// threads and a timer to service this queue:
	dialog_queue = g_async_queue_new();
	if (dialog_queue == NULL)
	{
		return false;
	}

	g_timeout_add(100, dialog_queue_check, NULL);

	return true;
}


bool
ui_show(const char *title, const char *text, const char *data)
{
	assert(dialog_queue);

	struct DialogInfo *info = malloc(sizeof(*info));
	if (info == NULL)
	{
		return false;
	}

	info->title = strdup(title);
	if (info->title == NULL)
	{
		goto errwithinfo;
	}

	info->text = strdup(text);
	if (info->text == NULL)
	{
		goto errwithtitle;
	}

	info->data = strdup(data);
	if (info->data == NULL)
	{
		goto errwithtext;
	}

	g_async_queue_push(dialog_queue, info);

	return true;

errwithtext:
	free(info->text);

errwithtitle:
	free(info->title);

errwithinfo:
	free(info);

	return false;
}


void*
ui_run(void *argp)
{
	// Create top-level window that terminates the whole process on close.
	main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(main_window, "destroy", G_CALLBACK(on_destroy), NULL);
	gtk_container_set_border_width(GTK_CONTAINER(main_window), 10);

	GtkWidget *box = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(main_window), box);

	GtkWidget *print = gtk_button_new_with_label("Print rusage");
	g_signal_connect(print, "clicked", G_CALLBACK(on_click_print),
	                 main_window);
	gtk_box_pack_start(GTK_BOX(box), print, TRUE, TRUE, 0);
	gtk_widget_show(print);

	GtkWidget *run = gtk_button_new_with_label("Small grep");
	g_signal_connect(run, "clicked", G_CALLBACK(on_click_run_grep_small),
	                 main_window);
	gtk_box_pack_start(GTK_BOX(box), run, TRUE, TRUE, 0);
	gtk_widget_show(run);

	run = gtk_button_new_with_label("Big grep");
	g_signal_connect(run, "clicked", G_CALLBACK(on_click_run_grep_large),
	                 main_window);
	gtk_box_pack_start(GTK_BOX(box), run, TRUE, TRUE, 0);
	gtk_widget_show(run);

	run = gtk_button_new_with_label("Run cc");
	g_signal_connect(run, "clicked", G_CALLBACK(on_click_run_cc),
	                 main_window);
	gtk_box_pack_start(GTK_BOX(box), run, TRUE, TRUE, 0);
	gtk_widget_show(run);

	GtkWidget *bye = gtk_button_new_with_label("Goodbye");
	g_signal_connect(bye, "clicked", G_CALLBACK(on_click_bye), main_window);
	gtk_box_pack_start(GTK_BOX(box), bye, TRUE, TRUE, 0);
	gtk_widget_show(bye);

	gtk_widget_show(box);
	gtk_widget_show(main_window);
	gtk_main();

	return NULL;
}


static gint
dialog_queue_check(gpointer data)
{
	gpointer p = g_async_queue_try_pop(dialog_queue);
	if (p == NULL)
	{
		return 1;
	}

	struct DialogInfo *info = (struct DialogInfo*) p;
	dialog_show(info->title, info->text, info->data);

	return 1;
}


static void
dialog_show(const char *title, const char *text, const char *data)
{
	assert(main_window);

	GtkWidget *dialog = gtk_dialog_new_with_buttons(title,
		GTK_WINDOW(main_window),
		GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
		NULL);

	gtk_container_border_width(GTK_CONTAINER(dialog), 4);
	gtk_signal_connect(GTK_OBJECT(dialog), "response",
		G_CALLBACK(gtk_widget_destroy), dialog);

	// Add normal-looking text:
	GtkWidget *label = gtk_label_new(text);
	gtk_misc_set_padding(GTK_MISC(label), 12, 12);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		label, TRUE, TRUE, 0);

	// Add monospaced, selectable/copyable text:
	label = gtk_label_new(data);
	gtk_label_set_selectable(GTK_LABEL(label), TRUE);
	gtk_misc_set_padding(GTK_MISC(label), 12, 12);

	PangoAttribute *attr = pango_attr_font_desc_new(
		pango_font_description_from_string("monospace"));
	PangoAttrList *attrs = pango_attr_list_new();
	pango_attr_list_insert(attrs, attr);
	gtk_label_set_attributes(GTK_LABEL(label), attrs);

	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		label, TRUE, TRUE, 0);

	// Finally, show the dialog:
	gtk_widget_show_all(dialog);
}


static void
on_click_print(GtkWidget *widget, gpointer data)
{
	assert(main_window);

	char buffer[1024];
	get_rusage_string(buffer, sizeof(buffer));

	dialog_show("Resource usage", "User clicked \"Print rusage\":",
		buffer);
}

static void
on_click_bye(GtkWidget *widget, gpointer data)
{
	assert(main_window);

	// Create modal Cancel/Quit dialog
	GtkWidget *dialog = gtk_dialog_new_with_buttons("Confirm exit",
		GTK_WINDOW(main_window),
		GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
		GTK_STOCK_QUIT, GTK_RESPONSE_ACCEPT,
		NULL);

	gtk_container_border_width(GTK_CONTAINER(dialog), 4);
	gtk_signal_connect(GTK_OBJECT(dialog), "response",
		GTK_SIGNAL_FUNC(on_confirm_exit), NULL);

	// Add a label for clarity's sake
	GtkWidget *label = gtk_label_new("Are you sure you want to exit?");
	gtk_misc_set_padding(GTK_MISC(label), 12, 12);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		label, TRUE, TRUE, 0);

	gtk_widget_show_all(dialog);
}

static void
on_click_run_cc(GtkWidget *widget, gpointer data)
{
    char **argv = malloc(sizeof(char*)*3);
    argv[0] = strdup("cc");
    argv[1] = strdup("hello.c");
    argv[2] = NULL;
    char *usage = malloc(sizeof(char)*200);
    get_command_rusage(2, argv, usage, 200);
    dialog_show("Big grep rusage", "User clicked cc, rusage:\n", usage);
}

static void
on_click_run_grep_large(GtkWidget *widget, gpointer data)
{
    char **argv = malloc(sizeof(char*)*4);
    argv[0] = strdup("grep");
    argv[1] = strdup("[A-Z486]+");
    argv[2] = strdup("random.txt");
    argv[3] = NULL;
    char *usage = malloc(sizeof(char)*200);
    get_command_rusage(3, argv, usage, 200);
    dialog_show("Big grep rusage", "User clicked big grep, rusage:\n", usage);
}

static void
on_click_run_grep_small(GtkWidget *widget, gpointer data)
{
	char **argv = malloc(sizeof(char*)*4);
    argv[0] = strdup("grep");
    argv[1] = strdup("pattern");
    argv[2] = strdup("CMakeLists.txt");
    argv[3] = NULL;
    char *usage = malloc(sizeof(char)*200);
    get_command_rusage(3, argv, usage, 200);
    dialog_show("Small grep rusage", "User clicked small grep, rusage:\n", usage);
}

static void
on_confirm_exit(GtkDialog *dialog, gint response_id, gpointer data)
{
	if (response_id == GTK_RESPONSE_ACCEPT)
	{
		gtk_main_quit();
	}

	gtk_widget_destroy((GtkWidget*) dialog);
}

static void
on_destroy(GtkWidget *widget, gpointer data)
{
	gtk_main_quit();
}
