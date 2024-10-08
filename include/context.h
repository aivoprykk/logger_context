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

#if !defined(VERSION_MAJOR)
#define VERSION_MAJOR 1
#endif

#if !defined(VERSION_MINOR)
#define VERSION_MINOR 0
#endif

#if !defined(VERSION_PATCH)
#define VERSION_PATCH 4
#endif

#if !defined(VERSION_TWEAK)
#define VERSION_TWEAK 2
#endif

#if !defined(PROJECT_VER)
#if defined(BUILD_MODE_DEV)
#define PROJECT_VER QUOTE_CMD(VERSION_MAJOR)"."QUOTE_CMD(VERSION_MINOR)"."QUOTE_CMD(VERSION_PATCH)"."QUOTE_CMD(VERSION_TWEAK)".dev"
#else
#define PROJECT_VER QUOTE_CMD(VERSION_MAJOR)"."QUOTE_CMD(VERSION_MINOR)"."QUOTE_CMD(VERSION_PATCH)"."QUOTE_CMD(VERSION_TWEAK)
#endif
#endif
#ifndef LOGGER_VERSION
#define LOGGER_VERSION (VERSION_MAJOR * 1000 + VERSION_MINOR * 100 + VERSION_PATCH * 10 + VERSION_TWEAK)
#endif

struct logger_config_s;
struct ubx_config_s;

typedef struct context_rtc_s {
    uint8_t version;

    uint8_t RTC_Board_Logo;
    uint8_t RTC_Sail_Logo;
    // uint8_t RTC_SLEEP_screen;
    // uint8_t RTC_OFF_screen;

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
    int8_t RTC_screen_rotation;
    uint8_t RTC_screen_auto_refresh;
} context_rtc_t;

#if defined(CONFIG_DISPLAY_DRIVER_ST7789)
#if !defined(SCR_DEFAULT_ROTATION)
#define SCR_DEFAULT_ROTATION 2 // 270deg
#endif
#if !defined(SCR_AUTO_REFRESH)
#define SCR_AUTO_REFRESH 1
#endif
#else
#if !defined(SCR_DEFAULT_ROTATION)
#define SCR_DEFAULT_ROTATION 1 // 90deg
#endif
#if !defined(SCR_AUTO_REFRESH)
#define SCR_AUTO_REFRESH 0
#endif
#endif

#define CONTEXT_RTC_DEFAULT_CONFIG() \
    (context_rtc_t) {                                 \
        .version = 1,                 \
        .RTC_Board_Logo = 1,           \
        .RTC_Sail_Logo = 1,            \
        .RTC_counter = 0,              \
        .RTC_offset = 0,               \
        .RTC_year = 0,                 \
        .RTC_month = 0,                \
        .RTC_day = 0,                  \
        .RTC_hour = 0,                 \
        .RTC_min = 0,                  \
        .RTC_calibration_speed = 0.0036,    \
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
        .RTC_screen_rotation = -1,      \
        .RTC_screen_auto_refresh = SCR_AUTO_REFRESH,      \
    }

context_rtc_t *g_context_rtc_init(context_rtc_t *rtc);
context_rtc_t *g_context_rtc_defaults(context_rtc_t *rtc);
void g_context_rtc_add_config(context_rtc_t *rtc, struct logger_config_s *config);
int write_rtc(const char *name, void *value, size_t len);
int read_rtc(const char *name, void *value);
int init_rtc();

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
    bool request_shutdown;
    bool logs_enabled;

    uint8_t button;
    uint8_t reed;
    uint8_t Field_choice;
    uint8_t Field_choice2;

    uint8_t stat_screen_count;
    uint8_t gpio12_screen_count;
    uint8_t stat_screen_cur;    // keuze stat scherm indien stilstand
    
    uint8_t gpio12_screen_cur;  // keuze welk scherm
    uint8_t _pad1;
    
    uint8_t stat_screen[16];    // which stat_screen you want to see ?
    uint8_t gpio12_screen[16];  // which stat_screen when gpio 12 toggles ?  

    uint8_t mac_address[6];     // unique mac adress of esp32
    io_but_status_t io_button_status[4];

    uint32_t last_delay;       // 4bytes
    uint64_t wifi_ap_timeout;  // 8bytes

    int low_bat_count;
    uint32_t freeSpace;

    char SW_version[16];

    char config_file_path[32];
    struct logger_config_s *config;
    struct context_rtc_s *rtc;
    struct gps_context_s gps;
#ifdef CONFIG_DISPLAY_DRIVER_ST7789
    uint8_t display_bl_level;
    uint8_t display_bl_level_set;
#endif
    uint8_t firmware_update_started;
    uint32_t fw_update_postponed;
    uint8_t fw_update_is_allowed;
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
        .request_shutdown = false, \
        .logs_enabled = false,   \
        .button = 0,             \
        .reed = 0,               \
        .Field_choice = 0,       \
        .Field_choice2 = 0,      \
        .stat_screen_count = 0,  \
        .gpio12_screen_count = 0, \
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
        .SW_version = PROJECT_VER,   \
        .config_file_path = {0}, \
        .config = NULL,          \
        .rtc = NULL,             \
        .gps = CONTEXT_GPS_DEFAULT_CONFIG, \
        .fw_update_postponed = 0, \
        .fw_update_is_allowed = 0, \
        .firmware_update_started = false, \
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
