#ifndef BACKEND_TYPES_H
#define BACKEND_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>

#define MAX_GPIO_PINS 40
#define MAX_NAME_LENGTH 100
#define BACKEND_VERSION "1.0.0"

// GPIO Pin modes
typedef enum {
    GPIO_MODE_INPUT,
    GPIO_MODE_OUTPUT,
    GPIO_MODE_PWM,
    GPIO_MODE_DISABLED
} GPIOMode;

// GPIO Pin states
typedef enum {
    GPIO_STATE_LOW = 0,
    GPIO_STATE_HIGH = 1
} GPIOState;

// Control types matching frontend
typedef enum {
    BACKEND_BUTTON,
    BACKEND_HSLIDER,
    BACKEND_VSLIDER,
    BACKEND_SWITCH
} BackendControlType;

// GPIO Pin configuration
typedef struct {
    int pin_number;              // Physical pin number
    int bcm_number;              // BCM GPIO number
    GPIOMode mode;
    char name[MAX_NAME_LENGTH];
    bool is_active;
    GPIOState current_state;
    double current_value;        // For PWM/analog (0.0-1.0)
    uint32_t pwm_frequency;      // PWM frequency in Hz
    uint32_t pwm_duty_cycle;     // PWM duty cycle (0-100)
    struct timespec last_update; // Last update timestamp
} GPIOPin;

// Control to GPIO mapping
typedef struct {
    char control_name[MAX_NAME_LENGTH];
    BackendControlType control_type;
    int gpio_pin;                // BCM GPIO number
    double min_value;
    double max_value;
    bool invert_logic;           // Invert HIGH/LOW for buttons/switches
    uint32_t debounce_ms;        // Debounce time for inputs
    bool is_mapped;
} ControlMapping;

// Real-time statistics
typedef struct {
    uint64_t total_updates;
    uint64_t missed_deadlines;
    double avg_latency_us;
    double max_latency_us;
    struct timespec last_stats_update;
} RTStats;

// Backend system state
typedef struct {
    GPIOPin pins[MAX_GPIO_PINS];
    ControlMapping mappings[100];  // Max 100 control mappings
    int pin_count;
    int mapping_count;
    bool is_initialized;
    bool real_time_enabled;
    pthread_t rt_thread;
    pthread_mutex_t state_mutex;
    volatile bool shutdown_requested;
    RTStats stats;
    int update_rate_hz;          // Target update rate (1000Hz for 1ms)
} BackendState;

#endif