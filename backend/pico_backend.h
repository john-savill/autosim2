#ifndef PICO_BACKEND_H
#define PICO_BACKEND_H

#include <stdint.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PICO_DEFAULT_IP "192.168.1.100"  // Pico's IP address
#define PICO_DEFAULT_PORT 8888
#define MAX_COMMAND_SIZE 256
#define PICO_TIMEOUT_MS 100

// Command types
typedef enum {
    PICO_CMD_BUTTON_PRESS,
    PICO_CMD_BUTTON_RELEASE,
    PICO_CMD_SLIDER_SET,
    PICO_CMD_SWITCH_ON,
    PICO_CMD_SWITCH_OFF,
    PICO_CMD_PWM_SET,
    PICO_CMD_DIGITAL_SET,
    PICO_CMD_PING,
    PICO_CMD_STATUS_REQUEST
} PicoCommandType;

// Command structure
typedef struct {
    PicoCommandType type;
    char control_name[64];
    uint8_t gpio_pin;
    float value;
    uint32_t timestamp;
    uint16_t checksum;
} PicoCommand;

// Pico connection state
typedef struct {
    int socket_fd;
    struct sockaddr_in pico_addr;
    bool is_connected;
    char pico_ip[16];
    uint16_t pico_port;
    uint64_t commands_sent;
    uint64_t ack_received;
    uint64_t timeouts;
    double avg_latency_ms;
} PicoConnection;

// Function declarations
int pico_backend_init(const char* pico_ip, uint16_t port);
void pico_backend_cleanup(void);
int pico_send_command(PicoCommandType type, const char* control_name, uint8_t gpio_pin, float value);
int pico_send_button_press(const char* control_name, uint8_t gpio_pin);
int pico_send_button_release(const char* control_name, uint8_t gpio_pin);
int pico_send_slider_value(const char* control_name, uint8_t gpio_pin, float value);
int pico_send_switch_state(const char* control_name, uint8_t gpio_pin, bool state);
int pico_send_pwm_value(const char* control_name, uint8_t gpio_pin, float duty_cycle, uint32_t frequency);
int pico_ping(void);
bool pico_is_connected(void);
PicoConnection* pico_get_connection_stats(void);
void pico_print_stats(void);

#endif
