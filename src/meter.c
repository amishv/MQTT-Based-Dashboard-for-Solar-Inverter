#include <stdlib.h>
#include <math.h>
#include <gtk/gtk.h>
#define ANGLE_RAD(x) (M_PI / 180.) * x
#define Y_OFFSET 0.0 // Offset in pixels to height from the base
GtkAdjustment *pvPwrVal, *pvVoltVal, *mpptTempVal, *invTempVal;
// GtkScale *pvPwrScale;
//  This just causes the drawing area to be redrawn when the slider is moved.
//  user_data is a pointer to drawingarea1
void on_scale_value_changed(__attribute__((unused)) GtkRange *range,
                            gpointer user_data)
{

    gtk_widget_queue_draw(GTK_WIDGET(user_data));
}

// user_data is a pointer to adjustment
gboolean on_drawingarea_draw(GtkWidget *widget,
                             cairo_t *cr,
                             gpointer user_data)
{
    if ((user_data == NULL) || widget == NULL)
        return FALSE;
    char text[10];

    static PangoLayout *layout = NULL; // Retained between calls
    GtkAllocation winRect;
    int width, height, baseline;
    gdouble value, limHi, limLo, percentVal;
    // Retrieve the ad slider value
    value = gtk_adjustment_get_value(GTK_ADJUSTMENT(user_data));
    limHi = gtk_adjustment_get_upper(GTK_ADJUSTMENT(user_data));
    limLo = gtk_adjustment_get_lower(GTK_ADJUSTMENT(user_data));
    percentVal = (value - limLo) / (limHi - limLo);
    //  Do some pango setup on the first call.
    cairo_save(cr);
    if (layout == NULL)
    {
        PangoFontDescription *desc;
        layout = pango_cairo_create_layout(cr);
        desc = pango_font_description_from_string("Sans Bold 15");
        pango_layout_set_font_description(layout, desc);
        pango_font_description_free(desc);
    }
    if (widget != NULL)
    {
        gtk_widget_get_allocated_size(widget, &winRect, &baseline);
       // g_print("widget size is %d,%d,%d,%d \n", winRect.x, winRect.y, winRect.height, winRect.width);
    }
    else
    {
        g_print("\n I should never reach here!\nNull widget in on_drawingarea_draw\n ");
        cairo_restore(cr);
        return FALSE;
    }
    double radius;
    if (winRect.height > winRect.width / 2) // meter will be semi circle
        radius = (0.4) * winRect.width;       // 40% of the window width
    else                                    // on the negative axis
        radius = (0.5) * winRect.height;
    //g_print(" radius = %lf\n\n", radius);
    cairo_set_source_rgb(cr, 0, 1, 0);
    cairo_set_line_width(cr, 10.0);
    // Set origin as middle of bottom edge of the drawing area.
    cairo_translate(cr, (double)winRect.width/(2.0),(double) winRect.height*(1.0));
    // Set the text to print
    sprintf(text, "%.0f", value);
    pango_layout_set_text(layout, text, -1);
    // Redo layout for new text
    pango_cairo_update_layout(cr, layout);
    // Retrieve the size of the rendered text
    pango_layout_get_size(layout, &width, &height);
    cairo_move_to(cr, 0.0 - (double)(width / PANGO_SCALE) / 2.0, 
                  0.0 - 0.7 * radius);//(winRect.height/2.5 )); //+ (double)height / PANGO_SCALE - Y_OFFSET));
    // Render the text
    pango_cairo_show_layout(cr, layout);
    cairo_stroke(cr);
    // Draw the meter
    //  Set size and colour of the pointer
    cairo_set_source_rgb(cr, 0, 1.0, 0);
    cairo_set_line_width(cr, 20.5);
    cairo_arc(cr, 0.00, 0.0, radius, ANGLE_RAD(180), ANGLE_RAD(180 + percentVal * M_PI));
    cairo_stroke(cr);
    cairo_set_source_rgb(cr, 0.005, 0.06,0.06); // complete the remaining arc invisible
    cairo_arc(cr, 0.0, 0.0, radius, ANGLE_RAD(180 + percentVal * M_PI), ANGLE_RAD(360));
    cairo_stroke(cr);
    //  Draw the meter pointer
    cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
    cairo_set_line_width(cr, 2.5);
    cairo_move_to(cr, 0.0, 0.0);
    cairo_line_to(cr, -0.8 *radius* cos(M_PI * percentVal), -0.8*radius * sin(M_PI * percentVal));
    cairo_stroke(cr);
    cairo_restore(cr); // restore the original context
    return FALSE;
}
