local ConfigService = require("api/ConfigService")

local Service = ConfigService:new(
	{
		delete = false,
		create = false
	})
local ConfigSection = Service:section("tuyad", "start")
ConfigSection:make_primary()

local opt_enable = ConfigSection:option("enable")
local opt_prod_id = ConfigSection:option("productID")
local opt_dev_id = ConfigSection:option("deviceID")
local opt_dev_secret = ConfigSection:option("device_secret")

return Service

