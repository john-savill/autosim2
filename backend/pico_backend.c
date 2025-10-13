#include "pico_backend.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>

static PicoConnection g_pico_conn = {0};

// Calculate simple checksum
static uint16_t calculate_checksum(const PicoCommand* cmd) {
    uint16_t checksum = 0;
    const uint8_t* data = (const uint8_t*)cmd;
    size_t len = sizeof(PicoCommand) - sizeof(cmd->checksum);
    
    for (size_t i = 0; i < len; i++) {
        checksum += data[i];
    }
    
    return checksum;
}

// Get current timestamp in microseconds
static uint64_t get_timestamp_us(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000000 + tv.tv_usec;
}

int pico_backend_init(const char* pico_ip, uint16_t port) {
    printf("Initializing Pico backend connection to %s:%d\n", pico_ip, port);
    
    // Create UDP socket
    g_pico_conn.socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (g_pico_conn.socket_fd < 0) {
        fprintf(stderr, "Error creating socket: %s\n", strerror(errno));
        return -1;
    }
    
    // Set socket timeout
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = PICO_TIMEOUT_MS * 1000;
    setsockopt(g_pico_conn.socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    
    // Setup Pico address
    memset(&g_pico_conn.pico_addr, 0, sizeof(g_pico_conn.pico_addr));
    g_pico_conn.pico_addr.sin_family = AF_INET;
    g_pico_conn.pico_addr.sin_port = htons(port);
    inet_pton(AF_INET, pico_ip, &g_pico_conn.pico_addr.sin_addr);
    
    strncpy(g_pico_conn.pico_ip, pico_ip, sizeof(g_pico_conn.pico_ip) - 1);
    g_pico_conn.pico_port = port;
    
    // Test connection with ping
    if (pico_ping() == 0) {
        g_pico_conn.is_connected = true;
        printf("Successfully connected to Pico at %s:%d\n", pico_ip, port);
        return 0;
    } else {
        printf("Warning: Could not ping Pico, but socket created\n");
        g_pico_conn.is_connected = false;
        return 0;  // Return success anyway, connection might come later
    }
}

void pico_backend_cleanup(void) {
    if (g_pico_conn.socket_fd >= 0) {
        close(g_pico_conn.socket_fd);
        g_pico_conn.socket_fd = -1;
    }
    g_pico_conn.is_connected = false;
    printf("Pico backend cleaned up\n");
}

int pico_send_command(PicoCommandType type, const char* control_name, uint8_t gpio_pin, float value) {
    PicoCommand cmd = {0};
    
    cmd.type = type;
    strncpy(cmd.control_name, control_name ? control_name : "", sizeof(cmd.control_name) - 1);
    cmd.gpio_pin = gpio_pin;
    cmd.value = value;
    cmd.timestamp = (uint32_t)(get_timestamp_us() & 0xFFFFFFFF);
    cmd.checksum = calculate_checksum(&cmd);
    
    uint64_t start_time = get_timestamp_us();
    
    // Send command
    ssize_t sent = sendto(g_pico_conn.socket_fd, &cmd, sizeof(cmd), 0,
                         (struct sockaddr*)&g_pico_conn.pico_addr, sizeof(g_pico_conn.pico_addr));
    
    if (sent != sizeof(cmd)) {
        fprintf(stderr, "Error sending command: %s\n", strerror(errno));
        g_pico_conn.timeouts++;
        return -1;
    }
    
    // Wait for acknowledgment (not required)
    char ack_buffer[16];
    ssize_t received = recvfrom(g_pico_conn.socket_fd, ack_buffer, sizeof(ack_buffer), 0, NULL, NULL);
    
    uint64_t end_time = get_timestamp_us();
    double latency = (end_time - start_time) / 1000.0;  // Convert to milliseconds
    
    if (received > 0) {
        g_pico_conn.ack_received++;
        // Update average latency
        g_pico_conn.avg_latency_ms = (g_pico_conn.avg_latency_ms * 0.9) + (latency * 0.1);
    } else {
        g_pico_conn.timeouts++;
    }
    
    g_pico_conn.commands_sent++;
    
    printf("Sent %s command for %s (GPIO %d) = %.2f, latency: %.2f ms\n",
           type == PICO_CMD_BUTTON_PRESS ? "BUTTON_PRESS" :
           type == PICO_CMD_SLIDER_SET ? "SLIDER_SET" :
           type == PICO_CMD_SWITCH_ON ? "SWITCH_ON" :
           type == PICO_CMD_SWITCH_OFF ? "SWITCH_OFF" : "UNKNOWN",
           control_name, gpio_pin, value, latency);
    
    return 0;
}

int pico_send_button_press(const char* control_name, uint8_t gpio_pin) {
    return pico_send_command(PICO_CMD_BUTTON_PRESS, control_name, gpio_pin, 1.0);
}

int pico_send_button_release(const char* control_name, uint8_t gpio_pin) {
    return pico_send_command(PICO_CMD_BUTTON_RELEASE, control_name, gpio_pin, 0.0);
}

int pico_send_slider_value(const char* control_name, uint8_t gpio_pin, float value) {
    return pico_send_command(PICO_CMD_SLIDER_SET, control_name, gpio_pin, value);
}

int pico_send_switch_state(const char* control_name, uint8_t gpio_pin, bool state) {
    return pico_send_command(state ? PICO_CMD_SWITCH_ON : PICO_CMD_SWITCH_OFF, 
                           control_name, gpio_pin, state ? 1.0 : 0.0);
}

int pico_send_pwm_value(const char* control_name, uint8_t gpio_pin, float duty_cycle, uint32_t frequency) {
    // For PWM, we can encode frequency in the command if needed
    return pico_send_command(PICO_CMD_PWM_SET, control_name, gpio_pin, duty_cycle);
}

int pico_ping(void) {
    return pico_send_command(PICO_CMD_PING, "PING", 0, 0.0);
}

bool pico_is_connected(void) {
    return g_pico_conn.is_connected && g_pico_conn.socket_fd >= 0;
}

PicoConnection* pico_get_connection_stats(void) {
    return &g_pico_conn;
}

void pico_print_stats(void) {
    printf("\n=== Pico Connection Statistics ===\n");
    printf("Pico IP: %s:%d\n", g_pico_conn.pico_ip, g_pico_conn.pico_port);
    printf("Connected: %s\n", g_pico_conn.is_connected ? "Yes" : "No");
    printf("Commands sent: %lu\n", g_pico_conn.commands_sent);
    printf("Acknowledgments received: %lu\n", g_pico_conn.ack_received);
    printf("Timeouts: %lu\n", g_pico_conn.timeouts);
    
    if (g_pico_conn.commands_sent > 0) {
        double success_rate = (double)g_pico_conn.ack_received / g_pico_conn.commands_sent * 100.0;
        printf("Success rate: %.1f%%\n", success_rate);
    }
    
    printf("Average latency: %.2f ms\n", g_pico_conn.avg_latency_ms);
    printf("==================================\n");
}
