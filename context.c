
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include <freertos/task.h>
#include <esp_sleep.h>
#include <esp_log.h>
#include <esp_mac.h>

#include "context.h"
#include "logger_config.h"
//#include "logger_common.h"
#include "ubx.h"

#ifndef CONFIG_SOC_RTC_FAST_MEM_SUPPORTED
#include "nvs.h"
#endif

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

#ifndef CONFIG_SOC_RTC_FAST_MEM_SUPPORTED

#include "str.h"
static const char *nvs_namespace = "storage";

int read_rtc(context_rtc_t *rtc) {
    assert(rtc);
    nvs_handle_t my_handle;
    int err = nvs_open(nvs_namespace, NVS_READONLY, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        err = 1024;
    }
    char tmp[16];
    size_t tlen = 16, len = 32;
    if (err != 1024)
        err = nvs_get_blob(my_handle, "RTC_calibration_speed", NULL, &tlen);
    if (err)
        rtc->RTC_calibration_speed = 3.6 / 1000;
    else {
        err = nvs_get_blob(my_handle, "RTC_calibration_speed", tmp, &tlen);
        rtc->RTC_calibration_speed = atof(tmp);
    }
    if (err != 1024)
        err = nvs_get_str(my_handle, "RTC_Sleep_txt", &(rtc->RTC_Sleep_txt[0]),
                          &(len));
    if (err)
        strcpy(rtc->RTC_Sleep_txt, "Your ID");
    if (err != 1024)
        err = nvs_get_blob(my_handle, "RTC_voltage_bat", 0, &tlen);
    if (err)
        rtc->RTC_voltage_bat = 3.6;
    else {
        err = nvs_get_blob(my_handle, "RTC_voltage_bat", tmp, &tlen);
        rtc->RTC_voltage_bat = atof(tmp);
    }
    // Simon
    if (err != 1024)
        err = nvs_get_blob(my_handle, "RTC_calibration_bat", 0, &tlen);
    if (err)
        rtc->RTC_calibration_bat =
            0.99;  // was 1.75| bij ontwaken uit deepsleep
                   // niet noodzakelijk config file lezen
    else {
        err = nvs_get_blob(my_handle, "RTC_calibration_bat", tmp, &tlen);
        rtc->RTC_calibration_bat = atof(tmp);
    }
    if (err != 1024)
        err = nvs_get_u8(my_handle, "RTC_Board_Logo", &rtc->RTC_Board_Logo);
    if (err)
        rtc->RTC_Board_Logo = 1;  // copy RTC memory !!
    if (err != 1024)
        err = nvs_get_u8(my_handle, "RTC_Sail_Logo", &rtc->RTC_Sail_Logo);
    if (err)
        rtc->RTC_Sail_Logo = 1;  // copy to RTC memory !!
    if (err != 1024)
        err = nvs_get_u8(my_handle, "RTC_SLEEP_screen", &rtc->RTC_SLEEP_screen);
    if (err)
        rtc->RTC_SLEEP_screen = 11 % 10;
    if (err != 1024)
        err = nvs_get_u8(my_handle, "RTC_OFF_screen", &rtc->RTC_OFF_screen);
    if (err)
        rtc->RTC_OFF_screen = 11 / 10 % 10;
    if (err != 1024)
        err = nvs_get_i16(my_handle, "RTC_offset", &rtc->RTC_offset);
    if (err)
        rtc->RTC_offset = 0;

    if (err != 1024)
        err = nvs_get_blob(my_handle, "RTC_distance", 0, &tlen);
    if (err)
        rtc->RTC_distance = 0;
    else {
        err = nvs_get_blob(my_handle, "RTC_distance", tmp, &tlen);
        rtc->RTC_distance = atof(tmp);
    }

    if (err != 1024)
        err = nvs_get_blob(my_handle, "RTC_avg_10s", 0, &tlen);
    if (err)
        rtc->RTC_avg_10s = 0;
    else {
        err = nvs_get_blob(my_handle, "RTC_avg_10s", tmp, &tlen);
        rtc->RTC_avg_10s = atof(tmp);
    }

    if (err != 1024)
        err = nvs_get_blob(my_handle, "RTC_max_2s", 0, &tlen);
    if (err)
        rtc->RTC_max_2s = 0;
    else {
        err = nvs_get_blob(my_handle, "RTC_max_2s", tmp, &tlen);
        rtc->RTC_max_2s = atof(tmp);
    }
    if (err != 1024)
        err = nvs_get_i16(my_handle, "RTC_year", &rtc->RTC_year);
    if (err)
        rtc->RTC_year = 0;
    if (err != 1024)
        err = nvs_get_i16(my_handle, "RTC_month", &rtc->RTC_month);
    if (err)
        rtc->RTC_month = 0;
    if (err != 1024)
        err = nvs_get_i16(my_handle, "RTC_day", &rtc->RTC_day);
    if (err)
        rtc->RTC_day = 0;
    if (err != 1024)
        err = nvs_get_i16(my_handle, "RTC_hour", &rtc->RTC_hour);
    if (err)
        rtc->RTC_hour = 0;
    if (err != 1024)
        err = nvs_get_i16(my_handle, "RTC_min", &rtc->RTC_min);
    if (err)
        rtc->RTC_min = 0;
    if (err != 1024)
        err = nvs_get_blob(my_handle, "RTC_alp", 0, &tlen);
    if (err)
        rtc->RTC_alp = 0;
    else {
        err = nvs_get_blob(my_handle, "RTC_alp", tmp, &tlen);
        rtc->RTC_alp = atof(tmp);
    }

    if (err != 1024)
        err = nvs_get_blob(my_handle, "RTC_500m", 0, &tlen);
    if (err)
        rtc->RTC_500m = 0;
    else {
        err = nvs_get_blob(my_handle, "RTC_500m", tmp, &tlen);
        rtc->RTC_500m = atof(tmp);
    }

    if (err != 1024)
        err = nvs_get_blob(my_handle, "RTC_1h", 0, &tlen);
    if (err)
        rtc->RTC_1h = 0;
    else {
        err = nvs_get_blob(my_handle, "RTC_1h", tmp, &tlen);
        rtc->RTC_1h = atof(tmp);
    }

    if (err != 1024)
        err = nvs_get_blob(my_handle, "RTC_mile", 0, &tlen);
    if (err)
        rtc->RTC_mile = 0;
    else {
        err = nvs_get_blob(my_handle, "RTC_mile", tmp, &tlen);
        rtc->RTC_mile = atof(tmp);
    }
    if (err != 1024)
        err = nvs_get_blob(my_handle, "RTC_R1_10s", 0, &tlen);
    if (err)
        rtc->RTC_R1_10s = 0;
    else {
        err = nvs_get_blob(my_handle, "RTC_R1_10s", tmp, &tlen);
        rtc->RTC_R1_10s = atof(tmp);
    }
    if (err != 1024)
        err = nvs_get_blob(my_handle, "RTC_R2_10s", 0, &tlen);
    if (err)
        rtc->RTC_R2_10s = 0;
    else {
        err = nvs_get_blob(my_handle, "RTC_R2_10s", tmp, &tlen);
        rtc->RTC_R2_10s = atof(tmp);
    }
    if (err != 1024)
        err = nvs_get_blob(my_handle, "RTC_R3_10s", 0, &tlen);
    if (err)
        rtc->RTC_R3_10s = 0;
    else {
        err = nvs_get_blob(my_handle, "RTC_R3_10s", tmp, &tlen);
        rtc->RTC_R3_10s = atof(tmp);
    }
    if (err != 1024)
        err = nvs_get_blob(my_handle, "RTC_R4_10s", 0, &tlen);
    if (err)
        rtc->RTC_R4_10s = 0;
    else {
        err = nvs_get_blob(my_handle, "RTC_R4_10s", tmp, &tlen);
        rtc->RTC_R4_10s = atof(tmp);
    }
    if (err != 1024)
        err = nvs_get_blob(my_handle, "RTC_R5_10s", 0, &tlen);
    if (err)
        rtc->RTC_R5_10s = 0;
    else {
        err = nvs_get_blob(my_handle, "RTC_R5_10s", tmp, &tlen);
        rtc->RTC_R5_10s = atof(tmp);
    }
    if (err != 1024)
        nvs_close(my_handle);

    return err == 1024 ? ESP_FAIL : err;
}

