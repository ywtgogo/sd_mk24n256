#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "adc.h"
#include "utilities/vector_table_info.h"

#include "fsl_debug_console.h"
#include "fsl_pmc.h"
#include "fsl_flash.h"
#include "common.h"
#include "spiflash_config.h"
#include "aes256.h"


bool hb_img_update_flag = false;
uint8_t hb_img_update_num = IMG_NONE;
uint8_t hb_img_rollback_num = IMG_NONE;
uint8_t hb_img_spi_flash_addr;

__attribute((aligned (16)))
uint8_t hb_tmp_img[HB_IMG_TMP_SIZE];

fota_node_header *internal_img_info;
fota_node_header *hb_tmp_img_info;
fota_node_header external_img_info[3]={0};
bool fota_img_update_state[3];

//! @brief Returns the user application address and stack pointer.
//!
//! For flash-resident and rom-resident target, gets the user application address
//! and stack pointer from the APP_VECTOR_TABLE.
//! Ram-resident version does not support jumping to application address.
static void get_user_application_entry(uint32_t *appEntry, uint32_t *appStack)
{
    assert(appEntry);
    assert(appStack);

    *appEntry = APP_VECTOR_TABLE[kInitialPC];
    *appStack = APP_VECTOR_TABLE[kInitialSP];
}

static void jump_to_application(uint32_t applicationAddress, uint32_t stackPointer)
{
//    shutdown_cleanup(kShutdownType_Shutdown);

    // Create the function call to the user application.
    // Static variables are needed since changed the stack pointer out from under the compiler
    // we need to ensure the values we are using are not stored on the previous stack
    static uint32_t s_stackPointer = 0;
    s_stackPointer = stackPointer;
    static void (*farewellBootloader)(void) = 0;
    farewellBootloader = (void (*)(void))applicationAddress;

    // Set the VTOR to the application vector table address.
    SCB->VTOR = (uint32_t)APP_VECTOR_TABLE;

    // Set stack pointers to the application stack pointer.
    __set_MSP(s_stackPointer);
    __set_PSP(s_stackPointer);

    // Jump to the application.
    farewellBootloader();
    // Dummy fcuntion call, should never go to this fcuntion call
//    shutdown_cleanup(kShutdownType_Shutdown);
}

void check_battery()
{
    adc16_config_t adc16ConfigStruct;
    adc16_channel_config_t adc16ChannelConfigStruct;

	PRINTF("\r\nSTEP 1 : battery adc check\r\n");
//	pmc_bandgap_buffer_config_t pmc_bandgap_buffer_config;
//	pmc_bandgap_buffer_config.enable = true;
//	PMC_ConfigureBandgapBuffer(PMC,&pmc_bandgap_buffer_config);


	EnableIRQ(DEMO_ADC16_IRQn);
	ADC16_GetDefaultConfig(&adc16ConfigStruct);
	ADC16_Init(DEMO_ADC16_BASE, &adc16ConfigStruct);
	ADC16_EnableHardwareTrigger(DEMO_ADC16_BASE, false);
#if defined(FSL_FEATURE_ADC16_HAS_CALIBRATION) && FSL_FEATURE_ADC16_HAS_CALIBRATION
    if (kStatus_Success == ADC16_DoAutoCalibration(DEMO_ADC16_BASE))
    {
        PRINTF("ADC16_DoAutoCalibration() Success.\r\n");
    }
    else
    {
        PRINTF("ADC16_DoAutoCalibration() Failed.\r\n");
    }
#endif /* FSL_FEATURE_ADC16_HAS_CALIBRATION */

	adc16ChannelConfigStruct.channelNumber = DEMO_ADC16_USER_CHANNEL;
	adc16ChannelConfigStruct.enableInterruptOnConversionCompleted = true;

#if defined(FSL_FEATURE_ADC16_HAS_DIFF_MODE) && FSL_FEATURE_ADC16_HAS_DIFF_MODE
    adc16ChannelConfigStruct.enableDifferentialConversion = false;
#endif /* FSL_FEATURE_ADC16_HAS_DIFF_MODE */

    uint32_t no_charging;
    g_Adc16ConversionDoneFlag = false;
    do{
    	   ADC16_SetChannelConfig(DEMO_ADC16_BASE, DEMO_ADC16_CHANNEL_GROUP, &adc16ChannelConfigStruct);
    	    while (!g_Adc16ConversionDoneFlag);
    	    PRINTF("ADC Value: %d\r\n",g_Adc16ConversionValue);

    	    no_charging = GPIO_ReadPinInput(GPIOB, 6);
    	    PRINTF("%s Charger\r\n", no_charging?"Has no ":"HAS");
    }while(g_Adc16ConversionValue<0x840 && no_charging);


    uint32_t charger;
    charger = !GPIO_ReadPinInput(GPIOB, 20);
    PRINTF("%s charging\r\n", charger?"Yes":"No");

}

