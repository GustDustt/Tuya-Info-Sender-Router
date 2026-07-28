#include "sys_info.h"

void on_connected(tuya_mqtt_context_t* context, void* user_data)
{
    syslog(LOG_INFO, "on connected");
}

void on_disconnect(tuya_mqtt_context_t* context, void* user_data)
{
    syslog(LOG_INFO, "on disconnect");
}

int connector(const struct arguments *arguments) 
{
    int ret = OPRT_OK;
    tuya_mqtt_context_t* client = &client_instance;

    ret = tuya_mqtt_init(client, &(const tuya_mqtt_config_t) {
        .host = "m1.tuyacn.com",
        .port = 8883,
        .cacert = tuya_cacert_pem,
        .cacert_len = sizeof(tuya_cacert_pem),
        .device_id = arguments->device_id,       
        .device_secret = arguments->secret,     
        .keepalive = 100,
        .timeout_ms = 2000,
        .on_connected = on_connected,
        .on_disconnect = on_disconnect,
        .on_messages = on_messages
    });
    
    if (ret != OPRT_OK) {
        syslog(LOG_ERR, "Failed to start Tuya IoT client. Error code: %d", ret);
        closelog();
        return EXIT_FAILURE; 
    }

    ret = tuya_mqtt_connect(client);
    if (ret != OPRT_OK) {
        syslog(LOG_ERR, "Failed to start Tuya IoT client. Error code: %d", ret);
        tuya_mqtt_deinit(client);
        closelog();
        return EXIT_FAILURE;
    }
    syslog(LOG_INFO, "Tuya IoT client started.");
    return OPRT_OK;
}

void log_creator(cJSON *text) {
    if (text == NULL || text->valuestring == NULL) {
        syslog(LOG_ERR, "Tuya Log: Received empty message");
        return;
    }
    
    syslog(LOG_INFO, "Tuya Cloud Request: %s", text->valuestring);
}

void on_messages(tuya_mqtt_context_t* context, void* user_data, const tuyalink_message_t* msg)
{
    syslog(LOG_INFO, "on message id:%s, type:%d, code:%d", msg->msgid, msg->type, msg->code);
    if (msg->type == THING_TYPE_ACTION_EXECUTE) {
        syslog(LOG_INFO, "Action received: %s", msg->data_string);
        cJSON *root = cJSON_Parse(msg->data_string);
        if (!root) {
            syslog(LOG_ERR, "Failed to parse cloud JSON payload");
            return;
        }

        cJSON *actionCode = cJSON_GetObjectItemCaseSensitive(root, "actionCode");
        cJSON *inputParams = cJSON_GetObjectItemCaseSensitive(root, "inputParams");

        if (actionCode && strcmp(actionCode->valuestring, "log_action") == 0 && inputParams) {
            cJSON *text = cJSON_GetObjectItemCaseSensitive(inputParams, "text");
            if (text && text->valuestring) {
                log_creator(text);
            } else {
                syslog(LOG_ERR, "Missing or invalid 'text' param inside inputParams");
            }
        }
        cJSON_Delete(root);
    } else {
        syslog(LOG_ERR, "Action mismatch or missing parameters at root level");
    }
}

int tuya_mqtt_checker(){
    tuya_mqtt_context_t* client = &client_instance;
    int ret = tuya_mqtt_loop(client);
    if (ret != OPRT_OK) {
        syslog(LOG_ERR, "tuya_iot_yield failed with error code: %d", ret);
        return ret;
    }
    return OPRT_OK;
}