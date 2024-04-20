#ifndef D3BE6356_4D28_4BE4_B7E0_FB5B1C241348
#define D3BE6356_4D28_4BE4_B7E0_FB5B1C241348

#include <stdint.h>
#include <stdbool.h>
#include "sdkconfig.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/time.h>
#include "gps_data.h"
#include "logger_common.h"

#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_PATCH 1

#if defined(DEBUG)
#define VERSION_BUILD 1
#define VERSION_STRING "v"QUOTE_CMD(VERSION_MAJOR)"."QUOTE_CMD(VERSION_MINOR)"."QUOTE_CMD(VERSION_PATCH)"-rc."QUOTE_CMD(VERSION_BUILD)
#else
#define VERSION_STRING "v"QUOTE_CMD(VERSION_MAJOR)"."QUOTE_CMD(VERSION_MINOR)"."QUOTE_CMD(VERSION_PATCH)
#endif
#ifndef VERSION
#if !defined(DEBUG)
#define VERSION (VERSION_MAJOR * 100 + VERSION_MINOR * 10 + VERSION_PATCH)
#else
#define VERSION (VERSION_MAJOR * 1000 + VERSION_MINOR * 100 + VERSION_PATCH * 10 + VERSION_BUILD)
#endif
#endif

struct logger_config_s;
struct ubx_config_s;

typedef struct context_rtc_s {
    uint8_t version;

    uint8_t RTC_Board_Logo;
    uint8_t RTC_Sail_Logo;
    uint8_t RTC_SLEEP_screen;
    uint8_t RTC_OFF_screen;

    uint8_t RTC_counter;

    int16_t RTC_offset;

    int16_t RTC_year;   // 2
    int16_t RTC_month;  // 2
    int16_t RTC_day;    // 2
    int16_t RTC_hour;   // 2

    int16_t RTC_min;  // 2
    // uint16_t _pad1; //2
    // uint32_t _pad2; //4

    float RTC_calibration_speed;
    float RTC_distance;
    float RTC_avg_10s;
    float RTC_max_2s;

    // Simon
    float RTC_alp;
    float RTC_500m;
    float RTC_1h;
    float RTC_mile;

    float RTC_R1_10s;
    float RTC_R2_10s;
    float RTC_R3_10s;
    float RTC_R4_10s;

    float RTC_R5_10s;
    // Simon
#ifdef USE_CUSTOM_CALIBRATION_VAL
    float RTC_calibration_bat;  // was 1.75| bij ontwaken uit deepsleep
                                // niet noodzakelijk config file lezen
#endif
    float RTC_voltage_bat;
    // float _pad3;

    char RTC_Sleep_txt[32];
} context_rtc_t;

#define CONTEXT_RTC_DEFAULT_CONFIG() \
    (context_rtc_t) {                                 \
        .version = 1,                 \
        .RTC_Board_Logo = 1,           \
        .RTC_Sail_Logo = 1,            \
        .RTC_SLEEP_screen = 11 % 10,         \
        .RTC_OFF_screen = 11 / 10 % 10,           \
        .RTC_counter = 0,              \
        .RTC_offset = 0,               \
        .RTC_year = 0,                 \
        .RTC_month = 0,                \
        .RTC_day = 0,                  \
        .RTC_hour = 0,                 \
        .RTC_min = 0,                  \
        .RTC_calibration_speed = 3.6/1000,    \
        .RTC_distance = 0,             \
        .RTC_avg_10s = 0,              \
        .RTC_max_2s = 0,               \
        .RTC_alp = 0,                  \
        .RTC_500m = 0,                 \
        .RTC_1h = 0,                   \
        .RTC_mile = 0,                 \
        .RTC_R1_10s = 0,               \
        .RTC_R2_10s = 0,               \
        .RTC_R3_10s = 0,               \
        .RTC_R4_10s = 0,               \
        .RTC_R5_10s = 0,               \
        .RTC_voltage_bat = 3.6,          \
        .RTC_Sleep_txt = "Your ID",          \
    }

context_rtc_t *g_context_rtc_init(context_rtc_t *rtc);
context_rtc_t *g_context_rtc_defaults(context_rtc_t *rtc);
void g_context_rtc_add_config(context_rtc_t *rtc, struct logger_config_s *config);
int write_rtc(struct context_rtc_s *rtc);
int read_rtc(struct context_rtc_s *rtc);

typedef enum {
    IO_BUT_12_STATUS=0,
    IO_BUT_39_STATUS=1
} io_but_status_t;