int write_rtc(context_rtc_t *rtc) {
    assert(rtc);
    nvs_handle_t my_handle;
    int err = nvs_open(nvs_namespace, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
        char tmp[16];
        size_t tlen = 16;
        ftoa(rtc->RTC_calibration_speed, &(tmp[0]), tlen);
        err = nvs_set_blob(my_handle, "RTC_calibration_speed", tmp, tlen);
        err = nvs_set_str(my_handle, "RTC_Sleep_txt", rtc->RTC_Sleep_txt);
        ftoa(rtc->RTC_voltage_bat, &(tmp[0]), tlen);
        err = nvs_set_blob(my_handle, "RTC_voltage_bat", tmp, tlen);
        ftoa(rtc->RTC_calibration_bat, &(tmp[0]), tlen);
        err = nvs_set_blob(my_handle, "RTC_calibration_bat", tmp, tlen);

        err = nvs_set_u8(my_handle, "RTC_Board_Logo", rtc->RTC_Board_Logo);
        err = nvs_set_u8(my_handle, "RTC_Sail_Logo", rtc->RTC_Sail_Logo);
        err = nvs_set_u8(my_handle, "RTC_SLEEP_screen", rtc->RTC_SLEEP_screen);
        err = nvs_set_u8(my_handle, "RTC_OFF_screen", rtc->RTC_OFF_screen);

        err = nvs_set_i16(my_handle, "RTC_offset", rtc->RTC_offset);

        ftoa(rtc->RTC_distance, &(tmp[0]), tlen);
        err = nvs_set_blob(my_handle, "RTC_distance", tmp, tlen);
        ftoa(rtc->RTC_avg_10s, &(tmp[0]), tlen);
        err = nvs_set_blob(my_handle, "RTC_avg_10s", tmp, tlen);
        ftoa(rtc->RTC_max_2s, &(tmp[0]), tlen);
        err = nvs_set_blob(my_handle, "RTC_max_2s", tmp, tlen);

        err = nvs_set_u8(my_handle, "RTC_year", rtc->RTC_year);
        err = nvs_set_u8(my_handle, "RTC_month", rtc->RTC_month);
        err = nvs_set_u8(my_handle, "RTC_day", rtc->RTC_day);
        err = nvs_set_u8(my_handle, "RTC_hour", rtc->RTC_hour);

        err = nvs_set_u8(my_handle, "RTC_min", rtc->RTC_min);
        ftoa(rtc->RTC_alp, &(tmp[0]), tlen);
        err = nvs_set_blob(my_handle, "RTC_alp", tmp, tlen);
        ftoa(rtc->RTC_500m, &(tmp[0]), tlen);
        err = nvs_set_blob(my_handle, "RTC_500m", tmp, tlen);
        ftoa(rtc->RTC_1h, &(tmp[0]), tlen);
        err = nvs_set_blob(my_handle, "RTC_1h", tmp, tlen);
        ftoa(rtc->RTC_mile, &(tmp[0]), tlen);
        err = nvs_set_blob(my_handle, "RTC_mile", tmp, tlen);
        ftoa(rtc->RTC_R1_10s, &(tmp[0]), tlen);
        err = nvs_set_blob(my_handle, "RTC_R1_10s", tmp, tlen);
        ftoa(rtc->RTC_R2_10s, &(tmp[0]), tlen);
        err = nvs_set_blob(my_handle, "RTC_R2_10s", tmp, tlen);
        ftoa(rtc->RTC_R3_10s, &(tmp[0]), tlen);
        err = nvs_set_blob(my_handle, "RTC_R3_10s", tmp, tlen);
        ftoa(rtc->RTC_R4_10s, &(tmp[0]), tlen);
        err = nvs_set_blob(my_handle, "RTC_R4_10s", tmp, tlen);
        ftoa(rtc->RTC_R5_10s, &(tmp[0]), tlen);
        err = nvs_set_blob(my_handle, "RTC_R5_10s", tmp, tlen);

        err = nvs_commit(my_handle);
        nvs_close(my_handle);
    }
    return err;
}

