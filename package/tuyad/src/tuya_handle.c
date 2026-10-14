#include "tuya_handle.h"

void on_connected(tuya_mqtt_context_t* context, void* user_data)
{
    // TY_LOGI("on connected");

}

void on_disconnect(tuya_mqtt_context_t* context, void* user_data)
{
    // TY_LOGI("on disconnect");
}

void on_messages(tuya_mqtt_context_t* context, void* user_data, const tuyalink_message_t* msg)
{
    //TY_LOGI("on message id:%s, type:%d, code:%d", msg->msgid, msg->type, msg->code);
    switch (msg->type) {
        case THING_TYPE_ACTION_EXECUTE:
            if(check_action_code(msg->data_string, "write_log") == 0){
                log_file(msg->data_string, "/tmp/tuya_action.log");
            }
            break;
        case THING_TYPE_PROPERTY_REPORT_RSP:
            if(msg->code == 0){
                system_log(7, "Property report accepted.");
            }
            else {
                char err_msg[50];
                snprintf(err_msg, 50, "Property report rejected, error: %d", (int)msg->code);
                system_log(7, err_msg);
            }
        default:
            break;
    }
    printf("\r\n");
}

int prepare_tuya(tuya_mqtt_context_t *client, tuya_mqtt_config_t *config)
{
    int ret = tuya_mqtt_init(client, config);
    if(ret != OPRT_OK){
        system_log(0, "Unable to mqtt init\n");
        return ret;
    }

    ret = tuya_mqtt_connect(client);
    if(ret != OPRT_OK){
        system_log(0, "Unable to mqtt connect\n");
    }
    return ret;
}

int send_interface_report(tuya_mqtt_context_t *client, struct ifaddrs *ifa)
{
    int linescount = 0;
    for(struct ifaddrs *i = ifa; i != NULL; i = i->ifa_next) {
        if(i->ifa_addr != NULL && i->ifa_addr->sa_family == AF_INET) { linescount++; } 
    }
    if(linescount == 0){
        return 0;
    }
    int sz = linescount * 128;
    char interfaces[sz];
    interfaces[0] = '\0';
    if(get_all_interface_json(ifa, interfaces, sz) == -1){
        return -1;
    }
    char data[32 + sizeof(interfaces)];
    data[0] = '\0';
    int ret = snprintf(data, 32 + sizeof(interfaces), "{ \"interfaces\" : { \"value\" : [%s], \"time\" : %ld}}" , interfaces, time(NULL));
    if(ret == sizeof(data)){
        system_log(3, "NOT ENOUGH SIZE FOR DATA IN INTERFACE REPORT");
        return -1;
    }

    return tuyalink_thing_property_report(client, NULL, data);
}

int send_sysinfo_report(tuya_mqtt_context_t *client, struct MemData *info)
{
    int linescount = 3;
    size_t sz = linescount * 72;
    char data[sz];
    int ret = 0;
    if((ret = convert_sysinfo_json(info, data, sz)) == sz){
        system_log(3, "NOT ENOUGH SIZE FOR DATA IN SYSTEM INFO REPORT");
        return -1;
    }
    return tuyalink_thing_property_report(client, NULL, data);
}

int send_cpu_usage_report(tuya_mqtt_context_t *client, struct cpu_load *cpu)
{
    int linescount = 2;
    size_t sz = linescount * 64;
    char data[sz];
    if(convert_cpu_json(cpu, data, sz) == sz){
        system_log(3, "NOT ENOUGH SIZE FOR DATA IN CPU LOAD REPORT");
        return -1;
    }
    return tuyalink_thing_property_report(client, NULL, data);
}

int send_uptime_report(tuya_mqtt_context_t *client, uint64_t uptime){
    size_t sz = 32;
    char data[sz];
    if(convert_uptime_json(uptime, data, sz) == sz){
        system_log(3, "NOT ENOUGH SIZE FOR DATA IN UPTIME REPORT");
        return -1;
    }
    return tuyalink_thing_property_report(client, NULL, data);
}