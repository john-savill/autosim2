#include "can_verify.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <poll.h>

// CSV parsing
CanTestCase* load_can_tests_from_csv(const char *filename, int *count) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return NULL;
    
    char line[256];
    fgets(line, sizeof(line), fp); // Skip header
    
    CanTestCase *tests = NULL;
    *count = 0;
    int capacity = 10;
    tests = malloc(capacity * sizeof(CanTestCase));
    
    while (fgets(line, sizeof(line), fp)) {
        if (*count >= capacity) {
            capacity *= 2;
            tests = realloc(tests, capacity * sizeof(CanTestCase));
        }
        
        char sensor[64], send_id[16];
        int expected_val, margin;
        
        if (sscanf(line, "%[^,],%[^,],%d,%d", sensor, send_id, &expected_val, &margin) == 4) {
            strncpy(tests[*count].sensor, sensor, 63);
            tests[*count].send_id = strtoul(send_id, NULL, 16);
            tests[*count].expected_return_val = expected_val;
            tests[*count].perc_error_margin = margin;
            (*count)++;
        }
    }
    
    fclose(fp);
    return tests;
}

void free_can_tests(CanTestCase *tests) {
    free(tests);
}

// CAN socket initialization
int can_socket_init(const char *interface) {
    int s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (s < 0) return -1;
    
    struct ifreq ifr;
    strcpy(ifr.ifr_name, interface);
    ioctl(s, SIOCGIFINDEX, 𝔦);
    
    struct sockaddr_can addr;
    memset(&addr, 0, sizeof(addr));
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    
    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(s);
        return -1;
    }
    
    return s;
}

// Send CAN message
bool send_can_message(int socket, uint32_t id, uint8_t *data, uint8_t len) {
    struct can_frame frame;
    memset(&frame, 0, sizeof(frame));
    
    frame.can_id = id;
    frame.can_dlc = len;
    if (data && len > 0) {
        memcpy(frame.data, data, len);
    }
    
    if (write(socket, &frame, sizeof(frame)) != sizeof(frame)) {
        return false;
    }
    return true;
}

// Receive CAN message with timeout
bool receive_can_message(int socket, uint32_t *id, uint8_t *data, uint8_t *len, int timeout_ms) {
    struct pollfd pfd;
    pfd.fd = socket;
    pfd.events = POLLIN;
    
    int ret = poll(&pfd, 1, timeout_ms);
    if (ret <= 0) return false;
    
    struct can_frame frame;
    if (read(socket, &frame, sizeof(frame)) < 0) return false;
    
    *id = frame.can_id;
    *len = frame.can_dlc;
    memcpy(data, frame.data, frame.can_dlc);
    
    return true;
}

// Verify response within margin
bool verify_can_response(uint32_t expected, uint32_t received, float margin) {
    if (margin == 0) {
        return expected == received;
    }
    
    float lower = expected * (1.0 - margin / 100.0);
    float upper = expected * (1.0 + margin / 100.0);
    
    return (received >= lower && received <= upper);
}

// Run single test
void run_can_test(CanVerifyContext *ctx, int test_index) {
    if (test_index < 0 || test_index >= ctx->count) return;
    
    CanTestCase *test = &ctx->test_cases[test_index];
    
    // Prepare send data (CAN ID in message)
    uint8_t send_data[2];
    send_data[0] = (test->send_id >> 8) & 0xFF;
    send_data[1] = test->send_id & 0xFF;
    
    // Send message
    if (!send_can_message(ctx->can_socket, test->send_id, send_data, 2)) {
        printf("Failed to send CAN message for %s\n", test->sensor);
        return;
    }
    
    // Receive response
    uint32_t recv_id;
    uint8_t recv_data[8];
    uint8_t recv_len;
    
    if (!receive_can_message(ctx->can_socket, &recv_id, recv_data, &recv_len, 1000)) {
        printf("Timeout waiting for response from %s\n", test->sensor);
        return;
    }
    
    // Extract value from response (assuming last byte contains value)
    uint32_t received_val = recv_len > 0 ? recv_data[recv_len - 1] : 0;
    
    // Verify
    bool pass = verify_can_response(test->expected_return_val, received_val, test->perc_error_margin);
    
    // Update UI
    GtkTreeIter iter;
    gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(ctx->list_store), &iter, NULL, test_index);
    gtk_list_store_set(ctx->list_store, &iter, 
                       4, received_val,
                       5, pass ? "PASS" : "FAIL",
                       -1);
}

// Run all tests
void run_all_tests(CanVerifyContext *ctx) {
    for (int i = 0; i < ctx->count; i++) {
        run_can_test(ctx, i);
        usleep(100000); // 100ms delay between tests
    }
}

// GTK callbacks
static void on_run_test_clicked(GtkButton *button, gpointer user_data) {
    CanVerifyContext *ctx = (CanVerifyContext *)user_data;
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(ctx->tree_view));
    GtkTreeModel *model;
    GtkTreeIter iter;
    
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        GtkTreePath *path = gtk_tree_model_get_path(model, &iter);
        int *indices = gtk_tree_path_get_indices(path);
        run_can_test(ctx, indices[0]);
        gtk_tree_path_free(path);
    }
}

static void on_run_all_clicked(GtkButton *button, gpointer user_data) {
    run_all_tests((CanVerifyContext *)user_data);
}

// Create GTK window
GtkWidget* create_can_verify_window(CanVerifyContext *ctx) {
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "CAN Message Verification");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    
    // Buttons
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *btn_run = gtk_button_new_with_label("Run Selected Test");
    GtkWidget *btn_run_all = gtk_button_new_with_label("Run All Tests");
    
    g_signal_connect(btn_run, "clicked", G_CALLBACK(on_run_test_clicked), ctx);
    g_signal_connect(btn_run_all, "clicked", G_CALLBACK(on_run_all_clicked), ctx);
    
    gtk_box_pack_start(GTK_BOX(hbox), btn_run, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(hbox), btn_run_all, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);
    
    // Tree view
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    ctx->tree_view = gtk_tree_view_new();
    gtk_container_add(GTK_CONTAINER(scrolled), ctx->tree_view);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled, TRUE, TRUE, 5);
    
    // Columns
    const char *titles[] = {"Sensor", "Send ID", "Expected", "Margin %", "Received", "Result"};
    for (int i = 0; i < 6; i++) {
        GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
        GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(
            titles[i], renderer, "text", i, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(ctx->tree_view), column);
    }
    
    // List store
    ctx->list_store = gtk_list_store_new(6, G_TYPE_STRING, G_TYPE_STRING, 
                                          G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_STRING);
    gtk_tree_view_set_model(GTK_TREE_VIEW(ctx->tree_view), GTK_TREE_MODEL(ctx->list_store));
    
    // Populate with test cases
    for (int i = 0; i < ctx->count; i++) {
        GtkTreeIter iter;
        char send_id_str[16];
        snprintf(send_id_str, sizeof(send_id_str), "0x%04X", ctx->test_cases[i].send_id);
        
        gtk_list_store_append(ctx->list_store, &iter);
        gtk_list_store_set(ctx->list_store, &iter,
                          0, ctx->test_cases[i].sensor,
                          1, send_id_str,
                          2, ctx->test_cases[i].expected_return_val,
                          3, (int)ctx->test_cases[i].perc_error_margin,
                          4, 0,
                          5, "PENDING",
                          -1);
    }
    
    return window;
}