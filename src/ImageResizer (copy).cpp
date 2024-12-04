#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global widgets
GtkWidget *progress_bar, *combo_box, *text_view, *file_label, *resize_button;

// Data structure for thread
typedef struct {
    gchar *img_path;
    gchar **files;
    int file_count;
} ThreadData;

// Function prototypes
void *resize_images_thread(void *data);
void update_text_view(gpointer text);
void update_progress_bar(gpointer fraction_ptr);

// Function to count .jpg files in the specified directory
int count_jpg_files(const gchar *img_path, gchar ***file_list) {
    GDir *dir;
    const gchar *filename;
    int jpg_count = 0;

    dir = g_dir_open(img_path, 0, NULL);
    if (!dir) {
        g_printerr("Cannot open directory: %s\n", img_path);
        return -1;
    }

    // Dynamically allocate memory for file names
    *file_list = NULL;

    while ((filename = g_dir_read_name(dir)) != NULL) {
        if (g_str_has_suffix(filename, ".jpg") || g_str_has_suffix(filename, ".JPG")) {
            *file_list = (gchar **)g_realloc(*file_list, sizeof(gchar *) * (jpg_count + 1));
            (*file_list)[jpg_count++] = g_strdup(filename);
        }
    }

    g_dir_close(dir);
    return jpg_count;
}

// Function to scale an image
void scale_img(const gchar *img_path, const gchar *img_name) {
    char img_full_path[1024];
    snprintf(img_full_path, sizeof(img_full_path), "%s/%s", img_path, img_name);

    GdkPixbuf *original = gdk_pixbuf_new_from_file(img_full_path, NULL);
    if (!original) {
        g_printerr("Failed to load image: %s\n", img_name);
        return;
    }

    int orgw = gdk_pixbuf_get_width(original);
    int orgh = gdk_pixbuf_get_height(original);
    int newh = 0;

    gint index = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_box));
    if (index == 0) newh = 1200;  // Photograph
    else if (index == 1) newh = 600;  // Article
    else if (index == 2) newh = 200;  // Thumbnail
    else if (index == 3) newh = 50;  // Icon

    int neww = (orgw * newh) / orgh;
    GdkPixbuf *resized = gdk_pixbuf_scale_simple(original, neww, newh, GDK_INTERP_BILINEAR);

    if (resized) {
        char new_img_path[1024];
        snprintf(new_img_path, sizeof(new_img_path), "%s/s_%s", img_path, img_name);
        gdk_pixbuf_save(resized, new_img_path, "jpeg", NULL, "quality", "100", NULL);
        g_object_unref(resized);
    }

    g_object_unref(original);
}

// Thread function
void *resize_images_thread(void *data) {
    ThreadData *thread_data = (ThreadData *)data;
    gchar *img_path = thread_data->img_path;
    gchar **files = thread_data->files;
    int file_count = thread_data->file_count;

    for (int i = 0; i < file_count; ++i) {
        gchar *output = g_strdup_printf("Scaling: %s\n", files[i]);
        g_idle_add((GSourceFunc)update_text_view, output);

        scale_img(img_path, files[i]);

        double fraction = (double)(i + 1) / file_count;
        double *fraction_ptr = (double *)g_malloc(sizeof(double));
        *fraction_ptr = fraction;
        g_idle_add((GSourceFunc)update_progress_bar, fraction_ptr);

        g_free(files[i]); // Free each file name
    }

    g_free(files); // Free the array of file names
    g_free(thread_data->img_path);
    g_free(thread_data);
    return NULL;
}

// GTK UI updates
void update_text_view(gpointer text) {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    gtk_text_buffer_insert_at_cursor(buffer, (gchar *)text, -1);
    g_free(text);
}

void update_progress_bar(gpointer fraction_ptr) {
    double fraction = *(double *)fraction_ptr;
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), fraction);
    g_free(fraction_ptr);
}

// Resize button callback
void on_resize_button_clicked(GtkButton *button, gpointer data) {
    const gchar *img_path = gtk_label_get_text(GTK_LABEL(file_label));
    gchar **file_list = NULL;
    int file_count = count_jpg_files(img_path, &file_list);

    if (file_count <= 0) {
        g_print("No images found in directory: %s\n", img_path);
        return;
    }

    ThreadData *thread_data = (ThreadData *)g_malloc(sizeof(ThreadData));
    thread_data->img_path = g_strdup(img_path);
    thread_data->files = file_list;
    thread_data->file_count = file_count;

    pthread_t thread;
    pthread_create(&thread, NULL, resize_images_thread, thread_data);
    pthread_detach(thread);
}

// Folder selection callback
void on_location_button_clicked(GtkButton *button, gpointer data) {
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Select Directory", NULL,
                                                    GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    "_Open", GTK_RESPONSE_ACCEPT, NULL);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *folder_path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        gtk_label_set_text(GTK_LABEL(file_label), folder_path);
        gtk_widget_set_sensitive(resize_button, TRUE);
        g_free(folder_path);
    }

    gtk_widget_destroy(dialog);
}

// Main function
int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Image Resizer");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 300);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    file_label = gtk_label_new("No folder selected");
    gtk_box_pack_start(GTK_BOX(vbox), file_label, FALSE, FALSE, 0);

    GtkWidget *location_button = gtk_button_new_with_label("Select Folder");
    g_signal_connect(location_button, "clicked", G_CALLBACK(on_location_button_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), location_button, FALSE, FALSE, 0);

    combo_box = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_box), NULL, "Photograph");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_box), NULL, "Article");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_box), NULL, "Thumbnail");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_box), NULL, "Icon");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_box), 0);
    gtk_box_pack_start(GTK_BOX(vbox), combo_box, FALSE, FALSE, 0);

    progress_bar = gtk_progress_bar_new();
    gtk_box_pack_start(GTK_BOX(vbox), progress_bar, FALSE, FALSE, 0);

    text_view = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), text_view);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    resize_button = gtk_button_new_with_label("Resize Images");
    gtk_widget_set_sensitive(resize_button, FALSE);
    g_signal_connect(resize_button, "clicked", G_CALLBACK(on_resize_button_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), resize_button, FALSE, FALSE, 0);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}

