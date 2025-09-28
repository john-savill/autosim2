
#include "workspace_app.h"

void on_button_clicked(GtkWidget *widget, gpointer data) {
    const char *button_name = (const char *)data;
    g_print("Button '%s' was clicked!\n", button_name);
    
    // You can add custom logic here based on button name
    if (g_strcmp0(button_name, "Start Process") == 0) {
        g_print("Starting process...\n");
    } else if (g_strcmp0(button_name, "Stop Process") == 0) {
        g_print("Stopping process...\n");
    } else if (g_strcmp0(button_name, "Emergency Stop") == 0) {
        g_print("EMERGENCY STOP ACTIVATED!\n");
    }
}

void on_scale_changed(GtkRange *range, gpointer data) {
    const char *scale_name = (const char *)data;
    double value = gtk_range_get_value(range);
    g_print("Scale '%s' changed to: %.2f\n", scale_name, value);
    
    // You can add custom logic here based on scale name
    if (g_strcmp0(scale_name, "Speed Control") == 0) {
        g_print("Setting speed to %.0f RPM\n", value);
    } else if (g_strcmp0(scale_name, "Temperature") == 0) {
        g_print("Setting temperature to %.1f°C\n", value);
    }
}
