#include "eol_app.h"

GtkWidget *create_workspace_view(AppData *app_data) {
    GtkWidget *main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    
    // Create menu bar
    GtkWidget *menubar = gtk_menu_bar_new();
    
    GtkWidget *file_menu = gtk_menu_new();
    GtkWidget *file_item = gtk_menu_item_new_with_label("File");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item), file_menu);
    
    GtkWidget *new_item = gtk_menu_item_new_with_label("New Workspace");
    //g_signal_connect(new_item, "activate", G_CALLBACK(on_new_workspace_menu_clicked), app_data);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), new_item);
    
    GtkWidget *load_item = gtk_menu_item_new_with_label("Load Workspace");
    //g_signal_connect(load_item, "activate", G_CALLBACK(on_load_workspace_clicked), app_data);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), load_item);
    
    GtkWidget *save_item = gtk_menu_item_new_with_label("Save Workspace");
    //g_signal_connect(save_item, "activate", G_CALLBACK(on_save_workspace_clicked), app_data);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), save_item);
    
    GtkWidget *config_menu = gtk_menu_new();
    GtkWidget *config_item = gtk_menu_item_new_with_label("Configuration");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(config_item), config_menu);
    GtkWidget *Communication_Configuration = gtk_menu_item_new_with_label("Communication Configuration");
    //g_signal_connect(Communication_Configuration, "activate", G_CALLBACK(on_gpio_mapping_clicked), app_data);
    gtk_menu_shell_append(GTK_MENU_SHELL(config_menu), Communication_Configuration);

    GtkWidget *settings_menu = gtk_menu_new();
    GtkWidget *settings_item = gtk_menu_item_new_with_label("Settings");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(settings_item), settings_menu);
    GtkWidget *app_settings = gtk_menu_item_new_with_label("Application Settings"); //TODO
    gtk_menu_shell_append(GTK_MENU_SHELL(settings_menu), app_settings);

    GtkWidget *help_menu = gtk_menu_new();
    GtkWidget *help_item = gtk_menu_item_new_with_label("Help");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(help_item), help_menu);
    GtkWidget *Information = gtk_menu_item_new_with_label("Information");
    //g_signal_connect(Information, "activate", G_CALLBACK(application_information), app_data);
    gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), Information);

    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), file_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), config_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), settings_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), help_item);
    gtk_box_pack_start(GTK_BOX(main_vbox), menubar, FALSE, FALSE, 0);
      
    //// Create scrolled window for workspace
    //GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    //gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), 
    //                               GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    
    //// Create workspace container
    //app_data->workspace_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    //gtk_container_set_border_width(GTK_CONTAINER(app_data->workspace_container), 10);
    
    //gtk_container_add(GTK_CONTAINER(scrolled), app_data->workspace_container);
    //gtk_box_pack_start(GTK_BOX(main_vbox), scrolled, TRUE, TRUE, 0);
    
    //if (app_data->workspace_loaded) {
    //    create_workspace_from_definition(app_data);
    //}

    return main_vbox;
}

void create_default_workspace(AppData *app_data) {
    
}