#include "workspace_app.h"

GtkWidget *create_workspace_view(AppData *app_data) {
    GtkWidget *main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    
    // Create menu bar
    GtkWidget *menubar = gtk_menu_bar_new();
    
    GtkWidget *file_menu = gtk_menu_new();
    GtkWidget *file_item = gtk_menu_item_new_with_label("File");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item), file_menu);
    
    GtkWidget *new_item = gtk_menu_item_new_with_label("New Workspace");
    g_signal_connect(new_item, "activate", G_CALLBACK(on_new_workspace_menu_clicked), app_data);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), new_item);
    
    GtkWidget *load_item = gtk_menu_item_new_with_label("Load Workspace");
    g_signal_connect(load_item, "activate", G_CALLBACK(on_load_workspace_clicked), app_data);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), load_item);
    
    GtkWidget *save_item = gtk_menu_item_new_with_label("Save Workspace");
    g_signal_connect(save_item, "activate", G_CALLBACK(on_save_workspace_clicked), app_data);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), save_item);
    
    GtkWidget *config_menu = gtk_menu_new();
    GtkWidget *config_item = gtk_menu_item_new_with_label("Configuration");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(config_item), config_menu);
    GtkWidget *Communication_Configuration = gtk_menu_item_new_with_label("Communication Configuration"); //TODO
    gtk_menu_shell_append(GTK_MENU_SHELL(config_menu), Communication_Configuration);

    GtkWidget *macro_menu = gtk_menu_new();
    GtkWidget *macro_item = gtk_menu_item_new_with_label("Macros");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(macro_item), macro_menu);

    GtkWidget *Create_Macro = gtk_menu_item_new_with_label("Create Macro"); //TODO
    gtk_menu_shell_append(GTK_MENU_SHELL(macro_menu), Create_Macro);

    GtkWidget *Record_Macro = gtk_menu_item_new_with_label("Record Macro"); //TODO
    gtk_menu_shell_append(GTK_MENU_SHELL(macro_menu), Record_Macro);

    GtkWidget *Load_macro = gtk_menu_item_new_with_label("Load Macro");
    g_signal_connect(Load_macro, "activate", G_CALLBACK(on_load_macro_clicked), app_data);
    gtk_menu_shell_append(GTK_MENU_SHELL(macro_menu), Load_macro);

    GtkWidget *Start_macro = gtk_menu_item_new_with_label("Start Macro");
    g_signal_connect(Start_macro, "activate", G_CALLBACK(on_start_macro_clicked), app_data);
    gtk_menu_shell_append(GTK_MENU_SHELL(macro_menu), Start_macro);
    GtkWidget *Stop_macro = gtk_menu_item_new_with_label("Stop Macro");
    g_signal_connect(Stop_macro, "activate", G_CALLBACK(on_stop_macro_clicked), app_data);
    gtk_menu_shell_append(GTK_MENU_SHELL(macro_menu), Stop_macro);

    GtkWidget *settings_menu = gtk_menu_new();
    GtkWidget *settings_item = gtk_menu_item_new_with_label("Settings");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(settings_item), settings_menu);
    GtkWidget *app_settings = gtk_menu_item_new_with_label("Application Settings"); //TODO
    gtk_menu_shell_append(GTK_MENU_SHELL(settings_menu), app_settings);

    GtkWidget *help_menu = gtk_menu_new();
    GtkWidget *help_item = gtk_menu_item_new_with_label("Help");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(help_item), help_menu);
    GtkWidget *Information = gtk_menu_item_new_with_label("Information");
    g_signal_connect(Information, "activate", G_CALLBACK(application_information), app_data);
    gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), Information);

    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), file_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), config_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), macro_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), settings_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), help_item);
    gtk_box_pack_start(GTK_BOX(main_vbox), menubar, FALSE, FALSE, 0);
      
    // Create scrolled window for workspace
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), 
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    
    // Create workspace container
    app_data->workspace_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(app_data->workspace_container), 10);
    
    gtk_container_add(GTK_CONTAINER(scrolled), app_data->workspace_container);
    gtk_box_pack_start(GTK_BOX(main_vbox), scrolled, TRUE, TRUE, 0);
    
    if (app_data->workspace_loaded) {
        create_workspace_from_definition(app_data);
    }

    return main_vbox;
}

