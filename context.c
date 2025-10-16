
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
#include "logger_config.h"
#include "ubx.h"
// #include "gps_user_cfg.h"

//extern struct config_s * m_config;
static const char *TAG = "context";

RTC_DATA_ATTR context_rtc_t m_context_rtc = CONTEXT_RTC_DEFAULT_CONFIG();
context_t m_context = CONTEXT_DEFAULT_CONFIG();

/* context_rtc_t *g_context_rtc_init(context_rtc_t *rtc) {
    assert(rtc);
    context_rtc_t rtcx = CONTEXT_RTC_DEFAULT_CONFIG();
    memcpy(rtc, &rtcx, sizeof(context_rtc_t));
    return rtc;
}; */

static const char *nvs_namespace = "logger_ctx";

static int read_rtc_i8(const char *ns, const char *name, void *value) {
    LOG_INFO(TAG, "[%s] name: %s", __FUNCTION__, name ? name : "-");
    if(!name) return -1;
    nvs_handle_t my_handle;
    int err = nvs_open(ns, NVS_READONLY, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        err = 1024;
    }
    if (err != 1024){
        err = nvs_get_i8(my_handle, name, (int8_t*)value);
        nvs_close(my_handle);
#if (C_LOG_LEVEL < 2) 
        LOG_INFO(TAG, "[%s] get %s %d", __FUNCTION__, name, *(int8_t*)value);
#endif
    }
    return err;
}

static int write_rtc_i8(const char *ns, const char *name, void *value, size_t len) {
    LOG_INFO(TAG, "[%s] name: %s", __FUNCTION__, name ? name : "-");
    if(!name) return -1;
    nvs_handle_t my_handle;
    int err = nvs_open(ns, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
#if (C_LOG_LEVEL < 2)
        LOG_INFO(TAG, "[%s] set %s %d", __FUNCTION__, name, *(int8_t*)value);
#endif
        err = nvs_set_i8(my_handle, name, *(int8_t*)value);
        err = nvs_commit(my_handle);
        nvs_close(my_handle);
    }
    return err;
}

int nvs_init() {
    LOG_INFO(TAG, "[%s]", __FUNCTION__);
    if(m_context.nvs_initialized) return ESP_OK;
    int ret = ESP_OK;
        ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ret = nvs_flash_erase();
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "esp_flash_erase failed: %s", esp_err_to_name(ret));
        }
        ret = nvs_flash_init();
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "esp_flash_init failed: %s", esp_err_to_name(ret));
        }
        m_context.nvs_initialized = true;
    } else if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_flash_init failed: %s", esp_err_to_name(ret));
    }
    return ret;
}

int init_rtc() {
#if (C_LOG_LEVEL < 3)
    ILOG(TAG, "[%s]", __FUNCTION__);
#endif
    esp_err_t err = nvs_init();
    if (!err) {
        if(m_context_rtc.RTC_screen_rotation == -1) {
            int8_t val = -1;
            read_rtc_i8(nvs_namespace, config_items[cfg_screen_rotation], &val);
            if(val > -1) {
                m_context_rtc.RTC_screen_rotation = val;
            }
        }
#if !defined(CONFIG_LCD_IS_EPD)
        if(m_context_rtc.RTC_screen_brightness == -1) {
            int8_t val = -1;
            read_rtc_i8(nvs_namespace, config_items[cfg_screen_brightness], &val);
            if(val > -1) {
                m_context_rtc.RTC_screen_brightness = val;
            }
        }
#endif
    }
    return err;
}

void g_context_rtc_add_config(context_rtc_t *rtc, logger_config_t *config) {
    LOG_INFO(TAG, "[%s]", __FUNCTION__);
    if(!rtc || !config) return;
    rtc->RTC_Board_Logo = config->screen.board_logo;  // copy RTC memory !!
    rtc->RTC_Sail_Logo = config->screen.sail_logo;    // copy to RTC memory !!
    rtc->bat_view = config->screen.bat_view;
    // rtc->RTC_SLEEP_screen = config->sleep_off_screen % 10;
    // rtc->RTC_OFF_screen = config->sleep_off_screen / 10 % 10;
    strcpy(rtc->RTC_Sleep_txt, config->sleep_info);
    if(config->screen.screen_rotation != rtc->RTC_screen_rotation){
#if (C_LOG_LEVEL < 2)
        LOG_INFO(TAG, "[%s] screen rotation change (rtc) %d to (conf) %d", __FUNCTION__, rtc->RTC_screen_rotation, config->screen.screen_rotation);
#endif
        rtc->RTC_screen_rotation = config->screen.screen_rotation;
        write_rtc_i8(nvs_namespace, &(config_items[cfg_screen_rotation][0]), &rtc->RTC_screen_rotation, sizeof(rtc->RTC_screen_rotation));
    }
#if !defined(CONFIG_LCD_IS_EPD)
    if(config->screen_brightness != rtc->RTC_screen_brightness){
    #if (C_LOG_LEVEL < 2)
        LOG_INFO(TAG, "[%s] screen brightness change (rtc) %d to (conf) %d", __FUNCTION__, rtc->RTC_screen_brightness, config->screen_brightness);
#endif
        rtc->RTC_screen_brightness = config->screen_brightness;
        write_rtc_i8(nvs_namespace, &(config_items[cfg_screen_brightness][0]), &rtc->RTC_screen_brightness, sizeof(rtc->RTC_screen_brightness));
    }
#endif
}

// void g_context_ubx_add_config(context_t *ctx, ubx_config_t *config) {
//     LOG_INFO(TAG, "[%s]", __FUNCTION__);
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
        ESP_LOGE(TAG, "Get base MAC address from BLK3 of EFUSE error (%s)", esp_err_to_name(err));
    }
    ctx->gps.SW_version = &(ctx->SW_version[0]);
    ctx->gps.mac_address =  &(ctx->mac_address[0]);
    ctx->rtc = &m_context_rtc;
    semVerStr(ctx->SW_version, false);
    ctx->context_initialized = 1;
    return ctx;
}

uint8_t lenHelper(unsigned x) { 
    if(x >= 100000u) {
        if(x >= 10000000u) {
            if(x >= 1000000000u) return 10;
            if(x >= 100000000u) return 9;
            return 8;
        }
        if(x >= 1000000u) return 7;
        return 6;
    } else {
        if(x >= 1000u) {
            if(x >= 10000u) return 5;
            return 4;
        } else {
            if(x >= 100u) return 3;
            if(x >= 10u) return 2;
            return 1;
        }
    }
}

context_t *g_context_add_config(context_t *ctx, logger_config_t *config) {
    if(!ctx || !config) return NULL;
    if(!ctx->config) {
        ctx->config = config;
    }
    ctx->config = config;
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
    return ctx->gps.ubx_device->rtc_conf->hw_type;
}
