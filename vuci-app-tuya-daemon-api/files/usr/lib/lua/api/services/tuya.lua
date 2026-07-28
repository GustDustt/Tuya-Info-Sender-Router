local FunctionService = require("api/FunctionService")

local Tuya = FunctionService:new()

local SERVICE = "sys_monitor_daemon"
local VALID_COMMANDS = { start = true, stop = true, restart = true }

function Tuya:GET_TYPE_test()
    return self:ResponseOK({
        result = "Tuya API works"
    })
end

function Tuya:GET_TYPE_status()
    local ubus = require("ubus")
    local conn = ubus.connect()
    local result = conn:call("service", "list", { name = SERVICE })
    conn:close()

    local instances = result and result[SERVICE] and result[SERVICE].instances
    local running = false
    local pid = nil
    if instances then
        for _, instance in pairs(instances) do
            if instance.running then
                running = true
                pid = instance.pid
            end
        end
    end

    return self:ResponseOK({
        running = running,
        pid = pid
    })
end

function Tuya:POST()
    local command = self.arguments and self.arguments.command

    if not VALID_COMMANDS[command] then
        return self:ResponseError("command must be one of: start, stop, restart")
    end

    os.execute("/etc/init.d/" .. SERVICE .. " " .. command)

    return self:ResponseOK({
        success = true,
        command = command
    })
end

return Tuya
