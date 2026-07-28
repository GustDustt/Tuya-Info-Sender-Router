#ifndef JSON_FORMATTER_H
#define JSON_FORMATTER_H

#include <cJSON.h>
#include <stdlib.h>
#include "network.h"

void tuya_add_number_prop(cJSON *object, const char *key, double value);
void tuya_add_string_prop(cJSON *object, const char *key, const char *value);
void tuya_add_array_prop(cJSON *object, const char *key, cJSON *array);

char* serialize_system_report(
    unsigned long long total_ram_mb,
    unsigned long long free_ram_mb,
    const char *uptime_str,
    const NetworkData *net_data,
    int net_count,
    double cpu_usage,
    int cpu_status
);

#endif