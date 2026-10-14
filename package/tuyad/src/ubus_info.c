#include "ubus_info.h"

void board_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
	struct SystemInfo *info = (struct SystemInfo *)req->priv;
	struct blob_attr *tb[__INFO_MAX];
	struct blob_attr *memory[__MEMORY_MAX];
	struct blob_attr *load[__LOAD_MAX];

	blobmsg_parse(info_policy, __INFO_MAX, tb, blob_data(msg), blob_len(msg));
	
	if (tb[UPTIME]) {
		info->uptime = blobmsg_get_u64(tb[UPTIME]);
	}
	else {	system_log(2, "No uptime data received\n"); }


	if(tb[LOAD])
	{
		blobmsg_parse_array(load_policy, __LOAD_MAX, load, blobmsg_data(tb[LOAD]), blobmsg_len(tb[LOAD]));
	}
	else { system_log(2, "No load data received"); }
	if(load[_1MIN] && load[_5MIN] && load[_15MIN]){
		info->load->_1min  = blobmsg_get_u32(load[_1MIN]);
		info->load->_5min  = blobmsg_get_u32(load[_5MIN]);
		info->load->_15min = blobmsg_get_u32(load[_15MIN]);
	} else { system_log(2, "No load data received"); }


	if(tb[MEMORY]){
		blobmsg_parse(memory_policy, __MEMORY_MAX, memory, blobmsg_data(tb[MEMORY]), blobmsg_data_len(tb[MEMORY]));
	} else { system_log(2, "No memory data received"); }
	info->memory->totalram    = blobmsg_get_u64(memory[TOTAL_RAM]);
	info->memory->freeram     = blobmsg_get_u64(memory[FREE_RAM]);
	return;
}

int ubus_prepare(struct ubus_context **ctx, uint32_t *id)
{
	*ctx = ubus_connect(NULL);
	if (!(*ctx)) {
		system_log(0, "Failed to connect to ubus");
		return CONN_ERR;
	}

	if (ubus_lookup_id(*ctx, "system", id)) { //?
		system_log(0, "Unable to find system id");
		return NO_ID;
	}

	system_log(7, "ubus returned succesfully..");

	return 1;
}

