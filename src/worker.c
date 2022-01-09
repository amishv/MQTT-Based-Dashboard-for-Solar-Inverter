#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
//#include <pthread.h>
#include <stdlib.h>
#include <math.h>
#include <mqueue.h>
#include <assert.h>

gboolean setWidgetVal(GtkWidget *widget, double val);
extern GtkApplication *app;
extern GThread *mqttThread;
extern GtkLabel *gridVoltage, *gridFreq, *inverterVoltage, *inverterFreq;
extern GtkLabel *pvPwrDisp, *pvVoltDisp, *mpptTempDisp, *invTempDisp;
extern GtkLabel *battVolts, *battCurrent, *InverterMode;
extern GtkLabel *loadAct, *loadApparent, *gridState;
extern GtkDrawingArea *pvPwr, *pvVolts, *mpptTemp, *inverterTemp;
extern GtkAdjustment *pvPwrVal, *pvVoltVal, *mpptTempVal, *invTempVal;
extern GtkScale *pvPwrScale;
extern GtkProgressBar *battPercent, *loadPercent;

gboolean
time_handler(gpointer *mqd_Ptr)
{
    mqd_t *mqd = (mqd_t *)mqd_Ptr;
    struct mq_attr attr = {0l, 0l, 0l, 0l, {0l, 0l, 0l, 0l}};
    assert(mq_getattr(*mqd, &attr) != -1);
    char *buffer = calloc(attr.mq_msgsize, 1);
    assert(buffer != NULL);
    /* Retrieve message from the queue and get its priority level */
    unsigned int priority = 0;
    if ((mq_receive(*mqd, buffer, attr.mq_msgsize, &priority)) != -1)
    {
        if ((priority == 11) && (buffer!=NULL))
        {
            g_print("Send Destroy Signal :buffer -> %s\n", buffer);
            g_application_quit(G_APPLICATION(app));
        }
        else
        {
            char *topic = NULL;
            char *payload = NULL;
            topic = strtok(buffer, " ");
            payload = strtok(NULL, " ");
            char *ptr;
            double ret;
            ret = strtod(payload, &ptr);
            if (strstr(topic, "AC_grid_voltage/state"))
            {
                gtk_label_set_label(gridVoltage, payload);
            }
            else if (strstr(topic, "AC_grid_frequency/state"))
            {
                gtk_label_set_label(gridFreq, payload);
            }
            else if (strstr(topic, "AC_out_voltage/state"))
            {
                gtk_label_set_label(inverterVoltage, payload);
            }
            else if (strstr(topic, "AC_out_frequency/state"))
                gtk_label_set_label(inverterFreq, payload);
            else if (strstr(topic, "PV_in_power/state"))
            {
                // gtk_label_set_label(pvPwrDisp, payload);
                gtk_adjustment_set_value(pvPwrVal, ret);
            }
            else if (strstr(topic, "PV_in_voltage/state"))
            {
                // gtk_label_set_label(pvVoltDisp, payload);
                gtk_adjustment_set_value(pvVoltVal, ret);
            }
            else if (strstr(topic, "MPPT_temperature/state"))
            {
                // gtk_label_set_label(mpptTempDisp, payload);
                gtk_adjustment_set_value(mpptTempVal, ret);
            }
            else if (strstr(topic, "Heatsink_temperature/state"))
            {
                // gtk_label_set_label(invTempDisp, payload);
                gtk_adjustment_set_value(invTempVal, ret);
            }
            else if (strstr(topic, "Battery_capacity/state"))
            {
                gtk_progress_bar_set_fraction(battPercent, ret / 100);
            }
            else if (strstr(topic, "Battery_voltage/state"))
            {
                gtk_label_set_label(battVolts, payload);
            }
            else if (strstr(topic, "Battery_charge_current/state"))
            {
                gtk_label_set_label(battCurrent, payload);
                if (ret > 0)
                    gtk_widget_set_name(GTK_WIDGET(battCurrent), "label_green");
                else
                    gtk_widget_set_name(GTK_WIDGET(battCurrent), " ");
            }
            else if (strstr(topic, "Battery_discharge_current/state"))
            {
                gtk_label_set_label(battCurrent, payload);
                if (ret > 0) // I rely on receiving charge current before discharge current
                    gtk_widget_set_name(GTK_WIDGET(battCurrent), "label_red");
            }
            else if (strstr(topic, "Load_watt/state"))
            {
                gtk_label_set_label(loadAct, payload);
            }
            else if (strstr(topic, "Load_va/state"))
            {
                gtk_label_set_label(loadApparent, payload);
            }
            else if (strstr(topic, "Load_pct/state"))
            {
                gtk_progress_bar_set_fraction(loadPercent, ret / 100);
            }
            else if (strstr(topic, "Inverter_mode/state"))
            {
                gtk_label_set_label(InverterMode, payload);
                if (strstr(payload, "Hybrid"))
                    gtk_widget_set_name(GTK_WIDGET(InverterMode), "label_green");
                else if (strstr(payload, "Battery"))
                    gtk_widget_set_name(GTK_WIDGET(InverterMode), "label_yellow");
                else
                    gtk_widget_set_name(GTK_WIDGET(InverterMode), "label_red");
            }
            else if (strstr(topic, "AC_Power_dir/state"))
            {
                gtk_label_set_label(gridState, ptr);
                if (strstr(payload, "Output"))
                    gtk_widget_set_name(GTK_WIDGET(gridState), "label_green");
                else if (strstr(payload, "Input"))
                    gtk_widget_set_name(GTK_WIDGET(gridState), "label_yellow");
                else
                    gtk_widget_set_name(GTK_WIDGET(gridState), "label_red");
            }
            topic = NULL; // dont leave the pointer dangling
            payload = NULL;
            mqd = NULL;
        }
    }
    /* Clean up the allocated memory and message queue */
    if (buffer != NULL)
    {
        free(buffer);
        buffer = NULL;
    }
    else
        fprintf(stderr, "%s:%d > I found a Null buffer in  \a\a\n", __FILE__, __LINE__);
    /* Return true so the function will be called again; returning false removes
     * this timeout function.
     */
    return TRUE;
}
// end worker.c