// #solarmon.h
// srarted on 5/3/2019
#ifndef SOLARMON_H__
#define SOLARMON_H__

#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>

#include <MQTTClient.h>

//#define ADDRESS     "tcp://your_mqtt_broker.local:1883"
#define ADDRESS     "tcp://yo.ur.mq.tt:1883"
#define CLIENTID "SolarmonDashboard"

#define RAW_CMD_TOPIC "homeassistant/raw_command"
#define RAW_CMD_RESP_TOPIC "homeassistant/raw_command/reply"

#define QOS 1
#define TIMEOUT 10000L
#define TOPIC "homeassistant/sensor/#"
#define PAYLOAD "Hello World!"
#define MQTT_TIME_OUT 1000L
#define MAX_MSG_LEN 150
#define debugPrint(...) fprintf(stderr, __VA_ARGS__)
//#define _POSIX_SOURCE 1 /* POSIX compliant source */

static const int enDay = 1;
static const int enMon = 2;
static const int enYear = 3;

static const int TRUE = 1;
static const int FALSE = -1;

static const char mqttTopic[100];
static const char command[][10] = {
    "^P005PI",
    "^P005GS",
    "^P005ET",
    "^P009EY",
    "^P011EM",
    "^P013ED"};

uint16_t cal_crc_half(uint8_t *pin, uint8_t len);
int sendMQTTmessage(MQTTClient client, char *topic, void *msg);
int processRawCmd(char *cmd, MQTTClient mqttClient);
int gui_main(void);
void *mqtt_main(void );

#endif // SOLARMON_H__
// gcc src/solarmon.c src/qpigs.c src/qpiri.c src/communication.c src/qmisc.c  -I./include -lpaho-mqtt3c