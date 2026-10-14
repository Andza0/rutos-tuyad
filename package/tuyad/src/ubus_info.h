#pragma once

#include <libubox/blobmsg_json.h>
#include <libubus.h>
#include "log_wrapper.h"
#include <stdio.h>

//error MACRO ==================
//#define NULL_PTR -1
#define CONN_ERR -2
#define NO_ID -3
//==============================


struct MemData{
    uint64_t totalram;
    uint64_t freeram;
};

struct cpu_load{
    uint32_t _1min;
    uint32_t _5min;
    uint32_t _15min;
};


struct SystemInfo {
    uint64_t uptime;
    struct MemData *memory;
    struct cpu_load *load;
};

enum {
    TOTAL_RAM,
    FREE_RAM,
    __MEMORY_MAX
};

enum {
    UPTIME,
    LOAD,
    MEMORY,
    __INFO_MAX
};

enum {
    _1MIN,
    _5MIN,
    _15MIN,
    __LOAD_MAX
};

static const struct blobmsg_policy info_policy[__INFO_MAX] = {
    [UPTIME]    = { .name = "uptime",    .type = BLOBMSG_TYPE_INT32},
    [LOAD]      = { .name = "load",      .type = BLOBMSG_TYPE_ARRAY},
    [MEMORY]    = { .name = "memory",    .type = BLOBMSG_TYPE_TABLE},
};

static const struct blobmsg_policy memory_policy[__MEMORY_MAX] = {
    [TOTAL_RAM] = { .name = "total", .type = BLOBMSG_TYPE_INT64},
    [FREE_RAM]  = { .name = "free",  .type = BLOBMSG_TYPE_INT64}
};

static const struct blobmsg_policy load_policy[__LOAD_MAX] = {
    [_1MIN]  = { .type = BLOBMSG_TYPE_INT32 },
    [_5MIN]  = { .type = BLOBMSG_TYPE_INT32 },
    [_15MIN] = { .type = BLOBMSG_TYPE_INT32 },
};

void board_cb(struct ubus_request *req, int type, struct blob_attr *msg);
int ubus_prepare(struct ubus_context **ctx, uint32_t *id);