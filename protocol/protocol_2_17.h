/*
 * protocol_2_17.h
 *
 *  Created on: 2016Äê6ÔÂ29ÈÕ
 *      Author: yiqiu.yang
 */

#ifndef SOURCES_PROTOCOL_2_17_H_
#define SOURCES_PROTOCOL_2_17_H_

#include "FreeRTOS.h"

#include "message.h"

#define AGING_TEST_INFO_CONFIG					0x41C

extern bool check_all_sensor_flag;
void protocol_2_17_sensor_type(RF_MSG *msg);
int smart_water_smoke_2_17_handle(SENSOR_STATUS_STORAGE_BLOCK *status, RF_MSG *get_queue_packet);

int smart_mag_2_17_handle(SENSOR_STATUS_STORAGE_BLOCK *status, RF_MSG *get_queue_packet);
int smart_pir_2_17_handle(SENSOR_STATUS_STORAGE_BLOCK *status, RF_MSG *get_queue_packet);

void handle_msg_callback(RF_MSG *get_queue_packet);

uint32_t GET_SENSOR_VBATT(SENSOR_STATUS_STORAGE_BLOCK *sensor_state);


#endif /* SOURCES_PROTOCOL_2_17_H_ */
