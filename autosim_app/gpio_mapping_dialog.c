
#include "workspace_app.h"
#include "frontend_bridge.h"
#include <stdio.h>

static GPIOMappingEntry g_gpio_mappings[MAX_CONTROLS];
static int g_mapping_count = 0;

void save_gpio_mappings_to_file(GPIOMappingEntry* mappings, int count) {
    FILE* file = fopen("gpio_mapping.conf", "w");
    if (!file) {
        g_print("Error: Cannot create gpio_mapping.conf\n");
        return;
    }
    
    fprintf(file, "# GPIO Backend Configuration\n");
    fprintf(file, "# Format: ControlName,ControlType,GPIOPin,MinValue,MaxValue,InvertLogic\n\n");
    
    int saved_count = 0;
    for (int i = 0; i < count; i++) {
        if (mappings[i].is_mapped && mappings[i].gpio_pin >= 0) {
            fprintf(file, "%s,%d,%d,%.6f,%.6f,%d\n",
                   mappings[i].control_name,
                   mappings[i].control_type,
                   mappings[i].gpio_pin,
                   mappings[i].min_value,
                   mappings[i].max_value,
                   mappings[i].invert_logic ? 1 : 0);
            saved_count++;
        }
    }
    
    fclose(file);
    g_print("GPIO mappings saved to gpio_mapping.conf (%d entries)\n", saved_count);
}

void load_gpio_mappings_from_file(GPIOMappingEntry* mappings, int* count) {
    FILE* file = fopen("gpio_mapping.conf", "r");
    if (!file) {
        g_print("No existing gpio_mapping.conf found\n");
        *count = 0;
        return;
    }
    
    char line[256];
    *count = 0;
    
    while (fgets(line, sizeof(line), file) && *count < MAX_CONTROLS) {
        // Skip comments and empty lines
        if (line[0] == '#' || line[0] == '\n' || strlen(line) < 5) {
            continue;
        }
        
        char control_name[MAX_NAME_LENGTH];
        int control_type, gpio_pin, invert_logic;
        double min_val, max_val;
        
        if (sscanf(line, "%99[^,],%d,%d,%lf,%lf,%d", 
                   control_name, &control_type, &gpio_pin, &min_val, &max_val, &invert_logic) == 6) {
            
            strncpy(mappings[*count].control_name, control_name, MAX_NAME_LENGTH - 1);
            mappings[*count].control_name[MAX_NAME_LENGTH - 1] = '\0';
            mappings[*count].control_type = control_type;
            mappings[*count].gpio_pin = gpio_pin;
            mappings[*count].min_value = min_val;
            mappings[*count].max_value = max_val;
            mappings[*count].invert_logic = (invert_logic != 0);
            mappings[*count].is_mapped = true;
            
            (*count)++;
        }
    }
    
    fclose(file);
    g_print("Loaded %d GPIO mappings from gpio_mapping.conf\n", *count);
}

static void on_gpio_pin_changed(GtkSpinButton *spin_button, gpointer data) {
    int mapping_index = GPOINTER_TO_INT(data);
    int new_pin = gtk_spin_button_get_value_as_int(spin_button);
    
    if (mapping_index >= 0 && mapping_index < g_mapping_count) {
        g_gpio_mappings[mapping_index].gpio_pin = new_pin;
        g_gpio_mappings[mapping_index].is_mapped = (new_pin >= 0);
        g_print("Updated %s -> GPIO %d\n", 
                g_gpio_mappings[mapping_index].control_name, new_pin);
    }
}

static void on_invert_logic_toggled(GtkToggleButton *toggle_button, gpointer data) {
    int mapping_index = GPOINTER_TO_INT(data);
    bool inverted = gtk_toggle_button_get_active(toggle_button);
    
    if (mapping_index >= 0 && mapping_index < g_mapping_count) {
        g_gpio_mappings[mapping_index].invert_logic = inverted;
        g_print("Updated %s invert logic: %s\n", 
                g_gpio_mappings[mapping_index].control_name,
                inverted ? "Yes" : "No");
    }
}

