#include "sys_info.h"
#include "json_formatter.h"
#define MAX_INTERFACES 8

tuya_mqtt_context_t client_instance;

int get_cpu_data(CPUData *data) {
    FILE *fp = fopen("/proc/stat", "r");
    if (fp == NULL) return -1;

    int res = fscanf(fp, "cpu  %llu %llu %llu %llu %llu %llu %llu %llu",
                     &data->user, &data->nice, &data->system, &data->idle,
                     &data->iowait, &data->irq, &data->softirq, &data->steal);

    fclose(fp);
    return (res == 8) ? 0 : -1;
}

struct system_ubus_data {
    uint64_t total_ram_bytes;
    uint64_t free_ram_bytes;
    uint64_t uptime;
};

enum {
    SYS_INFO_UPTIME,
    SYS_INFO_MEMORY,
    __SYS_INFO_MAX
};

static const struct blobmsg_policy sys_info_policy[__SYS_INFO_MAX] = {
    [SYS_INFO_UPTIME] = { .name = "uptime", .type = BLOBMSG_TYPE_UNSPEC },
    [SYS_INFO_MEMORY] = { .name = "memory", .type = BLOBMSG_TYPE_TABLE },
};

enum {
    MEM_INFO_TOTAL,
    MEM_INFO_FREE,
    __MEM_INFO_MAX
};

static const struct blobmsg_policy mem_info_policy[__MEM_INFO_MAX] = {
    [MEM_INFO_TOTAL] = { .name = "total", .type = BLOBMSG_TYPE_UNSPEC },
    [MEM_INFO_FREE]  = { .name = "free",  .type = BLOBMSG_TYPE_UNSPEC },
};

static uint64_t get_u64_val(struct blob_attr *attr) {
    if (!attr) return 0;
    if (blobmsg_type(attr) == BLOBMSG_TYPE_INT64) {
        return blobmsg_get_u64(attr);
    } else if (blobmsg_type(attr) == BLOBMSG_TYPE_INT32) {
        return blobmsg_get_u32(attr);
    }
    return 0;
}

static void system_info_cb(struct ubus_request *req, int type, struct blob_attr *msg) {
    struct system_ubus_data *ubus_data = (struct system_ubus_data *)req->priv;
    struct blob_attr *tb[__SYS_INFO_MAX];
    struct blob_attr *memory_tb[__MEM_INFO_MAX];

    blobmsg_parse(sys_info_policy, __SYS_INFO_MAX, tb, blobmsg_data(msg), blobmsg_len(msg));

    if (tb[SYS_INFO_UPTIME]) {
        ubus_data->uptime = get_u64_val(tb[SYS_INFO_UPTIME]);
    }

    if (tb[SYS_INFO_MEMORY]) {
        blobmsg_parse(mem_info_policy, __MEM_INFO_MAX, memory_tb, 
                      blobmsg_data(tb[SYS_INFO_MEMORY]), blobmsg_data_len(tb[SYS_INFO_MEMORY]));

        ubus_data->total_ram_bytes = get_u64_val(memory_tb[MEM_INFO_TOTAL]);
        ubus_data->free_ram_bytes  = get_u64_val(memory_tb[MEM_INFO_FREE]);
    }
}

static int get_system_info_via_ubus(unsigned long long *total_ram_mb, unsigned long long *free_ram_mb, long long *uptime_secs) {
    struct ubus_context *ctx = ubus_connect(NULL);
    if (!ctx) {
        syslog(LOG_ERR, "ubus: Failed to connect to ubus daemon");
        return -1;
    }

    uint32_t id;
    if (ubus_lookup_id(ctx, "system", &id) != 0) {
        syslog(LOG_ERR, "ubus: Failed to locate 'system' object");
        ubus_free(ctx);
        return -1;
    }

    struct system_ubus_data ubus_data = {0};
    int ret = ubus_invoke(ctx, id, "info", NULL, system_info_cb, &ubus_data, 3000);
    if (ret != 0) {
        syslog(LOG_ERR, "ubus: Failed to invoke 'system info' (error: %d)", ret);
        ubus_free(ctx);
        return -1;
    }

    *total_ram_mb = ubus_data.total_ram_bytes / (1024 * 1024);
    *free_ram_mb  = ubus_data.free_ram_bytes / (1024 * 1024);
    *uptime_secs  = ubus_data.uptime;

    ubus_free(ctx);
    return 0;
}

void print_system_info(const struct arguments *arguments, int report_timer, int ret, CPUData *last_cpu) {
    tuya_mqtt_context_t* client = &client_instance;

    unsigned long long total_ram_mb = 0, free_ram_mb = 0;
    long long secs = 0;

    if (get_system_info_via_ubus(&total_ram_mb, &free_ram_mb, &secs) != 0) {
        syslog(LOG_WARNING, "Failed to retrieve system info via ubus, sending partial/zeroed payload");
    }

    char uptime_str[32];
    snprintf(uptime_str, sizeof(uptime_str), "%02lld:%02lld:%02lld", secs / 3600, (secs % 3600) / 60, secs % 60);

    NetworkData net_data[MAX_INTERFACES];
    int net_count = get_network_data(net_data, MAX_INTERFACES);

    CPUData current_cpu;
    double cpu_usage = 0.0;
    int cpu_status = -1;
    if (get_cpu_data(&current_cpu) == 0) {
        unsigned long long total_time1 = last_cpu->user + last_cpu->nice + last_cpu->system + last_cpu->idle + last_cpu->iowait + last_cpu->irq + last_cpu->softirq + last_cpu->steal;
        unsigned long long total_time2 = current_cpu.user + current_cpu.nice + current_cpu.system + current_cpu.idle + current_cpu.iowait + current_cpu.irq + current_cpu.softirq + current_cpu.steal;
        unsigned long long idle1 = last_cpu->idle + last_cpu->iowait;
        unsigned long long idle2 = current_cpu.idle + current_cpu.iowait;

        double total_diff = (double)(total_time2 - total_time1);
        if (total_diff > 0) {
            cpu_usage = ((total_diff - (double)(idle2 - idle1)) / total_diff) * 100.0;
            cpu_status = 0;
        }
        
        *last_cpu = current_cpu;
    }

    char *json_str = serialize_system_report(
        total_ram_mb, free_ram_mb, uptime_str, 
        net_data, net_count, 
        cpu_usage, cpu_status
    );
    
    if (json_str != NULL) {
        syslog(LOG_INFO, "Reporting to Tuya: %s", json_str);
        
        ret = tuyalink_thing_property_report(client, arguments->device_id, json_str);
        if (ret != OPRT_OK) {
            syslog(LOG_ERR, "Failed to send properties to Tuya. Error: %d", ret);
        }

        free(json_str);
    } else {
        syslog(LOG_ERR, "Failed to generate JSON report.");
    }
}