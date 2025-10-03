#include "eol_app.h"

GtkWidget *create_welcome_screen(AppData *app_data) {
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 50);
    
    // Title
    GtkWidget *title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title), "<span size='24000' weight='bold'>End of Line Tester Alpha\nV0.01\nDeveloped by John Savill as part of the autosim2 application family</span>");
    gtk_widget_set_halign(title, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(vbox), title, FALSE, FALSE, 20);

    // Image
    GtkWidget *image;
    GdkPixbuf *pixbuf, *scaled_pixbuf;
    image = gtk_image_new_from_file("resources/eol.png");
    pixbuf = gdk_pixbuf_new_from_file("resources/eol.png", NULL);
    scaled_pixbuf = gdk_pixbuf_scale_simple(pixbuf, 200, 200, GDK_INTERP_BILINEAR);
    image = gtk_image_new_from_pixbuf(scaled_pixbuf);
    gtk_box_pack_start(GTK_BOX(vbox), image, TRUE, TRUE, 0);

    // Description
    GtkWidget *description = gtk_label_new(
        "Welcome to the EOL Tester\n\n"
        "Create or load a new workspace\n\n"
        "Current Features:\n"
        "• None\n"
    );
    gtk_label_set_justify(GTK_LABEL(description), GTK_JUSTIFY_CENTER);
    gtk_widget_set_halign(description, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(vbox), description, TRUE, TRUE, 0);

    //Create workspace, Load workspace
      
    g_object_unref(pixbuf); // Clean up the pixbuf
    g_object_unref(scaled_pixbuf);

    return vbox;
}