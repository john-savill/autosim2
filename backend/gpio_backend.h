#ifndef GPIO_BACKEND_H
#define GPIO_BACKEND_H

#include "backend_types.h"

// Initialization and cleanup
int backend_init(void);
void backend_cleanup(void);

// GPIO pin management
int gpio_configure_pin(int bcm_pin, GPIOMode mode, const char* name);
int gpio_set_output(int bcm_pin, GPIOState state);
int gpio_get_input(int bcm_pin, GPIOState* state);
int gpio_set_pwm(int bcm_pin, double duty_cycle, uint32_t frequency);

// Control mapping
int backend_map_control(const char* control_name, BackendControlType type, 
                       int bcm_pin, double min_val, double max_val);
int backend_unmap_control(const char* control_name);

// Real-time control interface
int backend_update_control(const char* control_name, double value);
int backend_button_press(const char* control_name);
int backend_switch_set(const char* control_name, bool state);

// Real-time thread management
int backend_start_realtime(int update_rate_hz);
void backend_stop_realtime(void);

// Status and monitoring
BackendState* backend_get_state(void);
RTStats* backend_get_stats(void);
void backend_print_status(void);

// Configuration save/load
int backend_save_config(const char* filename);
int backend_load_config(const char* filename);

// Utility functions
double backend_get_timestamp_us(void);
int backend_set_thread_priority(pthread_t thread, int priority);

#endif