typedef struct context_s {

    bool sdTrouble;
    bool sdOK;
    bool NTP_time_set;

    bool Shut_down_Save_session;

    bool ftpStatus;
    bool downloading_file;
    bool context_initialized;
    
    bool request_restart;
    bool firmware_update_started;
    bool deep_sleep;
    bool long_push;
    bool logs_enabled;

    uint8_t button;
    uint8_t reed;
    uint8_t Field_choice;
    uint8_t Field_choice2;

    uint8_t stat_screen_count;
    uint8_t gpio12_screen_count;
    uint8_t boot_screen_stage;
    uint8_t stat_screen_cur;    // keuze stat scherm indien stilstand
    
    uint8_t gpio12_screen_cur;  // keuze welk scherm
    uint8_t _pad1;
    
    uint8_t stat_screen[12];    // which stat_screen you want to see ?
    uint8_t gpio12_screen[12];  // which stat_screen when gpio 12 toggles ?  

    uint8_t mac_address[6];     // unique mac adress of esp32
    io_but_status_t io_button_status[4];

    uint32_t last_delay;       // 4bytes
    uint64_t wifi_ap_timeout;  // 8bytes

    int low_bat_count;
    uint32_t freeSpace;


#ifdef CONFIG_BUTTON_OLD_BEHAIVIOR
    struct Button_push
        Short_push12;  // (12,100,15,1); //GPIO12 pull up, 100ms push time, 15s
                       // long_pulse, count 1, STAT screen 4&5
    struct Button_push
        Long_push12;                  // (12,2000,10,4); //GPIO12 pull up, 2000ms push time, 10s
                                      // long_pulse, count 4, reset STAT screen 4&5
    struct Button_push Short_push39;  // (WAKE_UP_GPIO,100,10,8);//was 39
    struct Button_push Long_push39;   // (WAKE_UP_GPIO,1500,10,8);//was 39
#endif
    char SW_version[16];

    const char *filename;
    const char *filename_backup;
    char config_file_path[32];
    struct logger_config_s *config;
    struct context_rtc_s *rtc;
    struct gps_context_s gps;
#ifdef CONFIG_DISPLAY_DRIVER_ST7789
    uint8_t display_bl_level;
    uint8_t display_bl_level_set;
#endif
} context_t;

#define CONTEXT_DEFAULT_CONFIG() (context_t){ \
        .sdTrouble = false,      \
        .sdOK = false,           \
        .NTP_time_set = false,   \
        .Shut_down_Save_session = false, \
        .ftpStatus = false,      \
        .downloading_file = false, \
        .context_initialized = false, \
        .request_restart = false, \
        .firmware_update_started = false, \
        .deep_sleep = false,     \
        .long_push = false,      \
        .logs_enabled = false,   \
        .button = 0,             \
        .reed = 0,               \
        .Field_choice = 0,       \
        .Field_choice2 = 0,      \
        .stat_screen_count = 0,  \
        .gpio12_screen_count = 0, \
        .boot_screen_stage = 0,  \
        .stat_screen_cur = 0,    \
        .gpio12_screen_cur = 0,  \
        .stat_screen = {0},      \
        .gpio12_screen = {0},    \
        .mac_address = {0},      \
        .io_button_status = {0}, \
        .last_delay = 0,         \
        .wifi_ap_timeout = 0,    \
        .low_bat_count = 0,      \
        .freeSpace = 0,          \
        .SW_version = "1.0.0",   \
        .filename = "config.txt",        \
        .filename_backup = "config_backup.txt", \
        .config_file_path = {0}, \
        .config = NULL,          \
        .rtc = NULL,             \
        .gps = CONTEXT_GPS_DEFAULT_CONFIG, \
    }

context_t *g_context_init(context_t *ctx);
context_t *g_context_defaults(context_t *ctx);
context_t *g_context_add_config(context_t *ctx, struct logger_config_s *);
void g_context_ubx_add_config(context_t *ctx, struct ubx_config_s *);

enum ubx_hw_e;
enum ubx_hw_e g_context_get_ubx_hw(context_t *ctx);


#define SEM_VER_LEN 11
uint16_t semVer();
uint8_t semVerMajor();
uint8_t semVerMinor();
uint8_t semVerPatch();
uint8_t semVerBuild();
uint16_t semVerStr(char *str);
#ifdef __cplusplus
}
#endif
#endif /* D3BE6356_4D28_4BE4_B7E0_FB5B1C241348 */
