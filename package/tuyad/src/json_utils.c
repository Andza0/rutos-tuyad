#include "json_utils.h"

int convert_interface_json(struct ifaddrs *i, char dest[], size_t sz)
{
    if(i == NULL || dest == NULL || sz == 0){
        system_log(3, "NULL");
        return -1;
    }
    int ret;
    char ip_string[INET_ADDRSTRLEN] = "";
    ret = parse_inet(i, IP_ADDR, ip_string);
    if(ret == -1){
        system_log(3, "UNABLE TO READ IP_ADDRES");
    }

    char netmask[INET_ADDRSTRLEN] = "";
    ret = parse_inet(i, MASK, netmask);
    if(ret == -1){
        system_log(3, "UNABLE TO READ NETMASK");
    }

    long rx_bytes = get_rxtx(RX_BYTES, i);
    if(!rx_bytes){
        char error[64];
        snprintf(error, 64, "UNABLE TO READ RECEIVED BYTES, RETURNED: %ld", rx_bytes);
        system_log(1, error);
    }
    long rx = rx_bytes / (1024 * 1024);

    long tx_bytes = get_rxtx(TX_BYTES, i);
    if(!tx_bytes){
        char error[64];
        snprintf(error, 64, "UNABLE TO READ TRANSCEIVED BYTES, RETURNED: %ld", tx_bytes);
        system_log(1, error);
    }
    long tx = tx_bytes / (1024 * 1024);

    return snprintf(dest, sz,
    "\"{"
    "\\\"name\\\":\\\"%s\\\","
    "\\\"ip_addr\\\":\\\"%s\\\","
    "\\\"netmask\\\":\\\"%s\\\","
    "\\\"rx\\\":%ld,"
    "\\\"tx\\\":%ld"
    "}\",", i->ifa_name, ip_string, netmask, rx, tx);
}

int get_all_interface_json(struct ifaddrs *ifa, char result[], size_t sz)
{
    if(result == NULL || sz == 0){
        system_log(3, "NULL");
        return -1;
    }
    char interface[128];
    int ret = 0;
    for(struct ifaddrs *i = ifa; i != NULL; i = i->ifa_next){
        if(i->ifa_name == NULL || 
        i->ifa_addr == NULL || 
        strcmp(i->ifa_name, "lo") == 0 || 
        i->ifa_addr->sa_family != AF_INET){
            continue;
        }
        if((ret = convert_interface_json(i, interface, sizeof(interface))) == sizeof(interface)){
            system_log(3, "INTERFACE TRUNCATED");
            return -1;
        }

        if((strlen(result) + strlen(interface) + 1) < sizeof(result)){
            system_log(3, "NOT ENOUGH SPACE IN INTERFACES ARRAY");
            return -1;
        }
        strcat(result, interface);
    }
    result[strlen(result) - 1] = '\0';
    return 0;
}

int convert_sysinfo_json(struct MemData *info, char dest[], size_t sz)
{
    if(info == NULL || dest == NULL || sz == 0){
        system_log(3, "NULL");
        return -1;
    }
    return snprintf(dest, sz,
    "{"
    "\"total_ram\":%lld,"
    "\"free_ram\":%lld,"
    "}",
    ((info->totalram)/ (1024 * 1024)),
    ((info->freeram)/(1024 * 1024)));
}

int convert_cpu_json(struct cpu_load *cpu, char dest[], size_t sz)
{    
    if(cpu == NULL || dest == NULL || sz == 0){
        system_log(3, "NULL");
        return -1;
    }
    return snprintf(dest, sz,
        "{"
        "\"1min\" : %d, "
        "\"5min\": %d, "
        "\"15min\": %d"
        "}",
        cpu->_1min,
        cpu->_5min,
        cpu->_15min);
}

int convert_uptime_json(uint64_t uptime, char data[], size_t sz){
    if(data == NULL || sz == 0){
        system_log(3, "NULL");
        return -1;
    }
    return snprintf(data, sz,
        "{ \"uptime\" : { \"value\" : %ld, \"time\" : %ld }}",
        uptime, time(NULL)
    );
}

int check_action_code(char *msg, char *action_code)
{
    cJSON *root = cJSON_Parse(msg);
    if(root == NULL){
        system_log(3, "UNABLE TO READ THE RECEIVED JSON FROM THE CLOUD");
        return -1;
    }
    cJSON *action = cJSON_GetObjectItemCaseSensitive(root, "actionCode");
    if(!cJSON_IsString(action) || action_code == NULL){
        system_log(3, "BAD ACTION CODE");
        cJSON_Delete(root);
        return -1;
    }

    int res = strcmp(action->valuestring, action_code);
    cJSON_Delete(root);
    return res;
}