typedef struct
{
	uint32_t addr;
	uint32_t length;
	char block_sha1[1272];
	char img_sha1[32];
} fota_subnode_header;

typedef struct
{
	char head_sha1[32];
	char hwver[32];
	char swver[32];
	char aeskey[64];
	fota_subnode_header subnode_header;
} fota_node_header;

extern fota_node_header *internal_img_info;
extern fota_node_header *hb_tmp_img_info;
extern fota_node_header external_img_info[3];
extern bool fota_img_update_state[3];

#define IMG_NONE -1
#define IMG_DEFAULT_NUM  0
#define IMG_1_NUM  1
#define IMG_2_NUM  2

#define HEADER_BLOCK_SIZE 0x1000   //4K for header
#define HB_IMG_TMP_SIZE 0x32000//ram store img size , 20K
#define INTERNAL_CONFIG_ADDR 0x3F000 //

static const uint32_t img_addr[3] = {
		0x00100000,
		0x00140000,
		0x00180000
};


#define BLOCK_SIZE 4096
#define SHA1_SIZE 20

extern bool hb_img_update_flag;
extern uint8_t hb_img_update_num ;
extern uint8_t hb_img_rollback_num;
extern uint8_t hb_img_spi_flash_addr;
extern uint8_t hb_tmp_img[HB_IMG_TMP_SIZE];

void get_external_img_info();
