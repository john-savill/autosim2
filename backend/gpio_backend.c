#include "gpio_backend.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <sched.h>
#include <sys/time.h>
#include <signal.h>

// Raspberry Pi GPIO base addresses
#define BCM2835_PERI_BASE   0x3F000000  // Pi 3/4
#define BCM2711_PERI_BASE   0xFE000000  // Pi 4 (alternative)
#define GPIO_BASE_OFFSET    0x200000
#define BLOCK_SIZE          4096

// GPIO register offsets
#define GPFSEL0    0   // Function select
#define GPSET0     7   // Pin output set
#define GPCLR0     10  // Pin output clear
#define GPLEV0     13  // Pin level
#define GPEDS0     16  // Event detect status
#define GPREN0     19  // Rising edge detect enable
#define GPFEN0     22  // Falling edge detect enable

// PWM register offsets
#define PWM_BASE_OFFSET 0x20C000
#define PWM_CTL  0
#define PWM_STA  1
#define PWM_DMAC 2
#define PWM_RNG1 4
#define PWM_DAT1 5
#define PWM_FIF1 6
#define PWM_RNG2 8
#define PWM_DAT2 9

// Clock register offsets
#define CLOCK_BASE_OFFSET 0x101000
#define PWMCLK_CNTL 40
#define PWMCLK_DIV  41

// Global state
static BackendState g_backend_state = {0};
static volatile uint32_t *gpio_map = NULL;
static volatile uint32_t *pwm_map = NULL;
static volatile uint32_t *clk_map = NULL;
static int mem_fd = -1;

// Function implementations
double backend_get_timestamp_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000000.0 + (double)ts.tv_nsec / 1000.0;
}

static int setup_memory_mapping(void) {
    // Open /dev/mem
    mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (mem_fd < 0) {
        fprintf(stderr, "Error: Cannot open /dev/mem. Are you running as root?\n");
        return -1;
    }
    
    // Detect Pi version and set base address
    uint32_t peri_base = BCM2835_PERI_BASE;
    
    // Try to detect Pi 4
    FILE *cpuinfo = fopen("/proc/cpuinfo", "r");
    if (cpuinfo) {
        char line[256];
        while (fgets(line, sizeof(line), cpuinfo)) {
            if (strstr(line, "BCM2711")) {
                peri_base = BCM2711_PERI_BASE;
                break;
            }
        }
        fclose(cpuinfo);
    }
    
    printf("Using peripheral base: 0x%08X\n", peri_base);
    
    // Map GPIO memory
    gpio_map = (volatile uint32_t *)mmap(
        NULL, BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
        mem_fd, peri_base + GPIO_BASE_OFFSET
    );
    
    if (gpio_map == MAP_FAILED) {
        fprintf(stderr, "Error: GPIO mmap failed: %s\n", strerror(errno));
        close(mem_fd);
        return -1;
    }
    
    // Map PWM memory
    pwm_map = (volatile uint32_t *)mmap(
        NULL, BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
        mem_fd, peri_base + PWM_BASE_OFFSET
    );
    
    if (pwm_map == MAP_FAILED) {
        fprintf(stderr, "Warning: PWM mmap failed: %s\n", strerror(errno));
        pwm_map = NULL;
    }
    
    // Map Clock memory
    clk_map = (volatile uint32_t *)mmap(
        NULL, BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
        mem_fd, peri_base + CLOCK_BASE_OFFSET
    );
    
    if (clk_map == MAP_FAILED) {
        fprintf(stderr, "Warning: Clock mmap failed: %s\n", strerror(errno));
        clk_map = NULL;
    }
    
    return 0;
}

static void cleanup_memory_mapping(void) {
    if (gpio_map && gpio_map != MAP_FAILED) {
        munmap((void*)gpio_map, BLOCK_SIZE);
        gpio_map = NULL;
    }
    if (pwm_map && pwm_map != MAP_FAILED) {
        munmap((void*)pwm_map, BLOCK_SIZE);
        pwm_map = NULL;
    }
    if (clk_map && clk_map != MAP_FAILED) {
        munmap((void*)clk_map, BLOCK_SIZE);
        clk_map = NULL;
    }
    if (mem_fd >= 0) {
        close(mem_fd);
        mem_fd = -1;
    }
}

