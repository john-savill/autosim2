#include "workspace_app.h"

void create_default_workspace(AppData *app_data) {
    // Create a default workspace
    strcpy(app_data->current_workspace.name, "Default Workspace");
    app_data->current_workspace.window_count = 2;
    
    // Define first window - UPDATE control count to include switches
    strcpy(app_data->current_workspace.windows[0].title, "Control Panel 1");
    app_data->current_workspace.windows[0].control_count = 7; // INCREASED from 5
    
    // Add controls to first window
    app_data->current_workspace.windows[0].controls[0] = (ControlDef){CONTROL_BUTTON, "Start Process", 0, 0, 0};
    app_data->current_workspace.windows[0].controls[1] = (ControlDef){CONTROL_BUTTON, "Stop Process", 0, 0, 0};
    app_data->current_workspace.windows[0].controls[2] = (ControlDef){CONTROL_BUTTON, "Emergency Stop", 0, 0, 0};
    app_data->current_workspace.windows[0].controls[3] = (ControlDef){CONTROL_HSLIDER, "Speed Control", 50, 0, 100};
    app_data->current_workspace.windows[0].controls[4] = (ControlDef){CONTROL_VSLIDER, "Power Level", 75, 0, 100};
    // ADD THESE NEW SWITCH CONTROLS:
    app_data->current_workspace.windows[0].controls[5] = (ControlDef){CONTROL_SWITCH, "Main Power", 0, 0, 1}; // OFF by default
    app_data->current_workspace.windows[0].controls[6] = (ControlDef){CONTROL_SWITCH, "Auto Mode", 1, 0, 1}; // ON by default
    
    // Define second window - UPDATE control count
    strcpy(app_data->current_workspace.windows[1].title, "Monitor Panel");
    app_data->current_workspace.windows[1].control_count = 5; // INCREASED from 4
    
    // Add controls to second window
    app_data->current_workspace.windows[1].controls[0] = (ControlDef){CONTROL_BUTTON, "Refresh Data", 0, 0, 0};
    app_data->current_workspace.windows[1].controls[1] = (ControlDef){CONTROL_BUTTON, "Export Log", 0, 0, 0};
    app_data->current_workspace.windows[1].controls[2] = (ControlDef){CONTROL_HSLIDER, "Update Rate", 30, 1, 60};
    app_data->current_workspace.windows[1].controls[3] = (ControlDef){CONTROL_VSLIDER, "Alert Level", 60, 0, 100};
    // ADD THIS NEW SWITCH CONTROL:
    app_data->current_workspace.windows[1].controls[4] = (ControlDef){CONTROL_SWITCH, "Monitor Enable", 1, 0, 1}; // ON by default
}

// Helper function to escape CSV fields that contain commas or quotes
static char* escape_csv_field(const char* field) {
    if (strchr(field, ',') || strchr(field, '"') || strchr(field, '\n')) {
        // Need to escape - wrap in quotes and double any internal quotes
        int len = strlen(field);
        char* escaped = malloc(len * 2 + 3); // worst case: all chars are quotes, plus surrounding quotes and null
        
        escaped[0] = '"';
        int pos = 1;
        
        for (int i = 0; i < len; i++) {
            if (field[i] == '"') {
                escaped[pos++] = '"';
                escaped[pos++] = '"';
            } else {
                escaped[pos++] = field[i];
            }
        }
        
        escaped[pos++] = '"';
        escaped[pos] = '\0';
        
        return escaped;
    } else {
        return strdup(field);
    }
}

