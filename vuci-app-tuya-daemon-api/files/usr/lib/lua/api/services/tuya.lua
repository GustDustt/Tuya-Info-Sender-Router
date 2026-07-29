local ConfigService = require("api/ConfigService")

local TuyaConfig = ConfigService:new({
    delete = false,
    create = false,
    general_section = "settings"
})

local DaemonSettings = TuyaConfig:section(
    "sys_monitor_daemon", 
    "daemon"              
)
DaemonSettings:make_primary()

local opt_action = DaemonSettings:option("action")
function opt_action:validate(value)
    return self.dt:check_array(value, { "start", "stop", "restart" })
end

local opt_device_id = DaemonSettings:option("device_id")
opt_device_id.maxlength = 64

local opt_product_id = DaemonSettings:option("product_id")
opt_product_id.maxlength = 64

local opt_secret = DaemonSettings:option("secret")
opt_secret.maxlength = 128

function TuyaConfig:GET_after_data_hook(data)
    local running = false
    local pid = nil

    local ubus = require("ubus")
    local conn = ubus.connect()
    if conn then
        local service_name = "sys_monitor_daemon"
        local result = conn:call("service", "list", { name = service_name })
        conn:close()

        local instances = result and result[service_name] and result[service_name].instances
        if instances then
            for _, instance in pairs(instances) do
                if instance.running then
                    running = true
                    pid = instance.pid
                end
            end
        end
    end

    data.running = running
    data.pid = pid
end

function TuyaConfig:PUT_after_commit_hook()
    local action = self:table_get(self.config, self.sid, "action")

    if action == "start" or action == "stop" or action == "restart" then
        local uci = require("uci")
        local cursor = uci.cursor()
        cursor:set(self.config, self.sid, "enabled", (action == "stop") and "0" or "1")
        cursor:commit(self.config)

        os.execute("/etc/init.d/sys_monitor_daemon " .. action)
    end
end

return TuyaConfig