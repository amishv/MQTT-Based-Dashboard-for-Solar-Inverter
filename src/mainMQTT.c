#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <locale.h>
#include "solarmon.h"
#include <MQTTClient.h>
#include <gtk/gtk.h>
#include <mqueue.h>
#include <assert.h>

static MQTTClient mqttClient;
//static MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
volatile MQTTClient_deliveryToken deliveredtoken;
static int quit_thread = 0;
void quit_mainMQTT(void);
int mqtt_connect(void );

int msgArrvd(__attribute__((unused)) void *context, char *topicName, int topicLen, MQTTClient_message *message)
{ 
    if (topicLen <0)
        return -1;
    if (strstr(topicName, "/state"))
    {
        char *mqtBuff = (char *)malloc(sizeof(char) * (strlen(topicName) + message->payloadlen + 3));
        if (mqtBuff == NULL)
        {
            perror("Cannot allocate memory");
            return -1;
        }
        sprintf(mqtBuff, "%s %s", topicName, (char *)message->payload);
        puts(mqtBuff);
        mqd_t mqd = mq_open("/SOLARMON_MQ", O_WRONLY, 0600, NULL);
        /* Ensure the creation was successful */
        if (mqd == -1)
        {
            perror("mq_open");
            exit(1);
        }
        /* Send  message with priority 10, then close the queue.
          Note the size is incremented to include the null byte '\0'. */
        mq_send(mqd, mqtBuff, strlen(mqtBuff) + 1, 10);
        //debugPrint("Message : %s \n", mqtBuff);
        mq_close(mqd);
        /* Clean up the allocated memory and message queue */
        if (mqtBuff != NULL)
        {
            free(mqtBuff);
            mqtBuff = NULL;
        }
        else
            fprintf(stderr, "%s:%d > I found a Null buffer in  \a\a\n", __FILE__, __LINE__);
    }
    if (message != NULL)
      MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}
void msgDelivered(__attribute__((unused)) void *context, MQTTClient_deliveryToken dt)
{
    debugPrint("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}
void quit_mainMQTT(void)
{
    quit_thread = 1; // dangerous! But, I am quitting anyway
    mqd_t mqd = mq_open("/SOLARMON_MQ", O_WRONLY, 0600, NULL);
        /* Ensure the creation was successful */
        if (mqd == -1)
        {
            perror("mq_open");
            exit(1);
        }
        /* Send  message with priority 11, then close the queue.
          Note the size is incremented to include the null byte '\0'. */
        mq_send(mqd, "Quit", strlen("Quit") + 1, 11);
        debugPrint("Sending Quit Message \n");
        mq_close(mqd);
}
void connlost(__attribute__((unused)) void *context, char *cause)
{
    debugPrint("\nConnection lost\n");
    debugPrint("     cause: %s\n", cause);
    for ( int i = 0; i <5; i++)
    {
      if (mqtt_connect() == TRUE)
        return; // return if connected
      sleep(60); //wait for one min and retry
    }
    debugPrint("Unable to connect to MQTT server after 5 Attempts! Quitting\n");
    quit_mainMQTT(); //quit if no connection after 5 attempts.
}
int mqtt_connect(void )
{
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int mqttStatus;
    // uint8_t tmpBuf[150];
    if ((mqttStatus = MQTTClient_create(&mqttClient, ADDRESS, CLIENTID,
                                        MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTCLIENT_SUCCESS)
    {
        debugPrint("Failed to create client, return code %d\n", mqttStatus);
        return FALSE;
        //exit(EXIT_FAILURE);
    }
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    MQTTClient_setCallbacks(mqttClient, NULL, connlost, msgArrvd, msgDelivered);
    if ((mqttStatus = MQTTClient_connect(mqttClient, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        debugPrint("Failed to connect, return code %d\n", mqttStatus);
        return FALSE;
        //exit(EXIT_FAILURE);
    }
    debugPrint("Connected Successfully to %s\n", ADDRESS);
    MQTTClient_subscribe(mqttClient, TOPIC, 0);
    debugPrint("Subscribed to topic %s\nfor client %s using QoS %d\n", TOPIC, CLIENTID, 0);
    return TRUE;
}
// int mqtt_main(int argc, char *argv[])
void *mqtt_main(void )
{
    mqtt_connect();
    while (quit_thread != 1)
        {
          sleep(1);
        };
        debugPrint("Quitting the MQTT Thread\n");
        MQTTClient_disconnect(mqttClient, 0);
        MQTTClient_destroy(&mqttClient);
        g_thread_exit(0);
        return 0;
}