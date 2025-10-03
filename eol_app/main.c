#include "eol_app.h"

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;
    AppData app_data = {0};
    
    app = gtk_application_new("eol_tester.alpha", G_APPLICATION_DEFAULT_FLAGS); //G_APPLICATION_FLAGS_NONE is depr
    g_signal_connect(app, "activate", G_CALLBACK(activate_application), &app_data);
    
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    
    return status;
}