static void gpio_set_function(int bcm_pin, int function) {
    if (!gpio_map || bcm_pin < 0 || bcm_pin > 27) return;
    
    int reg = bcm_pin / 10;
    int shift = (bcm_pin % 10) * 3;
    uint32_t mask = 0b111 << shift;
    
    gpio_map[GPFSEL0 + reg] = (gpio_map[GPFSEL0 + reg] & ~mask) | ((function & 0b111) << shift);
}

int gpio_configure_pin(int bcm_pin, GPIOMode mode, const char* name) {
    if (bcm_pin < 0 || bcm_pin >= MAX_GPIO_PINS) {
        return -1;
    }
    
    pthread_mutex_lock(&g_backend_state.state_mutex);
    
    GPIOPin* pin = &g_backend_state.pins[bcm_pin];
    pin->bcm_number = bcm_pin;
    pin->mode = mode;
    pin->is_active = true;
    strncpy(pin->name, name ? name : "", MAX_NAME_LENGTH - 1);
    pin->name[MAX_NAME_LENGTH - 1] = '\0';
    clock_gettime(CLOCK_MONOTONIC, &pin->last_update);
    
    // Configure GPIO function
    switch (mode) {
        case GPIO_MODE_INPUT:
            gpio_set_function(bcm_pin, 0b000);  // Input
            break;
        case GPIO_MODE_OUTPUT:
            gpio_set_function(bcm_pin, 0b001);  // Output
            break;
        case GPIO_MODE_PWM:
            // PWM setup would go here
            gpio_set_function(bcm_pin, 0b001);  // Output for now
            break;
        case GPIO_MODE_DISABLED:
            pin->is_active = false;
            break;
    }
    
    g_backend_state.pin_count++;
    
    pthread_mutex_unlock(&g_backend_state.state_mutex);
    
    printf("Configured GPIO %d as %s: %s\n", bcm_pin, 
           mode == GPIO_MODE_INPUT ? "INPUT" : 
           mode == GPIO_MODE_OUTPUT ? "OUTPUT" : 
           mode == GPIO_MODE_PWM ? "PWM" : "DISABLED", name);
    
    return 0;
}

int gpio_set_output(int bcm_pin, GPIOState state) {
    if (!gpio_map || bcm_pin < 0 || bcm_pin >= MAX_GPIO_PINS) {
        return -1;
    }
    
    pthread_mutex_lock(&g_backend_state.state_mutex);
    
    GPIOPin* pin = &g_backend_state.pins[bcm_pin];
    if (!pin->is_active || pin->mode != GPIO_MODE_OUTPUT) {
        pthread_mutex_unlock(&g_backend_state.state_mutex);
        return -1;
    }
    
    // Set or clear the pin
    if (state == GPIO_STATE_HIGH) {
        gpio_map[GPSET0 + bcm_pin/32] = 1 << (bcm_pin % 32);
    } else {
        gpio_map[GPCLR0 + bcm_pin/32] = 1 << (bcm_pin % 32);
    }
    
    pin->current_state = state;
    clock_gettime(CLOCK_MONOTONIC, &pin->last_update);
    
    pthread_mutex_unlock(&g_backend_state.state_mutex);
    
    return 0;
}

int gpio_get_input(int bcm_pin, GPIOState* state) {
    if (!gpio_map || !state || bcm_pin < 0 || bcm_pin >= MAX_GPIO_PINS) {
        return -1;
    }
    
    pthread_mutex_lock(&g_backend_state.state_mutex);
    
    GPIOPin* pin = &g_backend_state.pins[bcm_pin];
    if (!pin->is_active) {
        pthread_mutex_unlock(&g_backend_state.state_mutex);
        return -1;
    }
    
    // Read pin state
    uint32_t level = gpio_map[GPLEV0 + bcm_pin/32] & (1 << (bcm_pin % 32));
    *state = level ? GPIO_STATE_HIGH : GPIO_STATE_LOW;
    
    pin->current_state = *state;
    clock_gettime(CLOCK_MONOTONIC, &pin->last_update);
    
    pthread_mutex_unlock(&g_backend_state.state_mutex);
    
    return 0;
}

