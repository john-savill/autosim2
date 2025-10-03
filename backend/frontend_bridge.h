#ifndef FRONTEND_BRIDGE_H
#define FRONTEND_BRIDGE_H

#include "gpio_backend.h"

// Bridge initialization
int bridge_init(void);
void bridge_cleanup(void);

// Frontend to backend communication
int bridge_control_updated(const char* control_name, double value);
int bridge_button_clicked(const char* button_name);
int bridge_switch_toggled(const char* switch_name, bool state);

// Configuration management
int bridge_load_gpio_mapping(const char* mapping_file);
int bridge_setup_default_mapping(void);

// Status monitoring
void bridge_print_performance_stats(void);

#endif