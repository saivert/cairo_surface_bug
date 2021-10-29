#include <cairo.h>
#include <cairo/cairo.h>
#include <gtk/gtk.h>

typedef struct {
    float x, y;
} coord_t;

static void do_drawing(cairo_t *);

static int workaround;

static gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, 
    gpointer user_data)
{

    if (workaround) {
        cairo_surface_t *surface;
        cairo_surface_t *imagesurface;
        cairo_t *imagecr;

        surface = cairo_get_target (cr);
        imagesurface = cairo_surface_map_to_image (surface, NULL);

        imagecr = cairo_create (imagesurface);

        do_drawing(imagecr);

        cairo_destroy (imagecr);

        cairo_surface_unmap_image(surface, imagesurface);

    } else {
        do_drawing(cr);
    }

    return FALSE;
}

static void
cairo_draw_lines (cairo_t *cr, coord_t *pts, int cnt) {
    cairo_move_to (cr, pts[0].x+1, pts[0].y+1);
    for (int i = 1; i < cnt; i++) {
        cairo_line_to (cr, pts[i].x+1, pts[i].y+1);
    }
}

static void
cairo_draw_poly (cairo_t *cr, coord_t *pts, int cnt) {
    cairo_move_to (cr, pts[0].x, pts[0].y);
    for (int i = 1; i < cnt; i++) {
        cairo_line_to (cr, pts[i].x, pts[i].y);
    }
}

ddb_tabstrip_draw_tab (cairo_t *cr, int x, int y, int w, int h) {

    coord_t points_filled[] = {
        { x+2, y + h },
        { x+2, y + 2 },
        { x + w - h + 1, y + 2 },
        { x + w - 1 + 1, y + h }
    };
    coord_t points_frame1[] = {
        { x, y + h-2 },
        { x, y + 0.5 },
        { x + 0.5, y },
        { x + w - h - 1, y },
        { x + w - h + 1, y + 1 },
        { x + w - 3, y + h - 3 },
        { x + w - 0, y + h - 2 },
    };
    coord_t points_frame2[] = {
        { x + 1, y + h -1 },
        { x + 1, y + 1 },
        { x + w - h - 1, y + 1 },
        { x + w - h + 1, y + 2 },
        { x + w - 3, y + h - 2 },
        { x + w - 0, y + h - 1 },
    };

    GdkColor clr_bg;
    GdkColor clr_outer_frame;
    GdkColor clr_inner_frame;
    gdk_color_parse ("#CECECE", &clr_bg);
    gdk_color_parse ("#DDDDDD", &clr_outer_frame);
    gdk_color_parse ("#808080", &clr_inner_frame);

    cairo_save(cr);

    GdkRGBA bg;
    gdk_rgba_parse(&bg, "CECECE");

    gdk_cairo_set_source_rgba(cr, &bg);
    // cairo_set_source_rgb (cr, clr_bg.red/65535.f, clr_bg.green/65535.f, clr_bg.blue/65535.0);
    cairo_new_path (cr);
    cairo_draw_poly (cr, points_filled, sizeof (points_filled)/sizeof(coord_t));
    cairo_close_path (cr);
    cairo_fill (cr);

    cairo_set_source_rgb (cr, clr_outer_frame.red/65535.f, clr_outer_frame.green/65535.f, clr_outer_frame.blue/65535.0);
    cairo_draw_lines (cr, points_frame1, sizeof (points_frame1)/sizeof(coord_t));
    cairo_stroke (cr);
    cairo_set_source_rgb (cr, clr_inner_frame.red/65535.f, clr_inner_frame.green/65535.f, clr_inner_frame.blue/65535.0);
    cairo_draw_lines (cr, points_frame2, sizeof (points_frame2)/sizeof(coord_t));
    cairo_stroke (cr);

    cairo_restore(cr);
}

static void do_drawing(cairo_t *cr)
{
  cairo_select_font_face(cr, "DejaVu Sans",
      CAIRO_FONT_SLANT_NORMAL,
      CAIRO_FONT_WEIGHT_BOLD);

  cairo_set_font_size(cr, 14);
  cairo_scale(cr, 2, 2);

  cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
  // cairo_set_source_rgb(cr, 0, 0, 0);
  // cairo_set_line_width(cr, 0.5);
  // cairo_move_to(cr, -200000, 300);
  // cairo_line_to(cr, 200000, 0);
  // cairo_stroke(cr);

  cairo_move_to(cr, 1, 10);
  cairo_set_source_rgb(cr, 0, 0, 0);
  cairo_show_text(cr, "Antialiasing set to NONE");  

  ddb_tabstrip_draw_tab (cr, 1, 20, 40, 18);

  cairo_set_antialias (cr, CAIRO_ANTIALIAS_DEFAULT);

  cairo_move_to(cr, 1, 60);
  cairo_set_source_rgb(cr, 0, 0, 0);
  cairo_show_text(cr, "Antialiasing set to DEFAULT");  

  ddb_tabstrip_draw_tab (cr, 1, 80, 40, 18);


  cairo_move_to(cr, 1, 120);
  cairo_set_source_rgb(cr, 0, 0, 0);
  char string[255];
  snprintf(string, sizeof(string)-1, "Surface type is %s", workaround ? "image" : "similar");
  cairo_show_text(cr, string);  

}

GtkWidget *darea;

gboolean button_release_event(GtkWidget *self, GdkEventButton *event,
                           gpointer user_data)
{

    if (event->button == 1) {
        workaround = workaround ? 0 : 1;
        gtk_widget_queue_draw(darea);
    }
    return FALSE;
}

int main(int argc, char *argv[])
{
  GtkWidget *window;
  
  workaround = 0;
  
  gtk_init(&argc, &argv);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  darea = gtk_drawing_area_new();
  gtk_container_add(GTK_CONTAINER(window), darea);
 
  gtk_widget_add_events(darea, GDK_BUTTON_PRESS_MASK|GDK_BUTTON_RELEASE_MASK);
  gtk_widget_set_can_focus(darea, TRUE);
 
  g_signal_connect(G_OBJECT(darea), "draw", 
      G_CALLBACK(on_draw_event), NULL); 

  g_signal_connect(window, "destroy",
      G_CALLBACK(gtk_main_quit), NULL);  

  g_signal_connect(G_OBJECT(darea), "button-release-event", 
      G_CALLBACK(button_release_event), NULL); 

  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(window), 500,500); 
  gtk_window_set_title(GTK_WINDOW(window), "Antialiasing bug demo");

  gtk_widget_show_all(window);

  

  gtk_main();

  return 0;
}
