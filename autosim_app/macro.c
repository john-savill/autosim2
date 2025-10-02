#include "workspace_app.h"

void init_macro_system(AppData *app_data) {
    // Initialize macro system
    memset(&app_data->current_macro, 0, sizeof(MacroDef));
    app_data->macro_timer_id = 0;
    
    // Create hash table to store control widget references
    app_data->control_widgets = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
}

void cleanup_macro_system(AppData *app_data) {
    // Stop any running macro
    stop_macro_execution(app_data);
    
    // Clean up hash table
    if (app_data->control_widgets) {
        g_hash_table_destroy(app_data->control_widgets);
        app_data->control_widgets = NULL;
    }
}

void register_control_widget(AppData *app_data, const char *name, GtkWidget *widget) {
    if (app_data->control_widgets && name && widget) {
        g_hash_table_insert(app_data->control_widgets, g_strdup(name), widget);
        g_print("Registered control widget: %s\n", name);
    }
}

gboolean load_macro_from_csv(AppData *app_data, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        g_print("Error: Could not open macro file: %s\n", filename);
        return FALSE;
    }
    
    char line[512];
    int line_number = 0;
    
    // Initialize macro
    memset(&app_data->current_macro, 0, sizeof(MacroDef));
    
    // Skip header line
    if (fgets(line, sizeof(line), file)) {
        line_number++;
    }
    
    while (fgets(line, sizeof(line), file) && app_data->current_macro.action_count < 100) {
        line_number++;
        
        // Remove newline
        line[strcspn(line, "\n")] = 0;
        
        // Skip empty lines
        if (strlen(line) == 0) {
            continue;
        }
        
        // Parse CSV line: MacroName,ActionType,TargetName,Value,DelayMS
        char macro_name[MAX_NAME_LENGTH];
        char action_type[64];
        char target_name[MAX_NAME_LENGTH];
        char value_str[64];
        char delay_str[64];
        
        // Simple CSV parsing
        if (sscanf(line, "%99[^,],%63[^,],%99[^,],%63[^,],%63s", 
                   macro_name, action_type, target_name, value_str, delay_str) >= 4) {
            
            // Set macro name if not set yet
            if (strlen(app_data->current_macro.name) == 0) {
                strcpy(app_data->current_macro.name, macro_name);
            }
            
            MacroAction *action = &app_data->current_macro.actions[app_data->current_macro.action_count];
            
            // Parse action type
            if (strcmp(action_type, "BUTTON_CLICK") == 0) {
                action->type = MACRO_BUTTON_CLICK;
            } else if (strcmp(action_type, "SLIDER_SET") == 0) {
                action->type = MACRO_SLIDER_SET;
            } else if (strcmp(action_type, "WAIT") == 0) {
                action->type = MACRO_WAIT;
            } else if (strcmp(action_type, "SWITCH_ON") == 0) {
                action->type = MACRO_SWITCH_ON;
            } else if (strcmp(action_type, "SWITCH_OFF") == 0) {
                action->type = MACRO_SWITCH_OFF;
            } else {
                g_print("Warning: Unknown action type '%s' on line %d\n", action_type, line_number);
                continue;
            }
            
            // Copy target name
            strcpy(action->target_name, target_name);
            
            // Parse value
            if (strlen(value_str) > 0 && strcmp(value_str, "-") != 0) {
                action->value = atof(value_str);
            } else {
                action->value = 0.0;
            }
            
            // Parse delay
            if (strlen(delay_str) > 0 && strcmp(delay_str, "-") != 0) {
                action->delay_ms = atoi(delay_str);
            } else {
                action->delay_ms = 100; // Default 100ms delay
            }
            
            app_data->current_macro.action_count++;
            g_print("Loaded action %d: %s %s\n", app_data->current_macro.action_count, action_type, target_name);
        }
    }
    
    fclose(file);
    
    g_print("Loaded macro '%s' with %d actions\n", 
            app_data->current_macro.name, app_data->current_macro.action_count);
    
    return app_data->current_macro.action_count > 0;
}

void start_macro_execution(AppData *app_data) {
    if (app_data->current_macro.action_count == 0) {
        g_print("No macro loaded to execute\n");
        return;
    }
    
    if (app_data->current_macro.is_running) {
        g_print("Macro is already running\n");
        return;
    }
    
    g_print("Starting macro execution: %s\n", app_data->current_macro.name);
    app_data->current_macro.is_running = TRUE;
    app_data->current_macro.current_action = 0;
    
    // Start immediate execution
    execute_next_macro_action(app_data);
}

void stop_macro_execution(AppData *app_data) {
    if (app_data->macro_timer_id > 0) {
        g_source_remove(app_data->macro_timer_id);
        app_data->macro_timer_id = 0;
    }
    
    app_data->current_macro.is_running = FALSE;
    app_data->current_macro.current_action = 0;
    g_print("Macro execution stopped\n");
}

