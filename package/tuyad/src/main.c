#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "tuya_cacert.h"
#include "tuya_log.h"
#include "tuya_error_code.h"
#include "tuyalink_core.h"
#include "tuya_handle.h"
#include "arg_parse.h"
#include "log_wrapper.h"
#include <signal.h>
#include "daemon.h"
#include "ubus_info.h"
#include <errno.h>

void sig_handler(int signum);
void prepare_sig_handler(void);

sig_atomic_t sig_flag = 0;

int main(int argc, char *argv[]){
    struct arguments args = {0};
    struct argp argp = {0};

    arg_init(argc, argv, &args, &argp);
    error_t parseret = arg_parse(&argp, argc, argv, &args);
    if(parseret != 0 ||
    args.deviceId == NULL ||
    args.deviceSecret == NULL ||
    args.productId == NULL){
        print_argp_help(&argp, argv);
        return 1;
    }


    prepare_sig_handler();  

    int ret;
    if(args.daemon == 1){
        start_daemon();
    }

    set_mask(7);
    size_t err_sz = 128;
    char err_msg[err_sz];

    tuya_mqtt_context_t client = {0};
    ret = prepare_tuya(&client, &(tuya_mqtt_config_t) {
        .host = "m1.tuyacn.com",
        .port = 8883,
        .cacert = tuya_cacert_pem,
        .cacert_len = sizeof(tuya_cacert_pem),
        .device_id = args.deviceId,
        .device_secret = args.deviceSecret,
        .keepalive = 100,
        .timeout_ms = 2000,
        .on_connected = on_connected,
        .on_disconnect = on_disconnect,
        .on_messages = on_messages
    });
    if(ret != OPRT_OK){
        snprintf(err_msg, err_sz, "Unable to init or connect to the cloud, returned: %d", ret);
        system_log(1, err_msg);
        goto free;
    }

    struct ifaddrs *addrs = NULL;
    int ifaddrsret = 0;
    system_log(7, "Pulling network interfaces data..");
    if((ifaddrsret = getifaddrs(&addrs))){
        snprintf(err_msg, err_sz, "Unable to get information for interfaces -- error: %d", errno);
        system_log(1, err_msg);
    }

    time_t last_report = 0;
    time_t now;

    struct SystemInfo info = {0};
    struct cpu_load load = {0};
    struct MemData memory = {0};
    info.load = &load;
    info.memory = &memory;


    struct ubus_context *ctx;
    uint32_t id;
	system_log(7, "Connecting to ubus..");
	if (!(ret = ubus_prepare(&ctx, &id))) {
        snprintf(err_msg, err_sz, "Failed to connect to ubus, returned: %d", ret);
        system_log(3, err_msg);
        goto free;
	}

    system_log(7, "Entering report loop..");
    while (sig_flag != 1)
    {
        tuya_mqtt_loop(&client);
        now = time(NULL);

        if((now - last_report) < 10){
            continue;
        }
        last_report = now;
        
        if(ifaddrsret == 0){ 
            system_log(7, "Sending network interfaces report..");
            if((ret = send_interface_report(&client, addrs)) < 0){
                snprintf(err_msg, err_sz, "Cannot send network interfaces report, returned: %d", ret);
                system_log(3, err_msg);
            }
        }

        system_log(7, "Sending uptime report..");
        ret = 0;
        if(info.uptime == 0 || (ret = send_uptime_report(&client, info.uptime)) < 0){
            system_log(3, "Cannot send uptime report.");
            if(ret < 0){
                snprintf(err_msg, err_sz, "returned: %d", ret);
                system_log(3, err_msg);
            }
        }   

        system_log(7, "Pulling ubus data..");
        if ((ret = ubus_invoke(ctx, id, "info", NULL, board_cb, &info, 3000)) != 0) {
            snprintf(err_msg, err_sz, "Cannot request system info from procd, returned: %d", ret);
		    system_log(1, err_msg);
            continue;
	    }
        

        system_log(7, "Sending system info report..");
        if((ret = send_sysinfo_report(&client, info.memory)) < 0){
            snprintf(err_msg, err_sz, "Cannot send system info report, returned: %d", ret);
            system_log(3, err_msg);
        }

        system_log(7, "Sending cpu load report..");
        if((ret = send_cpu_usage_report(&client, info.load)) < 0){
            snprintf(err_msg, err_sz, "Cannot send cpu load report, returned: %d", ret);
            system_log(3, err_msg);
        }
    }

    free:
    system_log(6, "Freeing system info memory.. ");
    freeifaddrs(addrs);
    ubus_free(ctx);
    tuya_mqtt_deinit(&client);
    return 0;
}

void prepare_sig_handler(void)
{
    struct sigaction sig = {0};
    sig.sa_handler = &sig_handler;
    sigemptyset(&sig.sa_mask);
    sig.sa_flags = 0;

    sigaction(SIGINT, &sig, NULL);
    sigaction(SIGTERM, &sig, NULL);
    sigaction(SIGQUIT, &sig, NULL);
}

void sig_handler(int signum)
{
    sig_flag = 1;
}