void save_workspace_simple(AppData *app_data, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        g_print("Error: Could not open file for writing: %s\n", filename);
        return;
    }
    
    // Write CSV header
    fprintf(file, "Section,Type,WindowIndex,WindowTitle,ControlIndex,ControlType,ControlName,Value,MinValue,MaxValue\n");
    
    // Write workspace metadata
    char* escaped_name = escape_csv_field(app_data->current_workspace.name);
    fprintf(file, "WORKSPACE,METADATA,-1,%s,%d,-,-,-,-,-\n", 
            escaped_name, app_data->current_workspace.window_count);
    free(escaped_name);
    
    // Write each window and its controls
    for (int i = 0; i < app_data->current_workspace.window_count; i++) {
        WindowDef *window = &app_data->current_workspace.windows[i];
        
        // Write window header
        char* escaped_title = escape_csv_field(window->title);
        fprintf(file, "WINDOW,HEADER,%d,%s,%d,-,-,-,-,-\n", 
                i, escaped_title, window->control_count);
        
        // Write each control - DON'T FREE escaped_title yet, we need it for controls
        for (int j = 0; j < window->control_count; j++) {
            ControlDef *control = &window->controls[j];
            
            char* escaped_control_name = escape_csv_field(control->name);
            const char* control_type = 
                control->type == CONTROL_BUTTON ? "BUTTON" :
                control->type == CONTROL_HSLIDER ? "HSLIDER" :
                control->type == CONTROL_VSLIDER ? "VSLIDER" :
                control->type == CONTROL_SWITCH ? "SWITCH" : "UNKNOWN";
            
            if (control->type == CONTROL_BUTTON) {
                fprintf(file, "CONTROL,DATA,%d,%s,%d,%s,%s,-,-,-\n", 
                        i, escaped_title, j, control_type, escaped_control_name);
            } else if (control->type == CONTROL_SWITCH) {
                fprintf(file, "CONTROL,DATA,%d,%s,%d,%s,%s,%.0f,-,-\n", 
                        i, escaped_title, j, control_type, escaped_control_name, control->value);
            } else {
                // Sliders - USE HIGHER PRECISION (6 decimal places)
                fprintf(file, "CONTROL,DATA,%d,%s,%d,%s,%s,%.6f,%.6f,%.6f\n", 
                        i, escaped_title, j, control_type, escaped_control_name,
                        control->value, control->min_val, control->max_val);
            }
            
            free(escaped_control_name);
        }
        
        // NOW we can free escaped_title after all controls for this window are written
        free(escaped_title);
    }
    
    fclose(file);
}

