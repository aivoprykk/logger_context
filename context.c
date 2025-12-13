
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include <freertos/task.h>
#include <esp_sleep.h>
#include <esp_mac.h>

#include "nvs.h"
#include "nvs_flash.h"

#include "sdkconfig.h"
#if (defined(CONFIG_LOGGER_USE_GLOBAL_LOG_LEVEL) && CONFIG_LOGGER_GLOBAL_LOG_LEVEL < CONFIG_LOGGER_COMMON_LOG_LEVEL)
#define C_LOG_LEVEL CONFIG_LOGGER_GLOBAL_LOG_LEVEL
#else
#define C_LOG_LEVEL CONFIG_LOGGER_COMMON_LOG_LEVEL
#endif
#include "common_log.h"

#include "logger_common.h"
#include "context.h"
// #include "logger_config.h"
#include "config_manager.h"
#include "unified_config.h"
#include "ubx.h"

// #include "gps_user_cfg.h"

//extern struct config_s * m_config;
static const char *TAG = "context";

// GPS session state in RTC memory (survives deep sleep)

// Main application context
context_t m_context = CONTEXT_DEFAULT_CONFIG();

// void g_context_ubx_add_config(context_t *ctx, ubx_ctx_t *config) {
//     ILOG(TAG, "[%s]", __FUNCTION__);
//     assert(ctx);
//     if(!ctx->gps.ubx_device)
//         ctx->gps.ubx_device = config;
//     assert(ctx->gps.ubx_device);
//     ctx->gps.ubx_device->rtc_conf->output_rate = ctx->config->gps.sample_rate;
//     ctx->gps.ubx_device->rtc_conf->nav_mode = ctx->config->gps.dynamic_model;
//     // ctx->gps.ubx_device->rtc_conf->msgout_sat = ctx->config->log_ubx_nav_sat;
//     if(ctx->config->gps.gnss > 5)
//         ctx->gps.ubx_device->rtc_conf->gnss = ctx->config->gps.gnss;
// }

context_t *g_context_init(context_t *ctx) {
    if(!ctx) return NULL;
    memset(ctx, 0, sizeof(struct context_s));
    context_t ctxx = CONTEXT_DEFAULT_CONFIG();
    memcpy(ctx, &ctxx, sizeof(struct context_s));
    return ctx;
}

context_t *g_context_defaults(context_t *ctx) {
    if(!ctx) return NULL;
    if (ctx->context_initialized)
        return ctx;
    //g_context_init(ctx);
    esp_err_t err = esp_efuse_mac_get_default(&(ctx->mac_address[0]));
    if (err != ESP_OK) {
        ELOG(TAG, "Get base MAC address from BLK3 of EFUSE error (%s)", esp_err_to_name(err));
    }
    ctx->gps.SW_version = &(ctx->SW_version[0]);
    ctx->gps.mac_address =  &(ctx->mac_address[0]);
    // Point to GPS session state (RTC memory, survives deep sleep)
    // Can be saved/loaded to NVS for power cycle persistence
    // ctx->rtc = &g_context_rtc;
    semVerStr(ctx->SW_version, false);
    ctx->context_initialized = 1;
    return ctx;
}

uint16_t semVer() {
    return (uint16_t) LOGGER_VERSION;
}
uint8_t semVerMajor() {
    return (uint8_t) VERSION_MAJOR;
}
uint8_t semVerMinor() {
    return (uint8_t) VERSION_MINOR;
}
uint8_t semVerPatch() {
    return (uint8_t) VERSION_PATCH;
}
uint8_t semVerBuild() {
    return (uint8_t) VERSION_TWEAK;
}

static const char gps_logger_version[] = PROJECT_VER;
static const char gps_logger_version_packed[] = PROJECT_VER_PACKED;

uint16_t semVerStr(char * str, bool packed) {
    if(!str) return UINT16_MAX;
    uint16_t size = sizeof(packed ? gps_logger_version_packed : gps_logger_version)-1, initial_size = size;
    memcpy(str, packed ? gps_logger_version_packed : gps_logger_version, size);
    str[size]=0;
    return (uint16_t)( size - initial_size );
}

enum ubx_hw_e g_context_get_ubx_hw(context_t *ctx) {
    if(!ctx) return UBX_TYPE_M0;
    return ctx->gps.ubx_device->hw_type;
}
