
#include <stdlib.h>
#include <math.h>
#include <gtk/gtk.h>
#include <mqueue.h>
#include <assert.h>

int gui_main(void);
void *mqtt_main(void *);
void quit_mainMQTT(void);
gboolean time_handler(gpointer *);

GtkLabel *gridVoltage, *gridFreq, *inverterVoltage, *inverterFreq;
GtkLabel *pvPwrDisp, *pvVoltDisp, *mpptTempDisp, *invTempDisp;
GtkLabel *battVolts, *battCurrent, *InverterMode;
GtkLabel *loadAct, *loadApparent, *gridState;
GtkDrawingArea *pvPwr, *pvVolts,  *mpptTemp, *inverterTemp;
GtkAdjustment *pvPwrVal, *pvVoltVal , *mpptTempVal , *invTempVal;
GtkScale *pvPwrScale;
GtkProgressBar *battPercent, *loadPercent;
GtkButton *quit_button;
// This just causes the drawing area to be redrawn when the slider is moved.
// user_data is a pointer to drawingarea1
void on_adjustment_value_changed(__attribute__((unused)) GtkRange *range,
			     gpointer  user_data)
{
    gtk_widget_queue_draw(GTK_WIDGET(user_data));
}
// Handle the user trying to close the window
gboolean windowDelete(__attribute__((unused)) GtkWidget *widget,
                      __attribute__((unused)) GdkEvent *event,
                      __attribute__((unused)) gpointer data)
{
    g_print("%s called.\n", __func__ );
    quit_mainMQTT();
    //g_application_quit (data);
    return FALSE; // Returning TRUE stops the window being deleted.
                  // Returning FALSE allows deletion.
}
//handle quit_button
void quitButtonClicked (__attribute__((unused))GtkWidget* widget, gpointer data)
{
    g_print("Quit Clicked\n");
    g_application_quit(G_APPLICATION(data));
}
// Widget and app initialization
static void activate (GtkApplication* app, gpointer user_data)
{
// Stop GCC to report the instance of all the initialised variable as error
#pragma GCC diagnostic ignored "-Wunused-but-set-variable" 
    GtkWidget *window;
    GtkBuilder *builder = NULL;
    builder = gtk_builder_new();
    if (gtk_builder_add_from_file(builder, "SolarDashboard.glade", NULL) == 0)
    {
        perror("gtk_builder_add_from_file FAILED\n");
        return;
    }
    window = GTK_WIDGET(gtk_builder_get_object(builder, "mainWindow"));
    gtk_window_set_application (GTK_WINDOW (window), GTK_APPLICATION (app));
    gridVoltage = GTK_LABEL(gtk_builder_get_object(builder, "gridVoltage"));
    gridFreq = GTK_LABEL(gtk_builder_get_object(builder, "gridFreq"));
    inverterVoltage = GTK_LABEL(gtk_builder_get_object(builder, "inverterVoltage"));
    inverterFreq = GTK_LABEL(gtk_builder_get_object(builder, "inverterFreq"));
    pvPwr = GTK_DRAWING_AREA(gtk_builder_get_object(builder, "pvPwr"));
    pvPwrVal = GTK_ADJUSTMENT(gtk_builder_get_object(builder, "pvPwrVal"));
    pvPwrDisp = GTK_LABEL(gtk_builder_get_object(builder, "pvPwrDisp"));
    pvVolts = GTK_DRAWING_AREA(gtk_builder_get_object(builder, "pvVolts"));
    pvVoltVal = GTK_ADJUSTMENT(gtk_builder_get_object(builder, "pvVoltVal"));
    pvVoltDisp = GTK_LABEL(gtk_builder_get_object(builder, "pvVoltDisp"));
    mpptTemp = GTK_DRAWING_AREA(gtk_builder_get_object(builder, "mpptTemp"));
    mpptTempVal = GTK_ADJUSTMENT(gtk_builder_get_object(builder, "mpptTempVal"));
    mpptTempDisp = GTK_LABEL(gtk_builder_get_object(builder, "mpptTempDisp"));
    inverterTemp = GTK_DRAWING_AREA(gtk_builder_get_object(builder, "inverterTemp"));
    invTempVal = GTK_ADJUSTMENT(gtk_builder_get_object(builder, "invTempVal"));
    invTempDisp = GTK_LABEL(gtk_builder_get_object(builder, "invTempDisp"));
    battVolts = GTK_LABEL(gtk_builder_get_object(builder, "battVolts"));
    battCurrent = GTK_LABEL(gtk_builder_get_object(builder, "battCurrent"));
    InverterMode = GTK_LABEL(gtk_builder_get_object(builder, "InverterMode"));
    battPercent = GTK_PROGRESS_BAR(gtk_builder_get_object(builder, "battPercent"));
    loadPercent = GTK_PROGRESS_BAR(gtk_builder_get_object(builder, "loadPercent"));
    loadAct = GTK_LABEL(gtk_builder_get_object(builder, "loadAct"));
    loadApparent = GTK_LABEL(gtk_builder_get_object(builder, "loadApparent"));
    gridState = GTK_LABEL(gtk_builder_get_object(builder, "gridState"));
    quit_button = GTK_BUTTON (gtk_builder_get_object(builder, "quit_button"));
#pragma GCC diagnostic pop
    g_print("value set\n");
    gtk_builder_connect_signals(builder, NULL);
    g_object_unref(G_OBJECT(builder));
    //////////////// begin CSS
    GError *error;
    GtkCssProvider *provider = gtk_css_provider_new ();
    GdkDisplay *display = gdk_display_get_default ();	  
    GdkScreen *screen = gdk_display_get_default_screen (display);
    //context = gtk_widget_get_style_context(window);
    gtk_style_context_add_provider_for_screen (screen,
                                    GTK_STYLE_PROVIDER(provider),
                                    GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    if (!gtk_css_provider_load_from_path (provider, "./SolarDashboard.css", &error))
    {
        perror( "load css failed" );
        g_free( error );
        return;
    }
    //g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy), app);
    g_signal_connect(G_OBJECT(quit_button), "clicked", G_CALLBACK(quitButtonClicked), app);
    //////////////////// end CSS
    // gtk_widget_show_all(window); //not needed as  marked visible in xml
    g_thread_new("mqtt_worker", (GThreadFunc )mqtt_main, NULL);
    g_print("\n started the Worker thread\n");
    g_idle_add((GSourceFunc)time_handler, (gpointer)user_data);
    return ;
}

int main(int argc, char *argv[]){
  GtkApplication *app;
  int status;
  //Open the message queue for reading
  mqd_t mqd = mq_open("/SOLARMON_MQ", O_CREAT | O_RDONLY | O_NONBLOCK, 0600, NULL);
  assert(mqd != -1);
  //gtk_init(&argc, &argv);
  app = gtk_application_new ("in.vuaie.solardashboard", G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK (activate), (gpointer) &mqd);

  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);
  mq_close(mqd);
  return status;
}
// end main.c