
#include <gtk/gtk.h>

// Structure to hold application data
typedef struct {
    GtkWidget *main_window;
    GtkWidget *workspace;
    int window_count;
} AppData;

// Callback functions
static void on_button_clicked(GtkWidget *widget, gpointer data) {
    const char *button_name = (const char *)data;
    g_print("Button '%s' was clicked!\n", button_name);
}

static void on_scale_changed(GtkRange *range, gpointer data) {
    const char *scale_name = (const char *)data;
    double value = gtk_range_get_value(range);
    g_print("Scale '%s' changed to: %.2f\n", scale_name, value);
}

static void on_new_window_clicked(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;
    
    // Create a new internal window
    char window_title[50];
    sprintf(window_title, "Internal Window %d", ++app_data->window_count);
    
    GtkWidget *frame = gtk_frame_new(window_title);
    gtk_frame_set_label_align(GTK_FRAME(frame), 0.5, 0.5);
    
    // Create content for the internal window
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
    
    // Add some buttons
    char button_label[50];
    for (int i = 1; i <= 3; i++) {
        sprintf(button_label, "Button %d", i);
        GtkWidget *button = gtk_button_new_with_label(button_label);
        char *button_data = g_strdup(button_label);
        g_signal_connect(button, "clicked", G_CALLBACK(on_button_clicked), button_data);
        gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
    }
    
    // Add a horizontal slider
    GtkWidget *hscale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.0, 100.0, 1.0);
    gtk_scale_set_value_pos(GTK_SCALE(hscale), GTK_POS_TOP);
    gtk_range_set_value(GTK_RANGE(hscale), 50.0);
    char *hscale_data = g_strdup("Horizontal Scale");
    g_signal_connect(hscale, "value-changed", G_CALLBACK(on_scale_changed), hscale_data);
    gtk_box_pack_start(GTK_BOX(vbox), hscale, FALSE, FALSE, 0);
    
    // Add a vertical slider
    GtkWidget *vscale = gtk_scale_new_with_range(GTK_ORIENTATION_VERTICAL, 0.0, 100.0, 1.0);
    gtk_scale_set_value_pos(GTK_SCALE(vscale), GTK_POS_RIGHT);
    gtk_range_set_value(GTK_RANGE(vscale), 25.0);
    gtk_widget_set_size_request(vscale, -1, 100);
    char *vscale_data = g_strdup("Vertical Scale");
    g_signal_connect(vscale, "value-changed", G_CALLBACK(on_scale_changed), vscale_data);
    gtk_box_pack_start(GTK_BOX(vbox), vscale, FALSE, FALSE, 0);
    
    gtk_container_add(GTK_CONTAINER(frame), vbox);
    gtk_box_pack_start(GTK_BOX(app_data->workspace), frame, TRUE, TRUE, 5);
    
    gtk_widget_show_all(frame);
}

static GtkWidget *create_internal_window(const char *title, int window_num) {
    // Create a frame to act as an internal window
    GtkWidget *frame = gtk_frame_new(title);
    gtk_frame_set_label_align(GTK_FRAME(frame), 0.5, 0.5);
    
    // Create a vertical box for content
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
    
    // Add some sample buttons
    char button_label[50];
    for (int i = 1; i <= 3; i++) {
        sprintf(button_label, "Window %d - Button %d", window_num, i);
        GtkWidget *button = gtk_button_new_with_label(button_label);
        char *button_data = g_strdup(button_label);
        g_signal_connect(button, "clicked", G_CALLBACK(on_button_clicked), button_data);
        gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
    }
    
    // Add a horizontal slider
    GtkWidget *hscale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.0, 100.0, 1.0);
    gtk_scale_set_value_pos(GTK_SCALE(hscale), GTK_POS_TOP);
    gtk_range_set_value(GTK_RANGE(hscale), 50.0 + (window_num * 10));
    char scale_label[50];
    sprintf(scale_label, "Window %d - H-Scale", window_num);
    char *hscale_data = g_strdup(scale_label);
    g_signal_connect(hscale, "value-changed", G_CALLBACK(on_scale_changed), hscale_data);
    gtk_box_pack_start(GTK_BOX(vbox), hscale, FALSE, FALSE, 0);
    
    // Add a vertical slider in a horizontal box (so it doesn't expand too much)
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *vscale = gtk_scale_new_with_range(GTK_ORIENTATION_VERTICAL, 0.0, 100.0, 1.0);
    gtk_scale_set_value_pos(GTK_SCALE(vscale), GTK_POS_RIGHT);
    gtk_range_set_value(GTK_RANGE(vscale), 25.0 + (window_num * 15));
    gtk_widget_set_size_request(vscale, -1, 100);
    sprintf(scale_label, "Window %d - V-Scale", window_num);
    char *vscale_data = g_strdup(scale_label);
    g_signal_connect(vscale, "value-changed", G_CALLBACK(on_scale_changed), vscale_data);
    
    gtk_box_pack_start(GTK_BOX(hbox), vscale, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new("Vertical Slider"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
    
    gtk_container_add(GTK_CONTAINER(frame), vbox);
    
    return frame;
}

static void activate(GtkApplication *app, gpointer user_data) {
    AppData *app_data = (AppData *)user_data;
    
    // Create the main window
    app_data->main_window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(app_data->main_window), "GTK Workspace Application");
    gtk_window_set_default_size(GTK_WINDOW(app_data->main_window), 800, 600);
    
    // Create main vertical box
    GtkWidget *main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    
    // Create toolbar
    GtkWidget *toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(toolbar), 5);
    
    GtkWidget *new_window_btn = gtk_button_new_with_label("Add Internal Window");
    g_signal_connect(new_window_btn, "clicked", G_CALLBACK(on_new_window_clicked), app_data);
    gtk_box_pack_start(GTK_BOX(toolbar), new_window_btn, FALSE, FALSE, 0);
    
    GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(main_vbox), toolbar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_vbox), separator, FALSE, FALSE, 0);
    
    // Create scrolled window for the workspace
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), 
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    
    // Create workspace (horizontal box for side-by-side windows)
    app_data->workspace = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(app_data->workspace), 10);
    
    // Create initial internal windows
    app_data->window_count = 2;
    GtkWidget *window1 = create_internal_window("Control Panel 1", 1);
    GtkWidget *window2 = create_internal_window("Control Panel 2", 2);
    
    gtk_box_pack_start(GTK_BOX(app_data->workspace), window1, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(app_data->workspace), window2, TRUE, TRUE, 5);
    
    gtk_container_add(GTK_CONTAINER(scrolled), app_data->workspace);
    gtk_box_pack_start(GTK_BOX(main_vbox), scrolled, TRUE, TRUE, 0);
    
    gtk_container_add(GTK_CONTAINER(app_data->main_window), main_vbox);
    gtk_widget_show_all(app_data->main_window);
}

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;
    AppData app_data = {0};
    
    app = gtk_application_new("com.example.workspace", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), &app_data);
    
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    
    return status;
}
