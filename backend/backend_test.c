#include "gpio_backend.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

static volatile bool running = true;

void signal_handler(int sig) {
    printf("\nReceived signal %d, shutting down...\n", sig);
    running = false;
}

int main(int argc, char* argv[]) {
    printf("GPIO Backend Test Application\n");
    printf("============================\n\n");
    
    // Install signal handler
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Initialize backend
    if (backend_init() != 0) {
        fprintf(stderr, "Failed to initialize backend\n");
        return 1;
    }
    
    // Configure some GPIO pins for testing
    gpio_configure_pin(18, GPIO_MODE_PWM, "Test PWM");
    gpio_configure_pin(20, GPIO_MODE_OUTPUT, "Test Output");
    gpio_configure_pin(21, GPIO_MODE_OUTPUT, "Test Switch");
    
    // Create some control mappings
    backend_map_control("Test Slider", BACKEND_HSLIDER, 18, 0.0, 100.0);
    backend_map_control("Test Switch", BACKEND_SWITCH, 20, 0.0, 1.0);
    backend_map_control("Test Button", BACKEND_BUTTON, 21, 0.0, 1.0);
    
    // Start real-time processing
    backend_start_realtime(1000);  // 1000Hz = 1ms period
    
    backend_print_status();
    
    printf("\nRunning test loop... (Ctrl+C to exit)\n");
    
    int counter = 0;
    while (running) {
        // Simulate control updates
        double slider_value = 50.0 + 40.0 * (counter * 0.1);

        // Simple loop for changing counter 
        if ( counter == 10) { counter = -10; }

        backend_update_control("Test Slider", slider_value);
        
        // Toggle switch every 2 seconds
        if ((counter % 200) == 0) {
            static bool switch_state = false;
            switch_state = !switch_state;
            backend_update_control("Test Switch", switch_state ? 1.0 : 0.0);
            printf("Switch: %s\n", switch_state ? "ON" : "OFF");
        }
        
        // Button press every 5 seconds
        if ((counter % 500) == 0) {
            backend_update_control("Test Button", 1.0);
            printf("Button pressed\n");
        }
        
        counter++;
        usleep(10000);  // 10ms delay
        
        // Print statistics every 10 seconds
        if ((counter % 1000) == 0) {
            backend_print_status();
        }
    }
    
    // Cleanup
    backend_cleanup();
    
    printf("Test completed\n");
    return 0;
}