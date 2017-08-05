#ifdef BRIDGE
void plc_bridge_task(void *pvParameters);
void sync_rtc_time_ack(uint32_t data,uint32_t seq);
void query_bridge_sensor_ack(uint32_t data,uint32_t seq);
void plc_alarm_happened(uint32_t sid,uint32_t seq);
void plc_alarm_cleared(uint32_t sid,uint32_t seq);
void plc_toggle_bridge_sensor_ack(uint32_t data,uint32_t sid,uint32_t seq);
void set_bridge_pir_sens_ack(uint32_t data,uint32_t sid,uint32_t seq);
void sent_sensor_list_ack(uint32_t seq);
void bridge_get_sensor_list(uint32_t seq);
void deploy_sensor_list_ack(uint32_t seq);
void plc_fw_need_update_ack(uint32_t seq);
void plc_request_img_block(uint32_t device_type,uint32_t data,uint32_t seq);
void plc_fw_update_result(uint32_t device_type,uint32_t data,uint32_t seq);
void plcalarmCallback();
#else
void sync_bridge_rtc_time(uint32_t bridge_id,uint32_t seq);
void query_bridge_sensor(uint32_t bridge_id,uint32_t seq);
void disengage_bridge_sensor(uint32_t bridge_id,uint32_t sid,uint32_t seq);
void engage_bridge_sensor(uint32_t bridge_id,uint32_t sid,uint32_t seq);
void set_bridge_pir_low_sens(uint32_t bridge_id,uint32_t sid,uint32_t seq);
void set_bridge_pir_high_sens(uint32_t bridge_id,uint32_t sid,uint32_t seq);
void sent_sensor_list_to_bridge(uint32_t bridge_id,uint32_t seq);
void deploy_sensor_list_to_bridge(uint32_t bridge_id,uint32_t seq);
void plc_fw_need_update(uint32_t bridge_id,uint32_t device_type,uint32_t seq);
void plc_request_img_block_ack(uint32_t device_type,uint32_t data,uint32_t hb_id,uint32_t seq);
#endif
