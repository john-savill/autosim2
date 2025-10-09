#ifndef WORKSPACE_APP_H
#define WORKSPACE_APP_H

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <glib/gstdio.h>

#define MAX_WINDOWS 10
#define MAX_CONTROLS 20
#define MAX_NAME_LENGTH 100

// Window definition
typedef struct {
    char title[MAX_NAME_LENGTH];
    int control_count;
} WindowDef;

// Workspace definition
typedef struct {
    char name[MAX_NAME_LENGTH];
    int window_count;
    WindowDef windows[MAX_WINDOWS];
} WorkspaceDef;

// Application data
typedef struct {
    GtkWidget *main_window;
    GtkWidget *welcome_screen;
    GtkWidget *workspace_view;
    GtkWidget *workspace_container;
    WorkspaceDef current_workspace;
    gboolean workspace_loaded;
} AppData;

void activate_application(GtkApplication *app, gpointer user_data);

// Welcome screen functions
void show_welcome_screen(AppData *app_data);
GtkWidget *create_welcome_screen(AppData *app_data);
void on_new_workspace_clicked(GtkWidget *widget, gpointer data);

// Workspace functions
GtkWidget *create_workspace_view(AppData *app_data);
void show_workspace_view(AppData *app_data);
void create_default_workspace(AppData *app_data);

#endif
