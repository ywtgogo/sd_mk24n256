#ifndef _SIMPLE_CONSOLE_H_
#define _SIMPLE_CONSOLE_H_

#define CONFIG_SYS_CBSIZE 64
#define CONFIG_SYS_MAXARGS 5

struct cmd_tbl_s {
        char            *name;          /* Command Name                 */
        int             maxargs;        /* maximum number of arguments  */
        int             (*cmd)(struct cmd_tbl_s *, int, int, char * const []);
        char            *usage;         /* Usage message        (short) */
};

struct test_tbl_s {
        char            *name;          /* test Name                 */
        int             maxargs;        /* maximum number of arguments  */
        int				testresult;
        int             (*runtest)(struct test_tbl_s *, int, int, char * const []);
        int             (*showstate)(struct test_tbl_s *, int, int, char * const []);
        int             (*setup)(struct test_tbl_s *, int, int, char * const []);
        char            *usage;         /* Usage message        (short) */
};

int readline_to_buffer(char *buffer);
int simple_run_command(char *cmd, int flag);

static int do_printenv(struct cmd_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[]);
static int do_help(struct cmd_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[]);
static int do_fota(struct cmd_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[]);
static int do_query(struct cmd_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[]);
static int do_reset(struct cmd_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[]);
static int do_get_sensor_list(struct cmd_tbl_s *cmdtp, int flag, int argc,
					   char * const argv[]);
static int do_list_sensor(struct cmd_tbl_s *cmdtp, int flag, int argc,
					   char * const argv[]);
static int do_send_fw_result(struct cmd_tbl_s *cmdtp, int flag, int argc,
					   char * const argv[]);
static int do_update_hb_heartbeat(struct cmd_tbl_s *cmdtp, int flag, int argc,
					   char * const argv[]);
static int do_update_hbb_id(struct cmd_tbl_s *cmdtp, int flag, int argc,
					   char * const argv[]);
static int do_install_sensor(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[]);
int do_install_sensor_lp(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[]);
static int do_get_attr(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[]);
static int do_set_attr(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[]);
static int do_hb_heartbeat(struct cmd_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[]);
static int do_heartbeat(struct cmd_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[]);
static int do_get_task_state(struct cmd_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[]);
static int do_hb_event(struct cmd_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[]);
static int do_sensor_event(struct cmd_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[]);
static int do_ifdown(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[]);
static int do_ifup(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[]);
static int do_hb_defer_log(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[]);
static int do_ppp(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[]);
static int do_ppp_relink(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[]);
static int do_gsm_sign(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[]);
static int do_hb_sys_log(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[]);
static int do_hb_erase_sector(struct cmd_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[]);
static int do_hb_erase_log_sector(struct cmd_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[]);
static int do_hb_erase_defer_sector(struct cmd_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[]);
static int do_hb_erase_spinor(struct cmd_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[]);
static int do_read_spi_flash(struct cmd_tbl_s *cmdtp, int flag, int argc,
        char * const argv[]);
static int do_hb_dump_log(struct cmd_tbl_s *cmdtp, int flag, int argc,
					   char * const argv[]);
static int do_hb_up_log(struct cmd_tbl_s *cmdtp, int flag, int argc,
					   char * const argv[]);
static int do_scan(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[]);
static int do_send_data(struct cmd_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[]);
static int do_save_hbb(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[]);
static int do_erase_hbb(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[]);
static int do_save_gprs_apn(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[]);
static int do_show_hbb_info(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[]);
static int do_erase_sensor_list(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[]);
static int do_update_rf_config(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[]);
static int do_testmode(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[]);
static int do_hb_erase_img2_test(struct cmd_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[]);
static int do_hb_erase_img3_test(struct cmd_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[]);
static int do_hb_erase_internal_test(struct cmd_tbl_s *cmdtp, int flag, int argc,
                       char * const argv[]);

#ifdef WIFI_MODULE
static int do_get_wifi_rssi(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[]);

static int do_get_wifi_ver(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[]);

static int do_get_wifi_ssid(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[]);


#endif
#ifdef PLC
static int do_sync_bridge_rtc_time(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[]);
static int do_query_bridge(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[]);
static int do_disengage_bridge_sensor(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[]);
static int do_engage_bridge_sensor(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[]);
static int do_set_plc_pir_low_sens(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[]);
static int do_set_plc_pir_high_sens(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[]);
static int do_sent_sensor_list_to_bridge(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[]);
static int do_deploy_sensor_list_to_bridge(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[]);
static int do_plc_fw_need_update(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[]);
static int do_plc_request_img_block_ack(struct cmd_tbl_s *cmdtp, int flag, int argc,
							  char * const argv[]);
#endif
#endif /* _SIMPLE_CONSOLE_ */
