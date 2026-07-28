#include "json_formatter.h"

void tuya_add_number_prop(cJSON *object, const char *key, double value) {
    cJSON *prop = cJSON_CreateObject();
    if (prop) {
        cJSON_AddNumberToObject(prop, "value", value);
        cJSON_AddItemToObject(object, key, prop);
    }
}

void tuya_add_string_prop(cJSON *object, const char *key, const char *value) {
    cJSON *prop = cJSON_CreateObject();
    if (prop) {
        cJSON_AddStringToObject(prop, "value", value);
        cJSON_AddItemToObject(object, key, prop);
    }
}

void tuya_add_array_prop(cJSON *object, const char *key, cJSON *array) {
    cJSON *prop = cJSON_CreateObject();
    if (prop && array) {
        cJSON_AddItemToObject(prop, "value", array);
        cJSON_AddItemToObject(object, key, prop);
    }
}

char* serialize_system_report(
    unsigned long long total_ram_mb,
    unsigned long long free_ram_mb,
    const char *uptime_str,
    const NetworkData *net_data,
    int net_count,
    double cpu_usage,
    int cpu_status
) {
    cJSON *root = cJSON_CreateObject();
    if (!root) return NULL;

    cJSON *ram_obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(ram_obj, "value", (double)total_ram_mb);
    cJSON_AddItemToObject(root, "total_ram", ram_obj);

    cJSON *free_ram_obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(free_ram_obj, "value", (double)free_ram_mb);
    cJSON_AddItemToObject(root, "free_ram", free_ram_obj);

    cJSON *uptime_obj = cJSON_CreateObject();
    cJSON_AddStringToObject(uptime_obj, "value", uptime_str);
    cJSON_AddItemToObject(root, "uptime", uptime_obj);

    cJSON *cpu_obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(cpu_obj, "value", cpu_usage);
    cJSON_AddItemToObject(root, "cpu_usage", cpu_obj);

    cJSON *interfaces_arr = cJSON_CreateArray();
    for (int i = 0; i < net_count; i++) {
        cJSON *temp_interface = cJSON_CreateObject();
        if (temp_interface) {
            cJSON_AddStringToObject(temp_interface, "interface", net_data[i].name);
            cJSON_AddStringToObject(temp_interface, "ip_addr", net_data[i].ip);
            cJSON_AddStringToObject(temp_interface, "netmask", net_data[i].netmask);
            
            double rx_mb = (double)net_data[i].rx_bytes / (1024.0 * 1024.0);
            double tx_mb = (double)net_data[i].tx_bytes / (1024.0 * 1024.0);
            cJSON_AddNumberToObject(temp_interface, "rx_mb", rx_mb);
            cJSON_AddNumberToObject(temp_interface, "tx_mb", tx_mb);

            char *serialized_interface_str = cJSON_PrintUnformatted(temp_interface);
            if (serialized_interface_str) {
                cJSON_AddItemToArray(interfaces_arr, cJSON_CreateString(serialized_interface_str));
                free(serialized_interface_str);
            }
            cJSON_Delete(temp_interface);
        }
    }

    char *serialized_array_str = cJSON_PrintUnformatted(interfaces_arr);
    cJSON_Delete(interfaces_arr);

    cJSON *interfaces_obj = cJSON_CreateObject();
    if (serialized_array_str) {
        cJSON_AddStringToObject(interfaces_obj, "value", serialized_array_str);
        free(serialized_array_str);
    } else {
        cJSON_AddStringToObject(interfaces_obj, "value", "[]");
    }
    cJSON_AddItemToObject(root, "interfaces", interfaces_obj);

    char *json_out = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    return json_out;
}