#include "workspace_app.h"

void activate_application(GtkApplication *app, gpointer user_data) {
    AppData *app_data = (AppData *)user_data;
    
    app_data->main_window = gtk_application_window_new(app);
    gtk_window_set_default_size(GTK_WINDOW(app_data->main_window), 900, 700);
    /*
    GdkPixbuf *icon;
    icon = gdk_pixbuf_new_from_file("resources/autosim_logo.png", NULL);
    gtk_window_set_icon (GTK_WINDOW(app_data->main_window), icon);

    or 

    gtk_window_set_default_icon(icon);
    */
    gtk_window_set_position(GTK_WINDOW(app_data->main_window), GTK_WIN_POS_CENTER);
    
    app_data->workspace_loaded = FALSE;
    app_data->welcome_screen = NULL;
    app_data->workspace_view = NULL;

    init_macro_system(app_data);
    
    show_welcome_screen(app_data);
}

void show_welcome_screen(AppData *app_data) {
    if (app_data->workspace_view) {
        gtk_container_remove(GTK_CONTAINER(app_data->main_window), app_data->workspace_view);
        app_data->workspace_view = NULL;
    }
    
    if (app_data->welcome_screen) {
        gtk_container_remove(GTK_CONTAINER(app_data->main_window), app_data->welcome_screen);
    }
    
    app_data->welcome_screen = create_welcome_screen(app_data);
    gtk_container_add(GTK_CONTAINER(app_data->main_window), app_data->welcome_screen);
    
    gtk_widget_show_all(app_data->main_window);
    gtk_window_set_title(GTK_WINDOW(app_data->main_window), "Autosim 2 alpha - Welcome");
}

void show_workspace_view(AppData *app_data) {
    if (app_data->welcome_screen) {
        gtk_container_remove(GTK_CONTAINER(app_data->main_window), app_data->welcome_screen);
        app_data->welcome_screen = NULL;
    }
    
    if (app_data->workspace_view) {
        gtk_container_remove(GTK_CONTAINER(app_data->main_window), app_data->workspace_view);
    }
    
    app_data->workspace_view = create_workspace_view(app_data);
    gtk_container_add(GTK_CONTAINER(app_data->main_window), app_data->workspace_view);
    
    gtk_widget_show_all(app_data->main_window);
    
    char title[200];
    snprintf(title, sizeof(title), "Autosim 2 - %s", app_data->current_workspace.name);
    gtk_window_set_title(GTK_WINDOW(app_data->main_window), title);
}