int gpio_set_pwm(int bcm_pin, double duty_cycle, uint32_t frequency) {
    if (bcm_pin < 0 || bcm_pin >= MAX_GPIO_PINS || duty_cycle < 0.0 || duty_cycle > 100.0) {
        return -1;
    }
    
    pthread_mutex_lock(&g_backend_state.state_mutex);
    
    GPIOPin* pin = &g_backend_state.pins[bcm_pin];
    if (!pin->is_active || pin->mode != GPIO_MODE_PWM) {
        pthread_mutex_unlock(&g_backend_state.state_mutex);
        return -1;
    }
    
    // Software PWM implementation (for now)
    // Hardware PWM would require more complex setup
    pin->pwm_duty_cycle = (uint32_t)duty_cycle;
    pin->pwm_frequency = frequency;
    pin->current_value = duty_cycle / 100.0;
    
    clock_gettime(CLOCK_MONOTONIC, &pin->last_update);
    
    pthread_mutex_unlock(&g_backend_state.state_mutex);
    
    return 0;
}

int backend_map_control(const char* control_name, BackendControlType type, 
                       int bcm_pin, double min_val, double max_val) {
    if (!control_name || bcm_pin < 0 || bcm_pin >= MAX_GPIO_PINS) {
        return -1;
    }
    
    pthread_mutex_lock(&g_backend_state.state_mutex);
    
    if (g_backend_state.mapping_count >= 100) {
        pthread_mutex_unlock(&g_backend_state.state_mutex);
        return -1;
    }
    
    ControlMapping* mapping = &g_backend_state.mappings[g_backend_state.mapping_count];
    strncpy(mapping->control_name, control_name, MAX_NAME_LENGTH - 1);
    mapping->control_name[MAX_NAME_LENGTH - 1] = '\0';
    mapping->control_type = type;
    mapping->gpio_pin = bcm_pin;
    mapping->min_value = min_val;
    mapping->max_value = max_val;
    mapping->invert_logic = false;
    mapping->debounce_ms = 10;
    mapping->is_mapped = true;
    
    g_backend_state.mapping_count++;
    
    pthread_mutex_unlock(&g_backend_state.state_mutex);
    
    printf("Mapped control '%s' to GPIO %d\n", control_name, bcm_pin);
    return 0;
}

int backend_update_control(const char* control_name, double value) {
    if (!control_name) return -1;
    
    double start_time = backend_get_timestamp_us();
    
    pthread_mutex_lock(&g_backend_state.state_mutex);
    
    // Find the mapping
    ControlMapping* mapping = NULL;
    for (int i = 0; i < g_backend_state.mapping_count; i++) {
        if (strcmp(g_backend_state.mappings[i].control_name, control_name) == 0) {
            mapping = &g_backend_state.mappings[i];
            break;
        }
    }
    
    if (!mapping || !mapping->is_mapped) {
        pthread_mutex_unlock(&g_backend_state.state_mutex);
        return -1;
    }
    
    int result = 0;
    
    switch (mapping->control_type) {
        case BACKEND_HSLIDER:
        case BACKEND_VSLIDER: {
            // Map slider value to PWM or analog output
            double normalized = (value - mapping->min_value) / (mapping->max_value - mapping->min_value);
            normalized = normalized < 0.0 ? 0.0 : (normalized > 1.0 ? 1.0 : normalized);
            
            if (g_backend_state.pins[mapping->gpio_pin].mode == GPIO_MODE_PWM) {
                result = gpio_set_pwm(mapping->gpio_pin, normalized * 100.0, 1000);
            } else {
                // Digital approximation
                GPIOState state = normalized > 0.5 ? GPIO_STATE_HIGH : GPIO_STATE_LOW;
                result = gpio_set_output(mapping->gpio_pin, state);
            }
            break;
        }
        
        case BACKEND_SWITCH: {
            GPIOState state = (value > 0.5) ? GPIO_STATE_HIGH : GPIO_STATE_LOW;
            if (mapping->invert_logic) {
                state = (state == GPIO_STATE_HIGH) ? GPIO_STATE_LOW : GPIO_STATE_HIGH;
            }
            result = gpio_set_output(mapping->gpio_pin, state);
            break;
        }
        
        case BACKEND_BUTTON: {
            // Button press - momentary high
            result = gpio_set_output(mapping->gpio_pin, GPIO_STATE_HIGH);
            break;
        }
    }
    
    // Update statistics
    g_backend_state.stats.total_updates++;
    double latency = backend_get_timestamp_us() - start_time;
    g_backend_state.stats.avg_latency_us = 
        (g_backend_state.stats.avg_latency_us * 0.99) + (latency * 0.01);
    
    if (latency > g_backend_state.stats.max_latency_us) {
        g_backend_state.stats.max_latency_us = latency;
    }
    
    if (latency > 1000.0) {  // > 1ms
        g_backend_state.stats.missed_deadlines++;
    }
    
    pthread_mutex_unlock(&g_backend_state.state_mutex);
    
    return result;
}