void record_reset_reason()
{
	PRINTF("\r\nSTEP 2 : record reset reason\r\n");
}

bool is_sw_newer(char sw_old[32],char sw_new[32])
{
	uint32_t *sw_o = (uint32_t *)sw_old;
	uint32_t *sw_n = (uint32_t *)sw_new;
	if(*sw_o==0xffffffff) return true;
	return *sw_n>*sw_o;
}

bool is_hw_same(char hwold[32],char hwnew[32])
{
	//for test
	return 1;
	int val = strcmp(hwold,hwnew);
	return  val?false:true;
}
bool has_img(fota_node_header *img_info)
{
	char none = 0xff;
	if(img_info->subnode_header.addr != none)
			return true;
	return false;
}

void get_internal_img_info()
{
	char header_sha1[20] ={0};
	PRINTF("\r\nread internal header in flash\r\n");
	internal_img_info = (fota_node_header *)INTERNAL_CONFIG_ADDR; //why invalid
	PRINTF("internal img sw = %x.%x.%x.%x\r\n",internal_img_info->swver[3],internal_img_info->swver[2],internal_img_info->swver[1],internal_img_info->swver[0]);
	sha1_csum((char *)internal_img_info + sizeof(internal_img_info->head_sha1), BLOCK_SIZE - sizeof(internal_img_info->head_sha1),header_sha1);
	for(int j = 0; j<20; j++)
	{
		if(header_sha1[j]!=internal_img_info->head_sha1[j])
		{
			PRINTF("internal header sha1 error\r\n");
			return false;
		}
	}
	PRINTF("Internal img header sha1 pass\r\n");
}

void get_external_img_info(uint32_t addr[3],uint32_t size)
{
	int num_i;
	char header_sha1[20] ={0};
	PRINTF("\r\nread external header in flash\r\n");
	memset(external_img_info, 0xff, sizeof(external_img_info));
	for(num_i=0;num_i<3;num_i++)
	{
		memset(header_sha1, 0x00, sizeof(header_sha1));
		norflash_read_data_ll(&g_m_handle,addr[num_i],size,&external_img_info[num_i]);
		PRINTF("NO.%d img sw = 0x %x.%x.%x.%x\r\n",num_i,external_img_info[num_i].swver[3],external_img_info[num_i].swver[2],external_img_info[num_i].swver[1],external_img_info[num_i].swver[0]);
		if(has_img(&external_img_info[num_i]))
		{
			sha1_csum((char *)&external_img_info[num_i] + sizeof(external_img_info[num_i].head_sha1), BLOCK_SIZE - sizeof(external_img_info[num_i].head_sha1),header_sha1);
			int j;
			for(j = 0; j<20; j++)
			{
				if(header_sha1[j]!=external_img_info[num_i].head_sha1[j])
				{
					PRINTF("external NO.%d header sha1 error\r\n",num_i);
					break;
				}
			}
			if(j == 20) PRINTF("external NO.%d img header sha1 pass\r\n",num_i);
			fota_img_update_state[num_i] = true;
		}
	}
}
bool check_img_need_update()
{
	int i_num;
	char *tmp_swver = internal_img_info->swver;
	hb_img_update_num = IMG_DEFAULT_NUM;

	for(i_num = 2;i_num>0;i_num--)
	{
		if(
				fota_img_update_state[i_num] &&
				has_img(&external_img_info[i_num])  &&
				is_hw_same(internal_img_info->hwver,external_img_info[i_num].hwver) &&
				is_sw_newer(tmp_swver,external_img_info[i_num].swver))
		{
			hb_img_update_num = i_num;
			tmp_swver = external_img_info[i_num].swver;
		}
	}
	if(hb_img_update_num != IMG_DEFAULT_NUM)
	{
		PRINTF("hb img need update sw, update img is NO. %d, sw = 0x %x.%x.%x.%x\r\n",hb_img_update_num,external_img_info[hb_img_update_num].swver[3],external_img_info[hb_img_update_num].swver[2],hb_img_update_num,external_img_info[hb_img_update_num].swver[1],external_img_info[hb_img_update_num].swver[0]);
		return true;
	}

	return false;
}

