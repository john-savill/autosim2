#include "workspace_app.h"

GtkWidget *create_welcome_screen(AppData *app_data) {
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 50);
    
    // Title
    GtkWidget *title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title), "<span size='24000' weight='bold'>Low Level ECU Simulator Alpha\nV0.30\nDeveloped by John Savill</span>");
    gtk_widget_set_halign(title, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(vbox), title, FALSE, FALSE, 20);

    // Image
    GtkWidget *image;
    GdkPixbuf *pixbuf, *scaled_pixbuf;
    image = gtk_image_new_from_file("resources/autosim_logo.png");
    pixbuf = gdk_pixbuf_new_from_file("resources/autosim_logo.png", NULL);
    scaled_pixbuf = gdk_pixbuf_scale_simple(pixbuf, 200, 200, GDK_INTERP_BILINEAR);
    image = gtk_image_new_from_pixbuf(scaled_pixbuf);
    gtk_box_pack_start(GTK_BOX(vbox), image, TRUE, TRUE, 0);

    // Description
    GtkWidget *description = gtk_label_new(
        "Welcome to the Low Level ECU Simulator\n\n"
        "Create or load a new workspace\n\n"
        "Current Features:\n"
        "• Save workspace configurations to external files\n"
        "• Load previously saved workspaces\n"
        "• Front end interface with buttons, sliders, and switches\n"
        "• Macro running\n"
        "• GPIO backend configuration\n"
    );
    gtk_label_set_justify(GTK_LABEL(description), GTK_JUSTIFY_CENTER);
    gtk_widget_set_halign(description, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(vbox), description, TRUE, TRUE, 0);
    
    // Buttons
    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    gtk_widget_set_halign(button_box, GTK_ALIGN_CENTER);
    
    GtkWidget *new_button = gtk_button_new_with_label("Create New Workspace");
    gtk_widget_set_size_request(new_button, 200, 50);
    g_signal_connect(new_button, "clicked", G_CALLBACK(on_new_workspace_clicked), app_data);
    gtk_box_pack_start(GTK_BOX(button_box), new_button, FALSE, FALSE, 0);
    
    GtkWidget *load_button = gtk_button_new_with_label("Load Existing Workspace");
    gtk_widget_set_size_request(load_button, 200, 50);
    g_signal_connect(load_button, "clicked", G_CALLBACK(on_load_workspace_clicked), app_data);
    gtk_box_pack_start(GTK_BOX(button_box), load_button, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(vbox), button_box, FALSE, FALSE, 20);
    
    g_object_unref(pixbuf); // Clean up the pixbuf
    g_object_unref(scaled_pixbuf);

    return vbox;
}

void on_new_workspace_clicked(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;
    
    create_default_workspace(app_data);
    app_data->workspace_loaded = TRUE;
    show_workspace_view(app_data);
    // REMOVED: create_workspace_from_definition(app_data); - Let workspace_view handle this
}

void on_load_workspace_clicked(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;
    
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Load Workspace",
                                                    GTK_WINDOW(app_data->main_window),
                                                    GTK_FILE_CHOOSER_ACTION_OPEN,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    "_Open", GTK_RESPONSE_ACCEPT,
                                                    NULL);
    
    // Add file filter for workspace files
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Workspace Files (*.csv)");
    gtk_file_filter_add_pattern(filter, "*.csv");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (result == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        
        if (load_workspace_simple(app_data, filename)) {
            app_data->workspace_loaded = TRUE;
            show_workspace_view(app_data);
            // REMOVED: create_workspace_from_definition(app_data); - Let workspace_view handle this
            g_print("Workspace loaded successfully from: %s\n", filename);
        } else {
            GtkWidget *error_dialog = gtk_message_dialog_new(GTK_WINDOW(app_data->main_window),
                                                           GTK_DIALOG_DESTROY_WITH_PARENT,
                                                           GTK_MESSAGE_ERROR,
                                                           GTK_BUTTONS_CLOSE,
                                                           "Failed to load workspace from: %s", filename);
            gtk_dialog_run(GTK_DIALOG(error_dialog));
            gtk_widget_destroy(error_dialog);
        }
        
        g_free(filename);
    }
    
    gtk_widget_destroy(dialog);
}