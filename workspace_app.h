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

// Control types
typedef enum {
    CONTROL_BUTTON,
    CONTROL_HSLIDER,
    CONTROL_VSLIDER
} ControlType;

// Control definition
typedef struct {
    ControlType type;
    char name[MAX_NAME_LENGTH];
    double value;  // For sliders
    double min_val, max_val;  // For sliders
} ControlDef;

// Window definition
typedef struct {
    char title[MAX_NAME_LENGTH];
    int control_count;
    ControlDef controls[MAX_CONTROLS];
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

// Function declarations
void activate_application(GtkApplication *app, gpointer user_data);
void show_welcome_screen(AppData *app_data);
void show_workspace_view(AppData *app_data);

// Welcome screen functions
GtkWidget *create_welcome_screen(AppData *app_data);
void on_new_workspace_clicked(GtkWidget *widget, gpointer data);
void on_load_workspace_clicked(GtkWidget *widget, gpointer data);

// Workspace functions
GtkWidget *create_workspace_view(AppData *app_data);
void create_workspace_from_definition(AppData *app_data);
void on_save_workspace_clicked(GtkWidget *widget, gpointer data);
void on_new_workspace_menu_clicked(GtkWidget *widget, gpointer data);

// Other Menu bar functions
void configure_communication(void);
void application_settings(void);
void application_information(GtkWidget *widget, gpointer data);

// Control callbacks
void on_button_clicked(GtkWidget *widget, gpointer data);
void on_scale_changed(GtkRange *range, gpointer data);

// File operations
void save_workspace_simple(AppData *app_data, const char *filename);
gboolean load_workspace_simple(AppData *app_data, const char *filename);
void create_default_workspace(AppData *app_data);

#endif
