#include "frontend_bridge.h"
#include <stdio.h>
#include <string.h>
// Just for usleep:
#include <unistd.h>

static bool bridge_initialized = false;

int bridge_init(void) {
    if (bridge_initialized) {
        return 0;
    }
    
    printf("Initializing frontend-backend bridge...\n");
    
    // Initialize backend
    if (backend_init() != 0) {
        fprintf(stderr, "Error: Failed to initialize GPIO backend\n");
        return -1;
    }
    
    // Setup default GPIO configuration
    bridge_setup_default_mapping();
    
    // Start real-time processing at 1000Hz (1ms period)
    if (backend_start_realtime(1000) != 0) {
        fprintf(stderr, "Warning: Failed to start real-time processing\n");
    }
    
    bridge_initialized = true;
    printf("Frontend-backend bridge initialized\n");
    return 0;
}

void bridge_cleanup(void) {
    if (!bridge_initialized) {
        return;
    }
    
    backend_cleanup();
    bridge_initialized = false;
    printf("Frontend-backend bridge cleaned up\n");
}

int bridge_control_updated(const char* control_name, double value) {
    if (!bridge_initialized || !control_name) {
        return -1;
    }
    
    return backend_update_control(control_name, value);
}

int bridge_button_clicked(const char* button_name) {
    if (!bridge_initialized || !button_name) {
        return -1;
    }
    
    // Button press - set high then low after brief delay
    int result = backend_update_control(button_name, 1.0);
    usleep(10000);  // 10ms pulse
    backend_update_control(button_name, 0.0);
    
    return result;
}

int bridge_switch_toggled(const char* switch_name, bool state) {
    if (!bridge_initialized || !switch_name) {
        return -1;
    }
    
    return backend_update_control(switch_name, state ? 1.0 : 0.0);
}

int bridge_setup_default_mapping(void) {
    printf("Setting up default GPIO mappings...\n");
    
    // Configure some GPIO pins
    gpio_configure_pin(18, GPIO_MODE_PWM, "PWM Output 1");
    gpio_configure_pin(19, GPIO_MODE_PWM, "PWM Output 2");
    gpio_configure_pin(20, GPIO_MODE_OUTPUT, "Digital Output 1");
    gpio_configure_pin(21, GPIO_MODE_OUTPUT, "Digital Output 2");
    gpio_configure_pin(22, GPIO_MODE_OUTPUT, "Button Output 1");
    gpio_configure_pin(23, GPIO_MODE_OUTPUT, "Button Output 2");
    
    // Map frontend controls to GPIO pins
    backend_map_control("Speed Control", BACKEND_HSLIDER, 18, 0.0, 100.0);
    backend_map_control("Power Level", BACKEND_VSLIDER, 19, 0.0, 100.0);
    backend_map_control("Main Power", BACKEND_SWITCH, 20, 0.0, 1.0);
    backend_map_control("Auto Mode", BACKEND_SWITCH, 21, 0.0, 1.0);
    backend_map_control("Start Process", BACKEND_BUTTON, 22, 0.0, 1.0);
    backend_map_control("Stop Process", BACKEND_BUTTON, 23, 0.0, 1.0);
    
    printf("Default GPIO mappings configured\n");
    return 0;
}

int bridge_load_gpio_mapping(const char* mapping_file) {
    return backend_load_config(mapping_file);
}

void bridge_print_performance_stats(void) {
    backend_print_status();
}