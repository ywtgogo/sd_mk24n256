#ifndef _UPLINK_KM_H_
#define _UPLINK_KM_H_


#pragma   pack(1)
typedef struct FRAMEDATA
{
	uint8_t			data[21];
}FRAMEDATA_t;

typedef struct FRAMEHEAD
{
	uint16_t		frame_header;
	uint8_t			frame_ver;
	uint8_t			ctl_code;
	uint32_t		dev_id;
	uint16_t		data_len;
}FRAMEHEAD_t;

typedef struct FRAME
{
	FRAMEHEAD_t		frame_head;
	FRAMEDATA_t		frame_data;
	uint8_t			frame_tail;
}FRAME_t;


typedef struct HUB_MSQ_TYPE
{
	uint16_t	len;
	uint8_t		cmd;
	uint8_t		data[32];
}HUB_MSQ_TYPE_t;

typedef struct SENSOR_MSQ_TYPE
{
	uint16_t	len;
	uint8_t		cmd;
	uint32_t	id;
	uint8_t		*data;
}SENSOR_MSQ_TYPE_t;

#pragma   pack()

#endif