gboolean load_workspace_simple(AppData *app_data, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        g_print("Error: Could not open file for reading: %s\n", filename);
        return FALSE;
    }
    
    char line[1024]; // INCREASED buffer size for longer lines
    int line_number = 0;
    
    // Initialize workspace
    memset(&app_data->current_workspace, 0, sizeof(WorkspaceDef));
    
    // Skip header line
    if (fgets(line, sizeof(line), file)) {
        line_number++;
    }
    
    while (fgets(line, sizeof(line), file)) {
        line_number++;
        
        // Remove newline
        line[strcspn(line, "\n")] = 0;
        
        // Skip empty lines
        if (strlen(line) == 0) {
            continue;
        }
        
        // Enhanced CSV parsing with better field handling
        char* fields[10];
        int field_count = 0;
        char* line_copy = strdup(line); // Make a copy for parsing
        char* ptr = line_copy;
        
        // Improved CSV parser - handles quotes and commas properly
        while (*ptr && field_count < 10) {
            // Skip leading whitespace
            while (*ptr == ' ' || *ptr == '\t') ptr++;
            
            char* field_start = ptr;
            
            if (*ptr == '"') {
                // Quoted field
                ptr++; // Skip opening quote
                field_start = ptr;
                
                // Find closing quote
                while (*ptr && *ptr != '"') {
                    ptr++;
                }
                
                if (*ptr == '"') {
                    *ptr = '\0'; // Null-terminate the field
                    ptr++; // Skip closing quote
                }
                
                // Skip comma if present
                if (*ptr == ',') {
                    ptr++;
                }
            } else {
                // Unquoted field
                while (*ptr && *ptr != ',') {
                    ptr++;
                }
                
                if (*ptr == ',') {
                    *ptr = '\0'; // Null-terminate the field
                    ptr++;
                }
            }
            
            fields[field_count++] = field_start;
        }
        
        // Process the parsed fields
        if (field_count >= 4) {
            char* section = fields[0];
            char* type = fields[1];
            int window_index = atoi(fields[2]);
            char* window_title = fields[3];
            
            if (strcmp(section, "WORKSPACE") == 0 && strcmp(type, "METADATA") == 0) {
                strncpy(app_data->current_workspace.name, window_title, MAX_NAME_LENGTH - 1);
                app_data->current_workspace.name[MAX_NAME_LENGTH - 1] = '\0';
                if (field_count >= 5) {
                    app_data->current_workspace.window_count = atoi(fields[4]);
                }
                
            } else if (strcmp(section, "WINDOW") == 0 && strcmp(type, "HEADER") == 0) {
                if (window_index >= 0 && window_index < MAX_WINDOWS && field_count >= 5) {
                    strncpy(app_data->current_workspace.windows[window_index].title, window_title, MAX_NAME_LENGTH - 1);
                    app_data->current_workspace.windows[window_index].title[MAX_NAME_LENGTH - 1] = '\0';
                    app_data->current_workspace.windows[window_index].control_count = atoi(fields[4]);
                }
                
            } else if (strcmp(section, "CONTROL") == 0 && strcmp(type, "DATA") == 0 && field_count >= 7) {
                int control_index = atoi(fields[4]);
                char* control_type = fields[5];
                char* control_name = fields[6];
                
                if (window_index >= 0 && window_index < MAX_WINDOWS && 
                    control_index >= 0 && control_index < MAX_CONTROLS) {
                    
                    ControlDef *control = &app_data->current_workspace.windows[window_index].controls[control_index];
                    strncpy(control->name, control_name, MAX_NAME_LENGTH - 1);
                    control->name[MAX_NAME_LENGTH - 1] = '\0';
                    
                    // Parse control type
                    if (strcmp(control_type, "BUTTON") == 0) {
                        control->type = CONTROL_BUTTON;
                    } else if (strcmp(control_type, "HSLIDER") == 0) {
                        control->type = CONTROL_HSLIDER;
                    } else if (strcmp(control_type, "VSLIDER") == 0) {
                        control->type = CONTROL_VSLIDER;
                    } else if (strcmp(control_type, "SWITCH") == 0) {
                        control->type = CONTROL_SWITCH;
                    }
                    
                    // Parse values based on control type with HIGHER PRECISION
                    if (control->type == CONTROL_SWITCH) {
                        // Switch: only parse the value (0 or 1)
                        if (field_count >= 8 && strcmp(fields[7], "-") != 0) {
                            control->value = (atof(fields[7]) != 0.0) ? 1.0 : 0.0;
                        }
                    } else if (control->type != CONTROL_BUTTON) {
                        // Sliders: parse value, min, max with HIGH PRECISION
                        if (field_count >= 8 && strcmp(fields[7], "-") != 0) {
                            control->value = strtod(fields[7], NULL); // Use strtod for better precision
                        }
                        if (field_count >= 9 && strcmp(fields[8], "-") != 0) {
                            control->min_val = strtod(fields[8], NULL);
                        }
                        if (field_count >= 10 && strcmp(fields[9], "-") != 0) {
                            control->max_val = strtod(fields[9], NULL);
                        }
                    }
                    
                    // Debug output for loaded controls
                    g_print("Loaded control: %s, type: %d, value: %.6f\n", 
                            control->name, control->type, control->value);
                }
            }
        }
        
        free(line_copy); // Free the copy we made for parsing
    }
    
    fclose(file);
    
    g_print("Successfully loaded workspace '%s' with %d windows\n", 
            app_data->current_workspace.name, app_data->current_workspace.window_count);
    
    return TRUE;
}