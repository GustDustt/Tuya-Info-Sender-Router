#include "sys_info.h"


// Global flag del grazaus shutdown
static volatile sig_atomic_t should_exit = 0;

static void signal_handler(int sig) {
    should_exit = 1;
}

bool is_blank(const char *str) {
        if (!str || *str == '\0') {
            return true; 
        }
        while (*str) {
            if (!isspace((unsigned char)*str)) {
                return false;
            }
            str++;
        }
        return true;
}

int main(int argc, char **argv) {
    struct arguments arguments = {NULL, NULL, NULL, 0};
    argp_parse(&argp, argc, argv, 0, 0, &arguments);
    openlog("tuya_daemon", LOG_PID | LOG_CONS, LOG_DAEMON);

    if (is_blank(arguments.device_id) || is_blank(arguments.secret) || is_blank(arguments.product_id)) {
        syslog(LOG_ERR, "Missing required parameters. Exiting.");
        (stderr, "Usage: sys_monitor_daemon -d <device-id> -s <secret> -p <product-id> [-D]\n");
        closelog();
        return EXIT_FAILURE;
    }

    syslog(LOG_INFO, "Starting Tuya system monitor daemon with device_id=%s", arguments.device_id);

    if (arguments.is_daemon) {
        if (daemon(0, 0) < 0) {
            syslog(LOG_ERR, "Failed to daemonize.");
            closelog();
            return EXIT_FAILURE;
        }
        syslog(LOG_INFO, "Running as daemon...");
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    if (connector(&arguments) != OPRT_OK) {
        syslog(LOG_ERR, "Failed to establish Tuya Connection. Exiting.");
        closelog();
        return EXIT_FAILURE;
    }

    CPUData last_cpu;
    get_cpu_data(&last_cpu);

    int report_timer = 0;

    while (!should_exit) {
        int ret = tuya_mqtt_checker();

        sleep(1); 
        report_timer++;

        if (report_timer >= 10) {
            print_system_info(&arguments, report_timer, ret, &last_cpu); 
            report_timer = 0;
        }
    }

    syslog(LOG_INFO, "Tuya system monitor daemon shutting down.");
    tuya_mqtt_disconnect(&client_instance);
    tuya_mqtt_deinit(&client_instance);
    closelog();
    return 0;
}