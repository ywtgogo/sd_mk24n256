typedef struct
{
	uint32_t addr;
	uint32_t length;
	uint8_t block_sha1[1272];
	uint8_t img_sha1[32];
} fota_subnode_header;

typedef struct
{
	uint8_t 		head_sha1[32];
	uint32_t		dev_type;
	uint32_t		hw_ver;
	uint8_t			hw_info[24];
	uint32_t		sw_ver;
	uint8_t			sw_info[92];
	fota_subnode_header subnode_header;
} fota_node_header;

typedef struct
{
	uint32_t	fw_change;
	uint32_t	fw_pre;
	uint32_t	fw_now;
	uint8_t		fw_err_block;
}boot_record;

enum
{
	FW_NOCHANGE = 0,
	FW_UPDATE_SUCCESS,
	FW_UPDATE_FAILED_BLOCK_SHA1_ERR,
	FW_UPDATE_FAILED_IMG_SHA1_ERR,
	FW_ROLLBACK,
};

extern fota_node_header *internal_img_info;
extern fota_node_header external_img_info[3];
extern bool fota_img_update_state[3];

#define IMG_NONE -1
#define IMG_DEFAULT_NUM  0
#define IMG_1_NUM  1
#define IMG_2_NUM  2

#define HEADER_BLOCK_SIZE 0x1000   //4K for header
#define HB_IMG_TMP_SIZE 0x34000//ram store img size ,
#define INTERNAL_CONFIG_ADDR 0x3F000 //
#define BOOTLOADER_CACHE 0x2002f000

static const uint32_t img_addr[3] = {
		0x00100000,
		0x00140000,
		0x00180000
};


#define BLOCK_SIZE 4096
#define SHA1_SIZE 20

extern bool hb_img_update_flag;
extern uint8_t hb_img_update_num ;
extern int hb_img_rollback_num;
extern uint8_t hb_img_spi_flash_addr;
extern uint8_t hb_tmp_img[HB_IMG_TMP_SIZE];

void get_external_img_info();
