#include "network.h"

int get_net_bytes(const char *iface, unsigned long long *rx, unsigned long long *tx) {
    FILE *fp = fopen("/proc/net/dev", "r");
    if (!fp) return -1;

    char line[256];
    if (!fgets(line, sizeof(line), fp) || !fgets(line, sizeof(line), fp)) {
        fclose(fp);
        return -1;
    }

    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, iface)) {
            char *colon = strchr(line, ':');
            if (colon) {
                unsigned long long dummy;
                int parsed = sscanf(colon + 1, "%llu %llu %llu %llu %llu %llu %llu %llu %llu",
                                    rx, &dummy, &dummy, &dummy, &dummy, &dummy, &dummy, &dummy, tx);
                if (parsed >= 9) {
                    fclose(fp);
                    return 0;
                }
            }
        }
    }
    fclose(fp);
    return -1;
}

int get_network_data(NetworkData *data_arr, int max_interfaces) {
    struct ifaddrs *ifaddr, *ifa;
    int count = 0;

    if (getifaddrs(&ifaddr) == -1) return -1;

    for (ifa = ifaddr; ifa != NULL && count < max_interfaces; ifa = ifa->ifa_next) {
        // Skip null interface names
        if (!ifa->ifa_name) continue;

        // Skip loopback interface to keep logs clean, or remove this check if you want 'lo' too!
        if (ifa->ifa_flags & IFF_LOOPBACK) continue;

        // To avoid duplicate entries for the same interface (since getifaddrs returns entries for IPv4, IPv6, Packet, etc.)
        int already_added = 0;
        for (int i = 0; i < count; i++) {
            if (strcmp(data_arr[i].name, ifa->ifa_name) == 0) {
                already_added = 1;
                
                // If we already added it but this pass has the IPv4 address, update it!
                if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET) {
                    struct sockaddr_in *sa = (struct sockaddr_in *)ifa->ifa_addr;
                    inet_ntop(AF_INET, &sa->sin_addr, data_arr[i].ip, INET_ADDRSTRLEN);

                    struct sockaddr_in *nm = (struct sockaddr_in *)ifa->ifa_netmask;
                    if (nm) {
                        inet_ntop(AF_INET, &nm->sin_addr, data_arr[i].netmask, INET_ADDRSTRLEN);
                    }
                }
                break;
            }
        }
        if (already_added) continue;

        NetworkData *data = &data_arr[count];
        memset(data, 0, sizeof(NetworkData));

        // Copy Interface Name
        strncpy(data->name, ifa->ifa_name, IFNAMSIZ - 1);
        data->name[IFNAMSIZ - 1] = '\0';

        // Check if there is an active IPv4 address assigned to it
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in *sa = (struct sockaddr_in *)ifa->ifa_addr;
            inet_ntop(AF_INET, &sa->sin_addr, data->ip, INET_ADDRSTRLEN);

            struct sockaddr_in *nm = (struct sockaddr_in *)ifa->ifa_netmask;
            if (nm) {
                inet_ntop(AF_INET, &nm->sin_addr, data->netmask, INET_ADDRSTRLEN);
            } else {
                strncpy(data->netmask, "255.255.255.0", INET_ADDRSTRLEN - 1);
            }
        } else {
            // No IP assigned yet (interface is disconnected or down)
            strncpy(data->ip, "0.0.0.0", INET_ADDRSTRLEN - 1);
            strncpy(data->netmask, "0.0.0.0", INET_ADDRSTRLEN - 1);
        }

        // Try to grab transmission bytes (fallback to 0 if the interface is down/unreadable)
        if (get_net_bytes(data->name, &data->rx_bytes, &data->tx_bytes) != 0) {
            data->rx_bytes = 0;
            data->tx_bytes = 0;
        }

        count++;
    }

    freeifaddrs(ifaddr);
    return count;
}