
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include <freertos/task.h>
#include <esp_sleep.h>
#include <esp_log.h>
#include <esp_mac.h>

#include "context.h"
#include "logger_config.h"
#include "ubx.h"

#include "nvs.h"
#include "nvs_flash.h"

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

int init_rtc() {
    LOG_INFO(TAG, "[%s]", __FUNCTION__);
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        LOG_INFO(TAG, "[%s] INIT NVS partition", __FUNCTION__);
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    } else {
        if(m_context_rtc.RTC_screen_rotation == -1) {
            int8_t val = -1;
            read_rtc(config_items[cfg_screen_rotation], &val);
            if(val > -1) {
                m_context_rtc.RTC_screen_rotation = val;
            }
        }
    }
    return err;
}

int read_rtc(const char *name, void *value) {
    LOG_INFO(TAG, "[%s] name: %s", __FUNCTION__, name ? name : "-");
    if(!name) return -1;
    nvs_handle_t my_handle;
    int err = nvs_open(nvs_namespace, NVS_READONLY, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        err = 1024;
    }
    if (err != 1024){
        err = nvs_get_i8(my_handle, name, (int8_t*)value);
        nvs_close(my_handle);    
        LOG_INFO(TAG, "[%s] get %s %d", __FUNCTION__, name, *(int8_t*)value);
    }
    return err;
}

int write_rtc(const char *name, void *value, size_t len) {
    LOG_INFO(TAG, "[%s] name: %s", __FUNCTION__, name ? name : "-");
    if(!name) return -1;
    nvs_handle_t my_handle;
    int err = nvs_open(nvs_namespace, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
        LOG_INFO(TAG, "[%s] set %s %d", __FUNCTION__, name, *(int8_t*)value);
        err = nvs_set_i8(my_handle, name, *(int8_t*)value);
        err = nvs_commit(my_handle);
        nvs_close(my_handle);
    }
    return err;
}

void g_context_rtc_add_config(context_rtc_t *rtc, logger_config_t *config) {
    LOG_INFO(TAG, "[%s]", __FUNCTION__);
    assert(rtc && config);
    rtc->RTC_Board_Logo = config->screen.board_logo;  // copy RTC memory !!
    rtc->RTC_Sail_Logo = config->screen.sail_logo;    // copy to RTC memory !!
#ifdef USE_CUSTOM_CALIBRATION_VAL
    rtc->RTC_calibration_bat = config->cal_bat <= 1.4 ? config->cal_bat : 1;
#endif
    rtc->RTC_calibration_speed = config->gps.speed_unit == 1 ? 0.0036 : config->gps.speed_unit == 2 ? 0.00194384449 : 0.001;  // 1=m/s, 3.6=km/h, 1.94384449 = knots, speed is now in mm/s
    // rtc->RTC_SLEEP_screen = config->sleep_off_screen % 10;
    // rtc->RTC_OFF_screen = config->sleep_off_screen / 10 % 10;
    strcpy(rtc->RTC_Sleep_txt, config->sleep_info);
    if(config->screen.screen_rotation != rtc->RTC_screen_rotation){
        LOG_INFO(TAG, "[%s] screen rotation change (rtc) %d to (conf) %d", __FUNCTION__, rtc->RTC_screen_rotation, config->screen.screen_rotation);
        rtc->RTC_screen_rotation = config->screen.screen_rotation;
        write_rtc(&(config_items[cfg_screen_rotation][0]), &rtc->RTC_screen_rotation, sizeof(rtc->RTC_screen_rotation));
    }
}

void g_context_ubx_add_config(context_t *ctx, ubx_config_t *config) {
    LOG_INFO(TAG, "[%s]", __FUNCTION__);
    assert(ctx);
    if(!ctx->gps.ublox_config)
        ctx->gps.ublox_config = config;
    assert(ctx->gps.ublox_config);
    ctx->gps.ublox_config->rtc_conf->output_rate = ctx->config->gps.sample_rate;
    ctx->gps.ublox_config->rtc_conf->nav_mode = ctx->config->gps.dynamic_model;
    // ctx->gps.ublox_config->rtc_conf->msgout_sat = ctx->config->log_ubx_nav_sat;
    if(ctx->config->gps.gnss > 5)
        ctx->gps.ublox_config->rtc_conf->gnss = ctx->config->gps.gnss;
}

context_t *g_context_init(context_t *ctx) {
    assert(ctx);
    memset(ctx, 0, sizeof(struct context_s));
    context_t ctxx = CONTEXT_DEFAULT_CONFIG();
    memcpy(ctx, &ctxx, sizeof(struct context_s));
    return ctx;
}

context_t *g_context_defaults(context_t *ctx) {
    assert(ctx);
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
    semVerStr(ctx->SW_version);
    #ifdef CONFIG_DISPLAY_DRIVER_ST7789
    ctx->display_bl_level=90;
    ctx->display_bl_level_set=90;
    #endif
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
    assert(ctx && config);
    if(!ctx->config) {
        ctx->config = config;
    }
    ctx->gps.time_out_gps_msg = (1000 / ctx->gps.ublox_config->rtc_conf->output_rate + 75);  // max time out = 175 ms
    uint16_t screen;                     // preserve value config
    uint8_t screen_count, i, j;
    
    screen = config->screen.stat_screens;
    if(screen>=UINT16_MAX) screen = UINT16_MAX;
    for (i = 0, ctx->stat_screen_count=0; i<16; ++i) {
            ctx->stat_screen[i] =  screen & (1 << i) ? 1 : 0;
            if(ctx->stat_screen[i]) ctx->stat_screen_count++;
    }
    ESP_LOGW(TAG, "[%s], stat screens: count %"PRIu8", screens %"PRIu16, __FUNCTION__, ctx->stat_screen_count, config->screen.stat_screens);

    screen = config->screen.gpio12_screens; 
    if(screen>=UINT8_MAX) screen = UINT8_MAX;
    for (i = 0,ctx->gpio12_screen_count=0; i<16; ++i) {
            ctx->gpio12_screen[i] =  screen & (1 << i) ? 1 : 0;
            if(ctx->gpio12_screen[i]) ctx->gpio12_screen_count++;
    }
    ESP_LOGW(TAG, "[%s], io12 screens: count %"PRIu8", screens %"PRIu16, __FUNCTION__, ctx->gpio12_screen_count, config->screen.gpio12_screens);

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

const char gps_logger_version[] = PROJECT_VER;

uint16_t semVerStr(char * str) {
    uint16_t size = sizeof(gps_logger_version);
    if(str) {
        memcpy(str, gps_logger_version, size);
        str[size]=0;
    }
    return (uint16_t) LOGGER_VERSION;
}

enum ubx_hw_e g_context_get_ubx_hw(context_t *ctx) {
    assert(ctx);
    return ctx->gps.ublox_config->rtc_conf->hw_type;
}
