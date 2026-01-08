#ifndef D3BE6356_4D28_4BE4_B7E0_FB5B1C241348
#define D3BE6356_4D28_4BE4_B7E0_FB5B1C241348

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "sdkconfig.h"
#include <sys/time.h>
#if defined(CONFIG_GPS_LOG_ENABLED)
#include "gps_data.h"
#endif
// #include "config_groups.h"

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

#ifndef PROJECT_VER_NUM_PACKED
#define PROJECT_VER_NUM_PACKED QUOTE_CMD(VERSION_MAJOR)QUOTE_CMD(VERSION_MINOR)QUOTE_CMD(VERSION_PATCH)QUOTE_CMD(VERSION_TWEAK)
#endif

#ifndef PROJECT_VER_PACKED
#if defined(BUILD_MODE_DEV)
#define PROJECT_VER_PACKED PROJECT_VER_NUM_PACKED".dev"
#else
#define PROJECT_VER_PACKED PROJECT_VER_NUM_PACKED
#endif
#endif

#ifndef VER_STR_EXT
#if defined(CONFIG_DISPLAY_DRIVER_ST7789)
#define VER_STR_EXT "st7789"
#else
#if defined(CONFIG_HAS_BOARD_LILYGO_EPAPER_T5_16MB_FLASH)
#if defined(CONFIG_SSD168X_PANEL_SSD1681)
#define VER_STR_EXT "ssd1681-16m"
#elif defined(CONFIG_SSD168X_SCREEN_GDEY0213B74)
#define VER_STR_EXT "gdey0213b74-16m"
#else
#define VER_STR_EXT "16m"
#endif
#else
#if defined(CONFIG_SSD168X_PANEL_SSD1681)
#define VER_STR_EXT "ssd1681"
#elif defined(CONFIG_SSD168X_SCREEN_GDEY0213B74)
#define VER_STR_EXT "gdey0213b74"
#endif
#endif
#endif
#endif

#ifndef PROJECT_VER_EXT
#ifndef VER_STR_EXT
#define PROJECT_VER_EXT PROJECT_VER
#else
#define PROJECT_VER_EXT PROJECT_VER"-"VER_STR_EXT
#endif
#endif

#ifndef PROJECT_VER_PACKED_EXT
#ifndef VER_STR_EXT
#define PROJECT_VER_PACKED_EXT PROJECT_VER_PACKED
#else
#define PROJECT_VER_PACKED_EXT PROJECT_VER_PACKED"-"VER_STR_EXT
#endif
#endif

struct logger_config_s;
struct ubx_ctx_s;

typedef enum {
    IO_BUT_12_STATUS=0,
    IO_BUT_39_STATUS=1
} io_but_status_t;

#define APP_MODE_LIST(l) \
l(UNKNOWN) \
l(BOOT) \
l(WIFI) \
l(GPS) \
l(SLEEP) \
l(CHARGE) \
l(SHUT_DOWN) \
l(RESTART)

#define APP_MODE_ENUM(x) APP_MODE_ ## x,
typedef enum app_mode_s {
    APP_MODE_LIST(APP_MODE_ENUM)
} app_mode_t;

typedef struct context_s {

    // bool sdOK;
    
    
    bool downloading_file;
    bool context_initialized;
    
    // uint8_t request_restart;
    // bool request_shutdown;
    app_mode_t request_app_mode;
    
    uint8_t button;
    uint8_t Field_choice;
    uint8_t Field_choice2;

    uint8_t stat_screen_cur;    // keuze stat scherm indien stilstand
#if defined(CONFIG_LOGGER_BUTTON_GPIO_1)  || defined(CONFIG_UBUTTON_GPIO_1)
    uint8_t gpio12_screen_cur;  // keuze welk scherm
#define CONFIG_GPIO12_SCR_N .gpio12_screen_cur = 0,
#else
#define CONFIG_GPIO12_SCR_N
#endif
    uint8_t _pad1;
    
    uint8_t mac_address[6];     // unique mac adress of esp32
    io_but_status_t io_button_status[4];

    uint32_t last_delay;       // 4bytes
    uint64_t wifi_ap_timeout;  // 8bytes

    char SW_version[32];

    // REMOVED: struct logger_config_s *config; - Use g_rtc_config instead
    // struct context_rtc_s *rtc;
#ifdef CONFIG_GPS_LOG_ENABLED
    struct gps_context_s gps;
#else
    void gps;
#define CONTEXT_GPS_DEFAULT_CONFIG() {0}
#endif
    uint8_t firmware_update_started;
    uint32_t fw_update_postponed;
    uint8_t fw_update_is_allowed;
} context_t;

#define CONTEXT_DEFAULT_CONFIG() { \
        .downloading_file = false, \
        .context_initialized = false, \
        .request_app_mode = APP_MODE_UNKNOWN, \
        .stat_screen_cur = 0,    \
        CONFIG_GPIO12_SCR_N  \
        .mac_address = {0},      \
        .io_button_status = {0}, \
        .last_delay = 0,         \
        .wifi_ap_timeout = 0,    \
        .SW_version = PROJECT_VER,   \
        .gps = CONTEXT_GPS_DEFAULT_CONFIG(), \
        .fw_update_postponed = 0, \
        .fw_update_is_allowed = 0, \
        .firmware_update_started = 0, \
    }

context_t *g_context_init(context_t *ctx);
context_t *g_context_defaults(context_t *ctx);

enum ubx_hw_e;
enum ubx_hw_e g_context_get_ubx_hw(context_t *ctx);


#define SEM_VER_LEN 11
uint16_t semVer();
uint8_t semVerMajor();
uint8_t semVerMinor();
uint8_t semVerPatch();
uint8_t semVerBuild();
uint16_t semVerStr(char *str, bool packed);
#ifdef __cplusplus
}
#endif
#endif /* D3BE6356_4D28_4BE4_B7E0_FB5B1C241348 */
