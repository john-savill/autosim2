
#include "workspace_app.h"

void create_default_workspace(AppData *app_data) {
    // Create a default workspace
    strcpy(app_data->current_workspace.name, "Default Workspace");
    app_data->current_workspace.window_count = 2;
    
    // Define first window
    strcpy(app_data->current_workspace.windows[0].title, "Control Panel 1");
    app_data->current_workspace.windows[0].control_count = 5;
    
    // Add controls to first window
    app_data->current_workspace.windows[0].controls[0] = (ControlDef){CONTROL_BUTTON, "Start Process", 0, 0, 0};
    app_data->current_workspace.windows[0].controls[1] = (ControlDef){CONTROL_BUTTON, "Stop Process", 0, 0, 0};
    app_data->current_workspace.windows[0].controls[2] = (ControlDef){CONTROL_BUTTON, "Emergency Stop", 0, 0, 0};
    app_data->current_workspace.windows[0].controls[3] = (ControlDef){CONTROL_HSLIDER, "Speed Control", 50, 0, 100};
    app_data->current_workspace.windows[0].controls[4] = (ControlDef){CONTROL_VSLIDER, "Power Level", 75, 0, 100};
    
    // Define second window
    strcpy(app_data->current_workspace.windows[1].title, "Monitor Panel");
    app_data->current_workspace.windows[1].control_count = 4;
    
    // Add controls to second window
    app_data->current_workspace.windows[1].controls[0] = (ControlDef){CONTROL_BUTTON, "Refresh Data", 0, 0, 0};
    app_data->current_workspace.windows[1].controls[1] = (ControlDef){CONTROL_BUTTON, "Export Log", 0, 0, 0};
    app_data->current_workspace.windows[1].controls[2] = (ControlDef){CONTROL_HSLIDER, "Update Rate", 30, 1, 60};
    app_data->current_workspace.windows[1].controls[3] = (ControlDef){CONTROL_VSLIDER, "Alert Level", 60, 0, 100};
}

void save_workspace_simple(AppData *app_data, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        g_print("Error: Could not open file for writing: %s\n", filename);
        return;
    }
    
    // Write workspace name
    fprintf(file, "WORKSPACE_NAME=%s\n", app_data->current_workspace.name);
    fprintf(file, "WINDOW_COUNT=%d\n", app_data->current_workspace.window_count);
    fprintf(file, "\n");
    
    // Write each window
    for (int i = 0; i < app_data->current_workspace.window_count; i++) {
        WindowDef *window = &app_data->current_workspace.windows[i];
        
        fprintf(file, "[WINDOW_%d]\n", i);
        fprintf(file, "TITLE=%s\n", window->title);
        fprintf(file, "CONTROL_COUNT=%d\n", window->control_count);
        
        // Write each control
        for (int j = 0; j < window->control_count; j++) {
            ControlDef *control = &window->controls[j];
            
            fprintf(file, "CONTROL_%d_TYPE=%s\n", j,
                    control->type == CONTROL_BUTTON ? "BUTTON" :
                    control->type == CONTROL_HSLIDER ? "HSLIDER" : "VSLIDER");
            fprintf(file, "CONTROL_%d_NAME=%s\n", j, control->name);
            
            if (control->type != CONTROL_BUTTON) {
                fprintf(file, "CONTROL_%d_VALUE=%.2f\n", j, control->value);
                fprintf(file, "CONTROL_%d_MIN=%.2f\n", j, control->min_val);
                fprintf(file, "CONTROL_%d_MAX=%.2f\n", j, control->max_val);
            }
        }
        fprintf(file, "\n");
    }
    
    fclose(file);
}

gboolean load_workspace_simple(AppData *app_data, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        g_print("Error: Could not open file for reading: %s\n", filename);
        return FALSE;
    }
    
    char line[256];
    char key[128], value[128];
    int current_window = -1;
    
    // Initialize workspace
    memset(&app_data->current_workspace, 0, sizeof(WorkspaceDef));
    
    while (fgets(line, sizeof(line), file)) {
        // Remove newline
        line[strcspn(line, "\n")] = 0;
        
        // Skip empty lines and comments
        if (strlen(line) == 0 || line[0] == '#') {
            continue;
        }
        
        // Check for window section
        if (line[0] == '[' && strstr(line, "WINDOW_")) {
            sscanf(line, "[WINDOW_%d]", &current_window);
            continue;
        }
        
        // Parse key=value pairs
        if (sscanf(line, "%127[^=]=%127s", key, value) == 2) {
            if (strcmp(key, "WORKSPACE_NAME") == 0) {
                strcpy(app_data->current_workspace.name, value);
            } else if (strcmp(key, "WINDOW_COUNT") == 0) {
                app_data->current_workspace.window_count = atoi(value);
            } else if (current_window >= 0 && current_window < MAX_WINDOWS) {
                WindowDef *window = &app_data->current_workspace.windows[current_window];
                
                if (strcmp(key, "TITLE") == 0) {
                    strcpy(window->title, value);
                } else if (strcmp(key, "CONTROL_COUNT") == 0) {
                    window->control_count = atoi(value);
                } else if (strstr(key, "CONTROL_") == key) {
                    int control_idx;
                    char control_attr[64];
                    
                    if (sscanf(key, "CONTROL_%d_%63s", &control_idx, control_attr) == 2) {
                        if (control_idx >= 0 && control_idx < MAX_CONTROLS) {
                            ControlDef *control = &window->controls[control_idx];
                            
                            if (strcmp(control_attr, "TYPE") == 0) {
                                if (strcmp(value, "BUTTON") == 0) {
                                    control->type = CONTROL_BUTTON;
                                } else if (strcmp(value, "HSLIDER") == 0) {
                                    control->type = CONTROL_HSLIDER;
                                } else if (strcmp(value, "VSLIDER") == 0) {
                                    control->type = CONTROL_VSLIDER;
                                }
                            } else if (strcmp(control_attr, "NAME") == 0) {
                                strcpy(control->name, value);
                            } else if (strcmp(control_attr, "VALUE") == 0) {
                                control->value = atof(value);
                            } else if (strcmp(control_attr, "MIN") == 0) {
                                control->min_val = atof(value);
                            } else if (strcmp(control_attr, "MAX") == 0) {
                                control->max_val = atof(value);
                            }
                        }
                    }
                }
            }
        }
    }
    
    fclose(file);
    return TRUE;
}
