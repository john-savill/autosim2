#include "pico_backend.h"
#include "frontend_bridge.h"
#include "workspace_app.h"
#include <stdio.h>
#include <string.h>

static bool bridge_initialized = false;
static GPIOMappingEntry gpio_mappings[MAX_CONTROLS];
static int mapping_count = 0;

int bridge_init(void) {
    if (bridge_initialized) {
        return 0;
    }
    
    printf("Initializing Pico frontend bridge...\n");
    
    // Initialize Pico backend connection
    if (pico_backend_init(PICO_DEFAULT_IP, PICO_DEFAULT_PORT) != 0) {
        fprintf(stderr, "Warning: Failed to initialize Pico connection\n");
    }
    
    // Load GPIO mappings
    bridge_load_gpio_mapping("gpio_mapping.conf");
    
    bridge_initialized = true;
    printf("Pico frontend bridge initialized\n");
    return 0;
}

void bridge_cleanup(void) {
    if (!bridge_initialized) {
        return;
    }
    
    pico_backend_cleanup();
    bridge_initialized = false;
    printf("Pico frontend bridge cleaned up\n");
}

int bridge_control_updated(const char* control_name, double value) {
    if (!bridge_initialized || !control_name) {
        return -1;
    }
    
    // Find the GPIO mapping for this control
    GPIOMappingEntry* mapping = NULL;
    for (int i = 0; i < mapping_count; i++) {
        if (strcmp(gpio_mappings[i].control_name, control_name) == 0) {
            mapping = &gpio_mappings[i];
            break;
        }
    }
    
    if (!mapping || !mapping->is_mapped) {
        printf("No GPIO mapping found for control: %s\n", control_name);
        return -1;
    }
    
    // Send appropriate command to Pico
    switch (mapping->control_type) {
        case BACKEND_HSLIDER:
        case BACKEND_VSLIDER:
            return pico_send_slider_value(control_name, mapping->gpio_pin, (float)value);
            
        case BACKEND_SWITCH:
            return pico_send_switch_state(control_name, mapping->gpio_pin, value > 0.5);
            
        default:
            return -1;
    }
}

int bridge_button_clicked(const char* button_name) {
    if (!bridge_initialized || !button_name) {
        return -1;
    }
    
    // Find the GPIO mapping
    GPIOMappingEntry* mapping = NULL;
    for (int i = 0; i < mapping_count; i++) {
        if (strcmp(gpio_mappings[i].control_name, button_name) == 0) {
            mapping = &gpio_mappings[i];
            break;
        }
    }
    
    if (!mapping || !mapping->is_mapped) {
        printf("No GPIO mapping found for button: %s\n", button_name);
        return -1;
    }
    
    // Send button press followed by release
    int result1 = pico_send_button_press(button_name, mapping->gpio_pin);
    usleep(50000);  // 50ms pulse
    int result2 = pico_send_button_release(button_name, mapping->gpio_pin);
    
    return (result1 == 0 && result2 == 0) ? 0 : -1;
}

int bridge_switch_toggled(const char* switch_name, bool state) {
    if (!bridge_initialized || !switch_name) {
        return -1;
    }
    
    // Find the GPIO mapping
    GPIOMappingEntry* mapping = NULL;
    for (int i = 0; i < mapping_count; i++) {
        if (strcmp(gpio_mappings[i].control_name, switch_name) == 0) {
            mapping = &gpio_mappings[i];
            break;
        }
    }
    
    if (!mapping || !mapping->is_mapped) {
        printf("No GPIO mapping found for switch: %s\n", switch_name);
        return -1;
    }
    
    return pico_send_switch_state(switch_name, mapping->gpio_pin, state);
}

void bridge_print_performance_stats(void) {
    pico_print_stats();
}

int bridge_load_gpio_mapping(const char* mapping_file) {
    FILE* file = fopen(mapping_file, "r");
    if (!file) {
        printf("No GPIO mapping file found: %s\n", mapping_file);
        return -1;
    }
    
    char line[256];
    mapping_count = 0;
    
    while (fgets(line, sizeof(line), file) && mapping_count < MAX_CONTROLS) {
        if (line[0] == '#' || line[0] == '\n' || strlen(line) < 5) {
            continue;
        }
        
        char control_name[64];
        int control_type, gpio_pin, invert_logic;
        double min_val, max_val;
        
        if (sscanf(line, "%63[^,],%d,%d,%lf,%lf,%d", 
                   control_name, &control_type, &gpio_pin, &min_val, &max_val, &invert_logic) == 6) {
            
            GPIOMappingEntry* mapping = &gpio_mappings[mapping_count];
            strncpy(mapping->control_name, control_name, sizeof(mapping->control_name) - 1);
            mapping->control_name[sizeof(mapping->control_name) - 1] = '\0';
            mapping->control_type = control_type;
            mapping->gpio_pin = gpio_pin;
            mapping->min_value = min_val;
            mapping->max_value = max_val;
            mapping->invert_logic = (invert_logic != 0);
            mapping->is_mapped = true;
            
            mapping_count++;
        }
    }
    
    fclose(file);
    printf("Loaded %d GPIO mappings from %s\n", mapping_count, mapping_file);
    return 0;
}