// Real-time thread function
static void* realtime_thread(void* arg) {
    int update_rate_hz = *(int*)arg;
    struct timespec sleep_time;
    sleep_time.tv_sec = 0;
    sleep_time.tv_nsec = 1000000000L / update_rate_hz;  // Convert Hz to nanoseconds
    
    printf("Real-time thread started at %d Hz (period: %ld ns)\n", 
           update_rate_hz, sleep_time.tv_nsec);
    
    while (!g_backend_state.shutdown_requested) {
        struct timespec start_time;
        clock_gettime(CLOCK_MONOTONIC, &start_time);
        
        // Process any pending updates here
        // This is where you'd handle time-critical operations
        
        // Sleep for the remainder of the period
        nanosleep(&sleep_time, NULL);
    }
    
    printf("Real-time thread terminated\n");
    return NULL;
}

int backend_set_thread_priority(pthread_t thread, int priority) {
    struct sched_param param;
    param.sched_priority = priority;
    
    int result = pthread_setschedparam(thread, SCHED_FIFO, &param);
    if (result != 0) {
        fprintf(stderr, "Warning: Failed to set thread priority: %s\n", strerror(result));
        return -1;
    }
    
    return 0;
}

int backend_start_realtime(int update_rate_hz) {
    if (g_backend_state.real_time_enabled) {
        return -1;  // Already running
    }
    
    g_backend_state.update_rate_hz = update_rate_hz;
    g_backend_state.shutdown_requested = false;
    
    int result = pthread_create(&g_backend_state.rt_thread, NULL, 
                               realtime_thread, &update_rate_hz);
    
    if (result != 0) {
        fprintf(stderr, "Error: Failed to create real-time thread: %s\n", strerror(result));
        return -1;
    }
    
    // Set real-time priority
    backend_set_thread_priority(g_backend_state.rt_thread, 90);
    
    g_backend_state.real_time_enabled = true;
    
    printf("Real-time backend started at %d Hz\n", update_rate_hz);
    return 0;
}

void backend_stop_realtime(void) {
    if (!g_backend_state.real_time_enabled) {
        return;
    }
    
    g_backend_state.shutdown_requested = true;
    pthread_join(g_backend_state.rt_thread, NULL);
    g_backend_state.real_time_enabled = false;
    
    printf("Real-time backend stopped\n");
}

int backend_init(void) {
    memset(&g_backend_state, 0, sizeof(BackendState));
    
    if (pthread_mutex_init(&g_backend_state.state_mutex, NULL) != 0) {
        fprintf(stderr, "Error: Failed to initialize mutex\n");
        return -1;
    }
    
    if (setup_memory_mapping() != 0) {
        pthread_mutex_destroy(&g_backend_state.state_mutex);
        return -1;
    }
    
    g_backend_state.is_initialized = true;
    
    printf("GPIO Backend initialized (version %s)\n", BACKEND_VERSION);
    return 0;
}

void backend_cleanup(void) {
    if (!g_backend_state.is_initialized) {
        return;
    }
    
    backend_stop_realtime();
    cleanup_memory_mapping();
    pthread_mutex_destroy(&g_backend_state.state_mutex);
    
    g_backend_state.is_initialized = false;
    
    printf("GPIO Backend cleaned up\n");
}

BackendState* backend_get_state(void) {
    return &g_backend_state;
}

RTStats* backend_get_stats(void) {
    return &g_backend_state.stats;
}