#else

int read_rtc(context_rtc_t *rtc) {
    assert(rtc);
    //if (!rtc->rtc_initialized) {
#ifdef USE_CUSTOM_CALIBRATION_VAL
        rtc->RTC_calibration_bat = 1;//1.02  // was 1.75| bij ontwaken uit deepsleep
                                          // niet noodzakelijk config file lezen
#endif
    //}
    return 0;
}

int write_rtc(context_rtc_t *rtc) {
    return 0;
}

#endif

/* context_rtc_t *g_context_rtc_defaults(context_rtc_t *rtc) {
    assert(rtc);
    if (rtc->rtc_initialized)
        return rtc;
    g_context_rtc_init(rtc);
    read_rtc(rtc);
    rtc->rtc_initialized = 1;
    return rtc;
} */

void g_context_rtc_add_config(context_rtc_t *rtc, logger_config_t *config) {
    assert(rtc && config);
    rtc->RTC_Board_Logo = config->board_Logo;  // copy RTC memory !!
    rtc->RTC_Sail_Logo = config->sail_Logo;    // copy to RTC memory !!
#ifdef USE_CUSTOM_CALIBRATION_VAL
    rtc->RTC_calibration_bat = config->cal_bat <= 1.4 ? config->cal_bat : 1;
#endif
    rtc->RTC_calibration_speed = config->cal_speed / 1000;  // 3.6=km/h, 1.94384449 = knots, speed is now in mm/s
    rtc->RTC_SLEEP_screen = config->sleep_off_screen % 10;
    rtc->RTC_OFF_screen = config->sleep_off_screen / 10 % 10;
    strcpy(rtc->RTC_Sleep_txt, config->sleep_info);
    write_rtc(rtc);
}