bool write_flash_img(uint32_t *new_img,uint32_t img_size,fota_node_header *new_header, uint32_t header_size)
{
	flash_config_t flash_config;
	uint32_t destAddr = BL_APP_VECTOR_TABLE_ADDRESS;
	PRINTF("\r\nSTEP  .2 : write img into flash\r\n");
    memset(&flash_config, 0, sizeof(flash_config));

	if(kStatus_FLASH_Success != FLASH_Init(&flash_config))
	{
		 PRINTF("FLASH_Init FAIL\r\n");
		 return false;
	}
	else
	{
		PRINTF("FLASH_Init pass \r\n");
	}

	int num = (flash_config.PFlashSectorSize + img_size - 1) >> 12;
    img_size = num  * (flash_config.PFlashSectorSize);

	if(kStatus_FLASH_Success != FLASH_Erase(&flash_config, destAddr, img_size, kFLASH_apiEraseKey))
	{
		 PRINTF("FLASH_Erase address %x  - %d blocks FAIL \r\n",destAddr,num);
		 return false;
	}
	else
	{
		 PRINTF("FLASH_Erase address %x  - %d blocks pass \r\n",destAddr,num);
	}

	if(kStatus_FLASH_Success != FLASH_VerifyErase(&flash_config, destAddr, img_size, kFLASH_marginValueUser))
	{
		PRINTF("FLASH_VerifyErase FAIL \r\n");
		 return false;
	}
	else
	{
		PRINTF("FLASH_VerifyErase pass \r\n");
	}

//	for(int i = 0;i<img_size;i+=8)
//	{
//		memcpy((uint8_t*)new_img+i,(uint8_t*)new_img+0xFF50,8);
//	}

	if(kStatus_FLASH_Success != FLASH_Program(&flash_config, destAddr, new_img, img_size))
	{
		 PRINTF("FLASH_Program address %x  - %d blocks FAIL \r\n",destAddr,num);
		 return false;
	}
	else
	{
		PRINTF("FLASH_Program address %x  - %d blocks pass \r\n",destAddr,num);
	}

    uint32_t failAddr, failDat;
    if(kStatus_FLASH_Success != FLASH_VerifyProgram(&flash_config, destAddr, img_size*sizeof(uint8_t), new_img, kFLASH_marginValueUser,
                                    &failAddr, &failDat))
    {
		 PRINTF("FLASH_VerifyProgram FAIL\r\n");
		 return false;
    }
    else
    {
    	 PRINTF("FLASH_VerifyProgram pass \r\n");
    }

    // flash header info
    destAddr = INTERNAL_CONFIG_ADDR;
    //erase at least one sectorSize
	if(kStatus_FLASH_Success != FLASH_Erase(&flash_config, destAddr, flash_config.PFlashSectorSize, kFLASH_apiEraseKey))
	{
		 PRINTF("FLASH_Erase address %x FAIL\r\n",destAddr);
		 return false;
	}
	else
	{
		PRINTF("FLASH_Erase address %x pass \r\n",destAddr);
	}
	if(kStatus_FLASH_Success != FLASH_VerifyErase(&flash_config, destAddr,  flash_config.PFlashSectorSize, kFLASH_marginValueUser))
	{
		PRINTF("FLASH_VerifyErase FAIL \r\n");
		 return false;
	}
	else
	{
		PRINTF("FLASH_VerifyErase pass \r\n");
	}

	if(kStatus_FLASH_Success != FLASH_Program(&flash_config, destAddr, new_header, header_size))
	{
		 PRINTF("FLASH_Program address %x FAIL\r\n",destAddr);
		 return false;
	}
	else
	{
		PRINTF("FLASH_Program  address %x pass \r\n",destAddr);
	}

    if(kStatus_FLASH_Success != FLASH_VerifyProgram(&flash_config, destAddr, header_size*sizeof(uint8_t), new_header, kFLASH_marginValueUser,
                                    &failAddr, &failDat))
    {
		 PRINTF("FLASH_VerifyProgram address %x FAIL\r\n",destAddr);
		 return false;
    }
    else
    {
    	 PRINTF("FLASH_VerifyProgram address %x pass \r\n",destAddr);
    }
	 return true;
}