// Separate dialog response handler
static void on_dialog_response(GtkDialog *dialog, gint response_id, gpointer user_data) {
    
    if (response_id == GTK_RESPONSE_ACCEPT) {
        
        // Save current mappings to file
        save_gpio_mappings_to_file(g_gpio_mappings, g_mapping_count);
        
        // Load mappings into backend
        bridge_load_gpio_mapping("gpio_mapping.conf");
        
        // Show confirmation
        GtkWidget *parent = gtk_widget_get_toplevel(GTK_WIDGET(dialog));
        GtkWidget *info_dialog = gtk_message_dialog_new(GTK_WINDOW(parent),
                                                       GTK_DIALOG_DESTROY_WITH_PARENT,
                                                       GTK_MESSAGE_INFO,
                                                       GTK_BUTTONS_OK,
                                                       "GPIO mappings saved successfully!");
        gtk_dialog_run(GTK_DIALOG(info_dialog));
        gtk_widget_destroy(info_dialog);
    } else {
        g_print("Cancel button clicked\n");
    }
    
    gtk_widget_destroy(GTK_WIDGET(dialog));
}

GtkWidget* create_gpio_mapping_dialog(AppData *app_data) {
    // Create dialog
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "GPIO Pin Mapping Configuration",
        GTK_WINDOW(app_data->main_window),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Save & Apply", GTK_RESPONSE_ACCEPT,
        NULL
    );
    
    gtk_window_set_default_size(GTK_WINDOW(dialog), 700, 500);
    
    // Get content area
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_set_border_width(GTK_CONTAINER(content_area), 10);
    
    // Create main container
    GtkWidget *main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    
    // Title and description
    GtkWidget *title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title), 
        "<span size='14000' weight='bold'>GPIO Pin Mapping Configuration</span>");
    gtk_widget_set_halign(title, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(main_vbox), title, FALSE, FALSE, 0);
    
    GtkWidget *description = gtk_label_new(
        "Map workspace controls to Raspberry Pi GPIO pins.\n"
        "Set GPIO pin to -1 to disable mapping for that control.");
    gtk_widget_set_halign(description, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(main_vbox), description, FALSE, FALSE, 5);
    
    // Create scrolled window
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(scrolled, -1, 300);
    
    // Create grid for mappings
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10);
    
    // Column headers
    GtkWidget *header_control = gtk_label_new("Control Name");
    GtkWidget *header_type = gtk_label_new("Type");
    GtkWidget *header_gpio = gtk_label_new("GPIO Pin");
    GtkWidget *header_range = gtk_label_new("Value Range");
    GtkWidget *header_invert = gtk_label_new("Invert Logic");
    
    gtk_widget_set_halign(header_control, GTK_ALIGN_START);
    gtk_widget_set_halign(header_type, GTK_ALIGN_START);
    gtk_widget_set_halign(header_gpio, GTK_ALIGN_START);
    gtk_widget_set_halign(header_range, GTK_ALIGN_START);
    gtk_widget_set_halign(header_invert, GTK_ALIGN_START);
    
    // Make headers bold
    PangoAttrList *attrs = pango_attr_list_new();
    pango_attr_list_insert(attrs, pango_attr_weight_new(PANGO_WEIGHT_BOLD));
    gtk_label_set_attributes(GTK_LABEL(header_control), attrs);
    gtk_label_set_attributes(GTK_LABEL(header_type), attrs);
    gtk_label_set_attributes(GTK_LABEL(header_gpio), attrs);
    gtk_label_set_attributes(GTK_LABEL(header_range), attrs);
    gtk_label_set_attributes(GTK_LABEL(header_invert), attrs);
    pango_attr_list_unref(attrs);
    
    gtk_grid_attach(GTK_GRID(grid), header_control, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), header_type, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), header_gpio, 2, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), header_range, 3, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), header_invert, 4, 0, 1, 1);
    
    // Add separator
    GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_grid_attach(GTK_GRID(grid), separator, 0, 1, 5, 1);
    
    // Load existing mappings
    int existing_mappings = 0;
    load_gpio_mappings_from_file(g_gpio_mappings, &existing_mappings);
    
    // Populate grid with current workspace controls
    g_mapping_count = 0;
    int row = 2;
    
    for (int w = 0; w < app_data->current_workspace.window_count; w++) {
        WindowDef *window = &app_data->current_workspace.windows[w];
        
        for (int c = 0; c < window->control_count; c++) {
            ControlDef *control = &window->controls[c];
            
            if (g_mapping_count >= MAX_CONTROLS) {
                g_print("WARNING: Too many controls, stopping at %d\n", MAX_CONTROLS);
                break;
            }
            
            // Initialize mapping entry
            GPIOMappingEntry *mapping = &g_gpio_mappings[g_mapping_count];
            strncpy(mapping->control_name, control->name, MAX_NAME_LENGTH - 1);
            mapping->control_name[MAX_NAME_LENGTH - 1] = '\0';
            mapping->control_type = (int)control->type;
            mapping->min_value = control->min_val;
            mapping->max_value = control->max_val;
            
            // Check if mapping already exists from file
            bool found_existing = false;
            for (int i = 0; i < existing_mappings; i++) {
                if (strcmp(g_gpio_mappings[i].control_name, control->name) == 0) {
                    found_existing = true;
                    break;
                }
            }
            
            if (!found_existing) {
                mapping->gpio_pin = -1;  // Default: not mapped
                mapping->invert_logic = false;
                mapping->is_mapped = false;
            }
            
            // Control name label
            GtkWidget *label_name = gtk_label_new(control->name);
            gtk_widget_set_halign(label_name, GTK_ALIGN_START);
            gtk_grid_attach(GTK_GRID(grid), label_name, 0, row, 1, 1);
            
            // Control type label
            const char *type_str = 
                control->type == CONTROL_BUTTON ? "Button" :
                control->type == CONTROL_HSLIDER ? "H-Slider" :
                control->type == CONTROL_VSLIDER ? "V-Slider" :
                control->type == CONTROL_SWITCH ? "Switch" : "Unknown";
            
            GtkWidget *label_type = gtk_label_new(type_str);
            gtk_widget_set_halign(label_type, GTK_ALIGN_START);
            gtk_grid_attach(GTK_GRID(grid), label_type, 1, row, 1, 1);
            
            // GPIO pin spin button
            GtkWidget *spin_gpio = gtk_spin_button_new_with_range(-1, 27, 1);
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_gpio), mapping->gpio_pin);
            g_signal_connect(spin_gpio, "value-changed", 
                           G_CALLBACK(on_gpio_pin_changed), GINT_TO_POINTER(g_mapping_count));
            gtk_grid_attach(GTK_GRID(grid), spin_gpio, 2, row, 1, 1);
            
            // Value range label
            char range_text[100];
            if (control->type == CONTROL_BUTTON) {
                strcpy(range_text, "N/A");
            } else {
                snprintf(range_text, sizeof(range_text), "%.1f - %.1f", 
                        control->min_val, control->max_val);
            }
            GtkWidget *label_range = gtk_label_new(range_text);
            gtk_widget_set_halign(label_range, GTK_ALIGN_START);
            gtk_grid_attach(GTK_GRID(grid), label_range, 3, row, 1, 1);
            
            // Invert logic checkbox
            GtkWidget *check_invert = gtk_check_button_new_with_label("Invert");
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_invert), mapping->invert_logic);
            g_signal_connect(check_invert, "toggled", 
                           G_CALLBACK(on_invert_logic_toggled), GINT_TO_POINTER(g_mapping_count));
            gtk_grid_attach(GTK_GRID(grid), check_invert, 4, row, 1, 1);
            
            g_mapping_count++;
            row++;
        }
    }
    
    gtk_container_add(GTK_CONTAINER(scrolled), grid);
    gtk_box_pack_start(GTK_BOX(main_vbox), scrolled, TRUE, TRUE, 0);
    
    // Add info section
    GtkWidget *info_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *info_title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(info_title), "<span weight='bold'>GPIO Pin Reference:</span>");
    gtk_widget_set_halign(info_title, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(info_box), info_title, FALSE, FALSE, 0);
    
    GtkWidget *info_text = gtk_label_new(
        "Common GPIO Pins: 18, 19 (PWM), 20, 21, 22, 23, 24, 25, 26, 27\n"
        "Set pin to -1 to disable mapping. Invert logic swaps HIGH/LOW for switches/buttons.");
    gtk_widget_set_halign(info_text, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(info_box), info_text, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(main_vbox), info_box, FALSE, FALSE, 0);
    
    gtk_container_add(GTK_CONTAINER(content_area), main_vbox);
    
    // FIXED: Connect response signal properly
    g_signal_connect(dialog, "response", G_CALLBACK(on_dialog_response), NULL);
    
    return dialog;
}

void show_gpio_mapping_dialog(AppData *app_data) {
    
    if (!app_data->workspace_loaded) {
        GtkWidget *error_dialog = gtk_message_dialog_new(GTK_WINDOW(app_data->main_window),
                                                       GTK_DIALOG_DESTROY_WITH_PARENT,
                                                       GTK_MESSAGE_WARNING,
                                                       GTK_BUTTONS_OK,
                                                       "Please load a workspace first before configuring GPIO mappings.");
        gtk_dialog_run(GTK_DIALOG(error_dialog));
        gtk_widget_destroy(error_dialog);
        return;
    }
    GtkWidget *dialog = create_gpio_mapping_dialog(app_data);
    
    gtk_widget_show_all(dialog);
    
}

void on_gpio_mapping_clicked(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;
    show_gpio_mapping_dialog(app_data);
}
