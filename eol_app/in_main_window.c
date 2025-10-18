// In your main application:
CanVerifyContext ctx = {0};
ctx.test_cases = load_can_tests_from_csv("test.csv", &ctx.count);
ctx.can_socket = can_socket_init("can0");

if (ctx.can_socket < 0 || !ctx.test_cases) {
    // Handle error
    return -1;
}

GtkWidget *verify_window = create_can_verify_window(&ctx);
gtk_widget_show_all(verify_window);

// Cleanup when done:
// close(ctx.can_socket);
// free_can_tests(ctx.test_cases);