void create_workspace_from_definition(AppData *app_data) {
    // Clear existing workspace
    GList *children = gtk_container_get_children(GTK_CONTAINER(app_data->workspace_container));
    for (GList *iter = children; iter != NULL; iter = g_list_next(iter)) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);
    
    // Create windows from definition
    for (int i = 0; i < app_data->current_workspace.window_count; i++) {
        WindowDef *window_def = &app_data->current_workspace.windows[i];
        
        GtkWidget *frame = gtk_frame_new(window_def->title);
        gtk_frame_set_label_align(GTK_FRAME(frame), 0.5, 0.5);
        
        GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
        gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
        
        // Create controls from definition
        for (int j = 0; j < window_def->control_count; j++) {
            ControlDef *control = &window_def->controls[j];
            
            if (control->type == CONTROL_BUTTON) {
                GtkWidget *button = gtk_button_new_with_label(control->name);
                char *button_data = g_strdup(control->name);
                g_signal_connect(button, "clicked", G_CALLBACK(on_button_clicked), button_data);
                gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);

                // REGISTER THE WIDGET
                register_control_widget(app_data, control->name, button);

            } else if (control->type == CONTROL_HSLIDER) {
                GtkWidget *label = gtk_label_new(control->name);
                gtk_widget_set_halign(label, GTK_ALIGN_START);
                gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
                
                GtkWidget *hscale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 
                                           control->min_val, control->max_val, 0.1); // Smaller increment
                gtk_scale_set_digits(GTK_SCALE(hscale), 2); // Show 2 decimal places
                gtk_scale_set_value_pos(GTK_SCALE(hscale), GTK_POS_TOP);
                gtk_range_set_value(GTK_RANGE(hscale), control->value);
                char *scale_data = g_strdup(control->name);
                g_signal_connect(hscale, "value-changed", G_CALLBACK(on_scale_changed), scale_data);
                gtk_box_pack_start(GTK_BOX(vbox), hscale, FALSE, FALSE, 0);

                // REGISTER THE WIDGET
                register_control_widget(app_data, control->name, hscale);

            } else if (control->type == CONTROL_VSLIDER) {
                GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
                
                GtkWidget *vscale = gtk_scale_new_with_range(GTK_ORIENTATION_VERTICAL, 
                                           control->min_val, control->max_val, 0.1); // Smaller increment
                gtk_scale_set_digits(GTK_SCALE(vscale), 2); // Show 2 decimal places
                gtk_scale_set_value_pos(GTK_SCALE(vscale), GTK_POS_RIGHT);
                gtk_range_set_value(GTK_RANGE(vscale), control->value);

                gtk_widget_set_size_request(vscale, -1, 100);
                char *scale_data = g_strdup(control->name);
                g_signal_connect(vscale, "value-changed", G_CALLBACK(on_scale_changed), scale_data);
                
                gtk_box_pack_start(GTK_BOX(hbox), vscale, FALSE, FALSE, 0);
                gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new(control->name), FALSE, FALSE, 0);
                gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

                // REGISTER THE WIDGET
                register_control_widget(app_data, control->name, vscale);

            } else if (control->type == CONTROL_SWITCH) {
                GtkWidget *switch_widget = gtk_toggle_button_new_with_label(control->name);
                        
                // Set initial state
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(switch_widget), 
                                            control->value != 0.0);
                        
                // Style the switch to look different from regular buttons
                GtkStyleContext *context = gtk_widget_get_style_context(switch_widget);
                gtk_style_context_add_class(context, "switch-control");
                        
                char *switch_data = g_strdup(control->name);
                g_signal_connect(switch_widget, "toggled", G_CALLBACK(on_switch_toggled), switch_data);
                        
                // REGISTER THE WIDGET
                register_control_widget(app_data, control->name, switch_widget);
                        
                gtk_box_pack_start(GTK_BOX(vbox), switch_widget, FALSE, FALSE, 0);
            }
        }

        gtk_container_add(GTK_CONTAINER(frame), vbox);
        gtk_box_pack_start(GTK_BOX(app_data->workspace_container), frame, TRUE, TRUE, 5);
    }
    
    //gtk_widget_show_all(app_data->workspace_view);
    // removed as it was causing errors on first workspace load and all widgets are shown with other call from workspace_app.c
}

void on_save_workspace_clicked(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;
    
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Save Workspace",
                                                    GTK_WINDOW(app_data->main_window),
                                                    GTK_FILE_CHOOSER_ACTION_SAVE,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    "_Save", GTK_RESPONSE_ACCEPT,
                                                    NULL);
    
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);
    
    // Add file filter
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Workspace Files (*.csv)");
    gtk_file_filter_add_pattern(filter, "*.csv");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (result == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        
        // Ensure .csv extension
        char *full_filename;
        if (!g_str_has_suffix(filename, ".csv")) {
            full_filename = g_strdup_printf("%s.csv", filename);
        } else {
            full_filename = g_strdup(filename);
        }
        
        save_workspace_simple(app_data, full_filename);
        g_print("Workspace saved to: %s\n", full_filename);
        
        GtkWidget *info_dialog = gtk_message_dialog_new(GTK_WINDOW(app_data->main_window),
                                                       GTK_DIALOG_DESTROY_WITH_PARENT,
                                                       GTK_MESSAGE_INFO,
                                                       GTK_BUTTONS_OK,
                                                       "Workspace saved successfully to: %s", full_filename);
        gtk_dialog_run(GTK_DIALOG(info_dialog));
        gtk_widget_destroy(info_dialog);
        
        g_free(filename);
        g_free(full_filename);
    }
    
    gtk_widget_destroy(dialog);
}

void on_new_workspace_menu_clicked(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;
    show_welcome_screen(app_data);
}

void configure_communication(void) {

}

void application_settings(void) {

}

void application_information(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;
    
    GtkWidget *dialog = gtk_message_dialog_new(
        GTK_WINDOW(app_data->main_window),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_OK,
        "Application v0.10\n\n"
        "This application is in the alpha version and currently\n"
        "contains just the front end interface.\n\n"
        "It is in development as of Q3 2025\n\n- John\n\n"
        "Built with GTK+ 3.0 for Ubuntu Linux"
    );
    
    gtk_window_set_title(GTK_WINDOW(dialog), "About");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}
