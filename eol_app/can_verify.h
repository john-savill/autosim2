#ifndef CAN_VERIFY_H
#define CAN_VERIFY_H

#include <gtk/gtk.h>
#include <stdbool.h>

typedef struct {
    char sensor[64];
    uint32_t send_id;
    uint32_t expected_return_val;
    float perc_error_margin;
} CanTestCase;

typedef struct {
    CanTestCase *test_cases;
    int count;
    int current_test;
    GtkWidget *tree_view;
    GtkListStore *list_store;
    int can_socket;
} CanVerifyContext;

// CSV parsing
CanTestCase* load_can_tests_from_csv(const char *filename, int *count);
void free_can_tests(CanTestCase *tests);

// CAN communication
int can_socket_init(const char *interface);
bool send_can_message(int socket, uint32_t id, uint8_t *data, uint8_t len);
bool receive_can_message(int socket, uint32_t *id, uint8_t *data, uint8_t *len, int timeout_ms);

// Verification
bool verify_can_response(uint32_t expected, uint32_t received, float margin);
void run_can_test(CanVerifyContext *ctx, int test_index);
void run_all_tests(CanVerifyContext *ctx);

// GTK UI
GtkWidget* create_can_verify_window(CanVerifyContext *ctx);

#endif