void backend_print_status(void) {
    pthread_mutex_lock(&g_backend_state.state_mutex);
    
    printf("\n=== GPIO Backend Status ===\n");
    printf("Initialized: %s\n", g_backend_state.is_initialized ? "Yes" : "No");
    printf("Real-time: %s\n", g_backend_state.real_time_enabled ? "Enabled" : "Disabled");
    printf("Update rate: %d Hz\n", g_backend_state.update_rate_hz);
    printf("Active pins: %d\n", g_backend_state.pin_count);
    printf("Control mappings: %d\n", g_backend_state.mapping_count);
    
    printf("\n=== Statistics ===\n");
    printf("Total updates: %lu\n", g_backend_state.stats.total_updates);
    printf("Missed deadlines: %lu\n", g_backend_state.stats.missed_deadlines);
    printf("Average latency: %.2f µs\n", g_backend_state.stats.avg_latency_us);
    printf("Max latency: %.2f µs\n", g_backend_state.stats.max_latency_us);
    
    if (g_backend_state.stats.total_updates > 0) {
        double deadline_miss_rate = (double)g_backend_state.stats.missed_deadlines / 
                                   g_backend_state.stats.total_updates * 100.0;
        printf("Deadline miss rate: %.3f%%\n", deadline_miss_rate);
    }
    
    printf("\n=== Active Pins ===\n");
    for (int i = 0; i < MAX_GPIO_PINS; i++) {
        if (g_backend_state.pins[i].is_active) {
            GPIOPin* pin = &g_backend_state.pins[i];
            printf("GPIO %d (%s): %s, State: %s\n", 
                   pin->bcm_number, pin->name,
                   pin->mode == GPIO_MODE_INPUT ? "INPUT" : 
                   pin->mode == GPIO_MODE_OUTPUT ? "OUTPUT" : 
                   pin->mode == GPIO_MODE_PWM ? "PWM" : "DISABLED",
                   pin->current_state == GPIO_STATE_HIGH ? "HIGH" : "LOW");
        }
    }
    
    printf("\n=== Control Mappings ===\n");
    for (int i = 0; i < g_backend_state.mapping_count; i++) {
        ControlMapping* mapping = &g_backend_state.mappings[i];
        if (mapping->is_mapped) {
            printf("%s -> GPIO %d (%.2f-%.2f)\n", 
                   mapping->control_name, mapping->gpio_pin,
                   mapping->min_value, mapping->max_value);
        }
    }
    
    pthread_mutex_unlock(&g_backend_state.state_mutex);
}

int backend_save_config(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Error: Cannot open config file for writing: %s\n", filename);
        return -1;
    }
    
    pthread_mutex_lock(&g_backend_state.state_mutex);
    
    fprintf(file, "# GPIO Backend Configuration\n");
    fprintf(file, "# Format: ControlName,ControlType,GPIOPin,MinValue,MaxValue,InvertLogic\n\n");
    
    for (int i = 0; i < g_backend_state.mapping_count; i++) {
        ControlMapping* mapping = &g_backend_state.mappings[i];
        if (mapping->is_mapped) {
            fprintf(file, "%s,%d,%d,%.6f,%.6f,%d\n",
                   mapping->control_name, mapping->control_type, mapping->gpio_pin,
                   mapping->min_value, mapping->max_value, mapping->invert_logic ? 1 : 0);
        }
    }
    
    pthread_mutex_unlock(&g_backend_state.state_mutex);
    
    fclose(file);
    printf("Backend configuration saved to: %s\n", filename);
    return 0;
}

int backend_load_config(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Cannot open config file for reading: %s\n", filename);
        return -1;
    }
    
    char line[256];
    int loaded_count = 0;
    
    while (fgets(line, sizeof(line), file)) {
        // Skip comments and empty lines
        if (line[0] == '#' || line[0] == '\n' || strlen(line) < 5) {
            continue;
        }
        
        char control_name[MAX_NAME_LENGTH];
        int control_type, gpio_pin, invert_logic;
        double min_val, max_val;
        
        if (sscanf(line, "%99[^,],%d,%d,%lf,%lf,%d", 
                   control_name, &control_type, &gpio_pin, &min_val, &max_val, &invert_logic) == 6) {
            
            if (backend_map_control(control_name, (BackendControlType)control_type, 
                                   gpio_pin, min_val, max_val) == 0) {
                g_backend_state.mappings[g_backend_state.mapping_count - 1].invert_logic = 
                    invert_logic ? true : false;
                loaded_count++;
            }
        }
    }
    
    fclose(file);
    printf("Loaded %d control mappings from: %s\n", loaded_count, filename);
    return 0;
}