void g_context_ubx_add_config(context_t *ctx, ubx_config_t *config) {
    assert(ctx);
    if(!ctx->gps.ublox_config)
        ctx->gps.ublox_config = config;
    assert(ctx->gps.ublox_config);
    ctx->gps.ublox_config->rtc_conf->output_rate = ctx->config->sample_rate;
    ctx->gps.ublox_config->rtc_conf->nav_mode = ctx->config->dynamic_model;
    ctx->gps.ublox_config->rtc_conf->msgout_sat = ctx->config->log_ubx_nav_sat;
    ctx->gps.ublox_config->rtc_conf->gnss = ctx->config->gnss;
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
    ctx->gps.time_out_gps_msg = (1000 / config->sample_rate + 75);  // max time out = 175 ms
    uint32_t screen;                     // preserve value config
    uint8_t screen_count, i, j;
    
    screen = config->stat_screens;
    ctx->stat_screen_count = screen_count = lenHelper(screen);
    if(screen == 0) {
        ctx->stat_screen[0] = 1;
    } else {
        for (i = screen_count; i>0; --i) {
            j = screen % 10;
            if(j>0){
                ESP_LOGW(TAG, "[%s], stat screen: %"PRIu8" in pos %"PRIu8, __FUNCTION__, j, i-1);
                ctx->stat_screen[i-1] = j;
                screen = screen / 10;
            }
        }
    }
    ESP_LOGW(TAG, "[%s], stat screens: count %"PRIu8", screens %"PRIu32, __FUNCTION__, ctx->stat_screen_count, config->stat_screens);

    screen = config->gpio12_screens; 
    ctx->gpio12_screen_count = screen_count = lenHelper(screen);
    if(screen == 0) {
        ctx->gpio12_screen[0] = 1;
    } else {
        for (i = screen_count; i>0; --i) {
            j = screen % 10;
            if(j>0){
                ctx->gpio12_screen[i-1] = j;
                screen = screen / 10;
            }
        }
    }
    ESP_LOGW(TAG, "[%s], io12 screens: count %"PRIu8", screens %"PRIu32, __FUNCTION__, ctx->gpio12_screen_count, config->gpio12_screens);

    ctx->config = config;
    return ctx;
}

uint16_t semVer() {
    return (uint16_t) VERSION;
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
#if defined(DEBUG)
    return (uint8_t) VERSION_BUILD;
#else
    return 255;
#endif
}

const char gps_logger_version[] = VERSION_STRING;

uint16_t semVerStr(char * str) {
    uint16_t size = sizeof(gps_logger_version);
    memcpy(str, gps_logger_version, size);
    str[size]=0;
    return (uint16_t) VERSION;
}

enum ubx_hw_e g_context_get_ubx_hw(context_t *ctx) {
    assert(ctx);
    return ctx->gps.ublox_config->rtc_conf->hw_type;
}
