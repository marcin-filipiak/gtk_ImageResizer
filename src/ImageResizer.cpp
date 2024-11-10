#define VERSION "0.1"

#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Widget declarations
GtkWidget *progress_bar, *combo_box, *text_view, *file_label, *resize_button;

// Function to count .jpg files in the specified directory
int count_jpg_files(const gchar *img_path) {
    GDir *dir;
    const gchar *filename;
    int jpg_count = 0;

    // Open the directory
    dir = g_dir_open(img_path, 0, NULL);
    if (!dir) {
        g_printerr("Cannot open directory: %s\n", img_path);
        return -1;
    }

    // Iterate over files in the directory
    while ((filename = g_dir_read_name(dir)) != NULL) {
        // Check if the file has a .jpg or .JPG extension
        if (g_str_has_suffix(filename, ".jpg") || g_str_has_suffix(filename, ".JPG")) {
            jpg_count++;
        }
    }

    // Close the directory
    g_dir_close(dir);

    return jpg_count;
}

// Function to scale an image based on selected size from the combo box
void scale_img(const char *img_path, const char *img_name) {
    char img_full_path[1024];
    snprintf(img_full_path, sizeof(img_full_path), "%s/%s", img_path, img_name);

    // Load the image
    GdkPixbuf *original = gdk_pixbuf_new_from_file(img_full_path, NULL);
    if (!original) {
        g_print("Failed to load image: %s\n", img_name);
        return;
    }

    int orgw = gdk_pixbuf_get_width(original);
    int orgh = gdk_pixbuf_get_height(original);
    int newh = 0;

    // Determine the new height based on combo box selection
    gint index = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_box));
    if (index == 0) newh = 1200;  // Photograph
    else if (index == 1) newh = 600;  // Article
    else if (index == 2) newh = 200;  // Thumbnail
    else if (index == 3) newh = 50;  // Icon

    int neww = (orgw * newh) / orgh;

    // Resize the image
    GdkPixbuf *resized = gdk_pixbuf_scale_simple(original, neww, newh, GDK_INTERP_BILINEAR);

    // Save the resized image
    char new_img_path[1024];
    snprintf(new_img_path, sizeof(new_img_path), "%s/s_%s", img_path, img_name);
    gdk_pixbuf_save(resized, new_img_path, "jpeg", NULL, "quality", "100", NULL);

    // Clean up
    g_object_unref(original);
    g_object_unref(resized);
}

// Callback function for the Resize button
void on_resize_button_clicked(GtkButton *button, gpointer data) {
    const gchar *img_path = gtk_label_get_text(GTK_LABEL(file_label));
    GDir *dir = g_dir_open(img_path, 0, NULL);
    if (!dir) {
        g_print("Failed to open directory: %s\n", img_path);
        return;
    }
    
    int iCountedImages = count_jpg_files(img_path);
    //g_printerr("Number of images: %d\n", count_jpg_files(img_path));

    const gchar *file;
    int i = 0;
    while ((file = g_dir_read_name(dir)) != NULL) {
        if (g_str_has_suffix(file, ".jpg") || g_str_has_suffix(file, ".JPG")) {
            gchar *output = g_strdup_printf("Scaling: %s\n", file);
            GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
            gtk_text_buffer_insert_at_cursor(buffer, output, -1);
            g_free(output);

            scale_img(img_path, file);

            // Update progress bar
            gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), (double)(i + 1) / iCountedImages); 
        }
        i++;
    }
    g_dir_close(dir);
}

// Callback function for the folder selection button
void on_location_button_clicked(GtkButton *button, gpointer data) {
    GtkWidget *dialog;
    dialog = gtk_file_chooser_dialog_new("Select Directory",
                                         NULL,
                                         GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                         ("_Cancel"), GTK_RESPONSE_CANCEL,
                                         ("_Open"), GTK_RESPONSE_ACCEPT,
                                         NULL);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *folder_path;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        folder_path = gtk_file_chooser_get_filename(chooser);

        gtk_label_set_text(GTK_LABEL(file_label), folder_path);
        gtk_widget_set_sensitive(resize_button, TRUE);

        g_free(folder_path);
    }

    gtk_widget_destroy(dialog);
}

// Main function
int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    // Create main window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Image Resizer");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 300);

    // Layout setup
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Folder label and selection button
    file_label = gtk_label_new("No folder selected");
    gtk_box_pack_start(GTK_BOX(vbox), file_label, FALSE, FALSE, 0);

    GtkWidget *location_button = gtk_button_new_with_label("Select Folder");
    g_signal_connect(location_button, "clicked", G_CALLBACK(on_location_button_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), location_button, FALSE, FALSE, 0);

    // Combo box for selecting the size category
    combo_box = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_box), NULL, "Photograph");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_box), NULL, "Article");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_box), NULL, "Thumbnail");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_box), NULL, "Icon");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_box), 0);
    gtk_box_pack_start(GTK_BOX(vbox), combo_box, FALSE, FALSE, 0);

    // Progress bar
    progress_bar = gtk_progress_bar_new();
    gtk_box_pack_start(GTK_BOX(vbox), progress_bar, FALSE, FALSE, 0);

    // Text view for log messages
    text_view = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), text_view);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    // Resize button
    resize_button = gtk_button_new_with_label("Resize Images");
    gtk_widget_set_sensitive(resize_button, FALSE);
    g_signal_connect(resize_button, "clicked", G_CALLBACK(on_resize_button_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), resize_button, FALSE, FALSE, 0);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}