gboolean execute_next_macro_action(gpointer data) {
    AppData *app_data = (AppData *)data;
    
    // Check if macro should continue
    if (!app_data->current_macro.is_running || 
        app_data->current_macro.current_action >= app_data->current_macro.action_count) {
        
        g_print("Macro execution completed\n");
        app_data->current_macro.is_running = FALSE;
        app_data->macro_timer_id = 0;
        return G_SOURCE_REMOVE;
    }
    
    MacroAction *action = &app_data->current_macro.actions[app_data->current_macro.current_action];
    
    g_print("Executing action %d: ", app_data->current_macro.current_action + 1);
    
    switch (action->type) {
        case MACRO_BUTTON_CLICK: {
            g_print("Clicking button '%s'\n", action->target_name);
            
            // Find the button widget
            GtkWidget *button = g_hash_table_lookup(app_data->control_widgets, action->target_name);
            if (button && GTK_IS_BUTTON(button)) {
                // Simulate button click
                // In a real implementation, this would interface with hardware/system
                g_signal_emit_by_name(button, "clicked");
                g_print("Button '%s' clicked successfully\n", action->target_name);
            } else {
                g_print("Warning: Button '%s' not found\n", action->target_name);
            }
            break;
        }
        
        case MACRO_SLIDER_SET: {
            g_print("Setting slider '%s' to %.2f\n", action->target_name, action->value);
            
            // Find the slider widget
            GtkWidget *slider = g_hash_table_lookup(app_data->control_widgets, action->target_name);
            if (slider && GTK_IS_RANGE(slider)) {
                // Set slider value
                // In a real implementation, this would interface with hardware/system
                gtk_range_set_value(GTK_RANGE(slider), action->value);
                g_print("Slider '%s' set to %.2f successfully\n", action->target_name, action->value);
            } else {
                g_print("Warning: Slider '%s' not found\n", action->target_name);
            }
            break;
        }

        case MACRO_SWITCH_ON: {
            g_print("Turning switch '%s' ON\n", action->target_name);

            GtkWidget *switch_widget = g_hash_table_lookup(app_data->control_widgets, action->target_name);
            if (switch_widget && GTK_IS_TOGGLE_BUTTON(switch_widget)) {
                // In a real implementation, this would interface with hardware/system
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(switch_widget), TRUE);
                g_print("Switch '%s' turned ON successfully\n", action->target_name);
            } else {
                g_print("Warning: Switch '%s' not found\n", action->target_name);
            }
            break;
        }

        case MACRO_SWITCH_OFF: {
            g_print("Turning switch '%s' OFF\n", action->target_name);

            GtkWidget *switch_widget = g_hash_table_lookup(app_data->control_widgets, action->target_name);
            if (switch_widget && GTK_IS_TOGGLE_BUTTON(switch_widget)) {
                // In a real implementation, this would interface with hardware/system
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(switch_widget), FALSE);
                g_print("Switch '%s' turned OFF successfully\n", action->target_name);
            } else {
                g_print("Warning: Switch '%s' not found\n", action->target_name);
            }
            break;
        }
        
        case MACRO_WAIT: {
            g_print("Waiting %.0f ms\n", action->value);
            // Wait action - the delay will be handled by the timer
            break;
        }
    }
    
    // Move to next action
    app_data->current_macro.current_action++;
    
    // Schedule next action with delay
    int next_delay = action->delay_ms;
    if (action->type == MACRO_WAIT) {
        next_delay += (int)action->value; // Add wait time to delay
    }
    
    app_data->macro_timer_id = g_timeout_add(next_delay, execute_next_macro_action, app_data);
    
    return G_SOURCE_REMOVE; // Don't repeat this timer
}

// UI Callback functions
void on_load_macro_clicked(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;
    
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Load Macro",
                                                    GTK_WINDOW(app_data->main_window),
                                                    GTK_FILE_CHOOSER_ACTION_OPEN,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    "_Open", GTK_RESPONSE_ACCEPT,
                                                    NULL);
    
    // Add file filter for macro files
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Macro Files (*.csv)");
    gtk_file_filter_add_pattern(filter, "*.csv");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (result == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        
        if (load_macro_from_csv(app_data, filename)) {
            GtkWidget *info_dialog = gtk_message_dialog_new(GTK_WINDOW(app_data->main_window),
                                                           GTK_DIALOG_DESTROY_WITH_PARENT,
                                                           GTK_MESSAGE_INFO,
                                                           GTK_BUTTONS_OK,
                                                           "Macro '%s' loaded successfully with %d actions",
                                                           app_data->current_macro.name,
                                                           app_data->current_macro.action_count);
            gtk_dialog_run(GTK_DIALOG(info_dialog));
            gtk_widget_destroy(info_dialog);
        } else {
            GtkWidget *error_dialog = gtk_message_dialog_new(GTK_WINDOW(app_data->main_window),
                                                           GTK_DIALOG_DESTROY_WITH_PARENT,
                                                           GTK_MESSAGE_ERROR,
                                                           GTK_BUTTONS_CLOSE,
                                                           "Failed to load macro from: %s", filename);
            gtk_dialog_run(GTK_DIALOG(error_dialog));
            gtk_widget_destroy(error_dialog);
        }
        
        g_free(filename);
    }
    
    gtk_widget_destroy(dialog);
}

void on_start_macro_clicked(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;
    
    if (app_data->current_macro.action_count == 0) {
        GtkWidget *error_dialog = gtk_message_dialog_new(GTK_WINDOW(app_data->main_window),
                                                       GTK_DIALOG_DESTROY_WITH_PARENT,
                                                       GTK_MESSAGE_WARNING,
                                                       GTK_BUTTONS_OK,
                                                       "No macro loaded. Please load a macro first.");
        gtk_dialog_run(GTK_DIALOG(error_dialog));
        gtk_widget_destroy(error_dialog);
        return;
    }
    
    start_macro_execution(app_data);
}

void on_stop_macro_clicked(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;
    stop_macro_execution(app_data);
}
