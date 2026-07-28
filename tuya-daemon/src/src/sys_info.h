#ifndef SYS_INFO_H
#define SYS_INFO_H

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE  
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

// Tuya nustatymai ir headers
#include "tuya_log.h"
#include "tuya_cacert.h"
#include "tuya_endpoint.h"
#include "tuya_error_code.h"
#include "system_interface.h"
#include "mqtt_client_interface.h"
#include "tuyalink_core.h"

#include <unistd.h>
#include <time.h>
#include <sys/sysinfo.h>
#include <argp.h>     
#include <libubox/blobmsg_json.h>
#include <libubus.h>
#include <syslog.h> 

// Undefine tuos macros kurie gali conflicts sukelti
#undef LOG_DEBUG
#undef LOG_INFO
#undef LOG_WARN
#undef LOG_ERROR

#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <assert.h>

extern tuya_mqtt_context_t client_instance;

// CPU datos sekimas
typedef struct {
    unsigned long long user;
    unsigned long long nice;
    unsigned long long system;
    unsigned long long idle;
    unsigned long long iowait;
    unsigned long long irq;
    unsigned long long softirq;
    unsigned long long steal;
} CPUData;

struct arguments {
    char *device_id;
    char *secret;
    char *product_id;
    int is_daemon;
};

static char doc[] = "Tuya IoT Cloud System Monitoring Daemon Process";
static struct argp_option options[] = {
    {"device-id",  'd', "ID",     0, "Tuya Device ID", 0},
    {"secret",     's', "SECRET", 0, "Tuya Device Secret", 0},
    {"product-id", 'p', "PID",    0, "Tuya Product ID", 0},
    {"daemon",     'D', 0,        0, "Run program as a daemon process", 0},
    { 0 }
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = state->input;
    switch (key) {
        case 'd': arguments->device_id = arg; break;
        case 's': arguments->secret = arg; break;
        case 'p': arguments->product_id = arg; break;
        case 'D': arguments->is_daemon = 1; break;
        default: return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = { options, parse_opt, 0, doc, 0, 0, 0 };

int get_cpu_data(CPUData *data);
void on_connected(tuya_mqtt_context_t* context, void* user_data);
void on_disconnect(tuya_mqtt_context_t* context, void* user_data);
void on_messages(tuya_mqtt_context_t* context, void* user_data, const tuyalink_message_t* msg);
static error_t parse_opt(int key, char *arg, struct argp_state *state);
int connector(const struct arguments *arguments);
void print_system_info(const struct arguments *arguments, int report_timer, int ret, CPUData *last_cpu);
int tuya_mqtt_checker();

#endif