int get_previous_img_num()
{
	PRINTF("get previous img\r\n");
	int i_num = 0;
	for(i_num = 2;i_num>=0;i_num--)
	{
		if(
			(i_num!=hb_img_rollback_num) &&
			has_img(&external_img_info[i_num])  &&
			is_hw_same(external_img_info[i_num].hwver,hb_tmp_img_info->hwver) &&
			is_sw_newer(external_img_info[i_num].swver,hb_tmp_img_info->swver))
		{
			break;
		}
	}
	hb_img_rollback_num = i_num;
	return i_num;
}

bool check_internal_img_right()
{
	char hb_img_sha1[20] = {0};
	char *block_sha1_from_header = internal_img_info->subnode_header.img_sha1;
	char *img = BL_APP_VECTOR_TABLE_ADDRESS;
	int num = (internal_img_info->subnode_header.length+ BLOCK_SIZE - 1) >> 12;

	sha1_csum(img, BLOCK_SIZE*num, hb_img_sha1);

	for(int j = 0; j<20; j++)
	{
		if(hb_img_sha1[j]!=internal_img_info->subnode_header.img_sha1[j])
		{
			PRINTF("internal img sha1 error\r\n");
			return false;
		}
	}
	PRINTF("img sha1 pass\r\n");

	return true;

}


bool check_spi_flash_img(uint8_t img_num,uint8_t *hb_img_decrypt,uint32_t spi_flash_addr,uint32_t length)
{
	PRINTF("read NO.%d img from spi flash\r\n",img_num);

	unsigned char aesivv[20]="SandlacusData#@1";
	unsigned char aeskey[40]="SandlacusData#@1SandlacusData#@1";
	uint8_t  fota_data_read[4096]={0};
	uint8_t  fota_data_decrypt[4096] = {0};
	uint8_t *hb_img_tmp_decrypt = hb_img_decrypt;
	uint8_t ivv[SHA1_SIZE] = { '\0' };
	char block_sha1[20];
	char *block_sha1_from_header;
	char hb_img_sha1[20] = {0};
	AES_KEY enc_key;
	AES_set_decrypt_key(aeskey, 256, &enc_key);
	int num = (length+ 4096 - 1) >> 12;
	uint32_t id = 0;
	norflash_read_ID_ll(&g_m_handle,&id);

	block_sha1_from_header =external_img_info[img_num].subnode_header.block_sha1;
	for(int i =0;i<num;i++)
	{
		memset(fota_data_read, 0xff, BLOCK_SIZE);
		memset(fota_data_decrypt, 0, BLOCK_SIZE);
		memset(block_sha1, 0, 20);
		memcpy(ivv, aesivv, 20);

		norflash_read_data_ll(&g_m_handle,spi_flash_addr,BLOCK_SIZE,fota_data_read);
		sha1_csum(fota_data_read, BLOCK_SIZE, block_sha1);

		for(int j = 0; j<20; j++)
		{
			if(block_sha1[j]!=block_sha1_from_header[j])
			{
				PRINTF("block %d sha1 error\r\n",i+1);
				return false;
			}
		}

		AES_cbc_encrypt(fota_data_read, fota_data_decrypt, BLOCK_SIZE, &enc_key, ivv, 0,1);
		memcpy(hb_img_decrypt, fota_data_decrypt, BLOCK_SIZE);
		block_sha1_from_header+=20;
		hb_img_decrypt+=BLOCK_SIZE;
		spi_flash_addr +=BLOCK_SIZE;
	}
	PRINTF("all blocks sha1 pass\r\n");

	sha1_csum(hb_img_tmp_decrypt, BLOCK_SIZE*num, hb_img_sha1);
	for(int j = 0; j<20; j++)
	{
		if(hb_img_sha1[j]!=external_img_info[img_num].subnode_header.img_sha1[j])
		{
			PRINTF("img sha1 error\r\n");
			return false;
		}
	}
	PRINTF("img sha1 pass\r\n");
	return true;
}

bool check_flash_img()
{

	PRINTF("\r\nSTEP 3 : check internal img \r\n");
	if(check_internal_img_right())
		return true;
	else
	{
		PRINTF("\r\nSTEP 4-1 : img damaged ,img need turn back\r\n");
    	memset(hb_tmp_img,0xff,HB_IMG_TMP_SIZE);
	    if(get_previous_img_num() < 0) return false;
	    if(check_spi_flash_img(hb_img_rollback_num,hb_tmp_img,img_addr[hb_img_rollback_num]+HEADER_BLOCK_SIZE,external_img_info[hb_img_rollback_num].subnode_header.length))
	    {
	    	write_flash_img(hb_tmp_img,external_img_info[hb_img_rollback_num].subnode_header.length,&external_img_info[hb_img_rollback_num],sizeof(fota_node_header));
	    	hb_tmp_img_info = &external_img_info[hb_img_rollback_num];
	    }
	    return check_flash_img();
	}
}



void main()
{
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    PRINTF("\r\nThis is bootloader\r\n");

//    check_battery();

    record_reset_reason();

	get_internal_img_info();

	spi_flash_init();
	memset(fota_img_update_state,false,sizeof(fota_img_update_state));
	get_external_img_info(img_addr,sizeof(fota_node_header));


	hb_img_update_flag = false;
    while(check_img_need_update())
    {
    	PRINTF("\r\nSTEP .1 : img need update\r\n");
    	memset(hb_tmp_img,0xff,HB_IMG_TMP_SIZE);
    	if(check_spi_flash_img(hb_img_update_num,hb_tmp_img,img_addr[hb_img_update_num]+HEADER_BLOCK_SIZE,external_img_info[hb_img_update_num].subnode_header.length))
    	{
    		if(write_flash_img(hb_tmp_img,external_img_info[hb_img_update_num].subnode_header.length,&external_img_info[hb_img_update_num],sizeof(fota_node_header)))
    		{
        		hb_img_update_flag = true;
        		break;
    		}
    	}
    	else
    	{
			fota_img_update_state[hb_img_update_num] = false;
    	}
    }

	hb_tmp_img_info = internal_img_info;
    //check internal flash img
    if(!check_flash_img())
    {
    	PRINTF("\r\nERROR: NO HB IMG CAN BE USE\r\n");
    	LED_RED_INIT(LOGIC_LED_OFF);
    	LED_RED_ON();
    }

	uint32_t applicationAddress, stackPointer;
	PRINTF("\r\nSTEP 4 : Jump into app img\r\n");
	get_user_application_entry(&applicationAddress, &stackPointer);
	jump_to_application(applicationAddress, stackPointer);

	return 0;
}
