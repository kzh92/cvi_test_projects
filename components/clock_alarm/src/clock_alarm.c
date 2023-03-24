/*
 * Copyright (C) 2019-2020 Alibaba Group Holding Limited
 */
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <aos/kernel.h>
#include <aos/kv.h>
#include <uservice/uservice.h>
#include <uservice/eventid.h>

#include <clock_alarm.h>
#include <rtc_alarm.h>

#define TAG "clock_alarm"

#ifndef EVENT_CLOCK_ALARM
#define EVENT_CLOCK_ALARM  (EVENT_USER + 0x3001)
#endif

#ifndef EVENT_CLOCK_TIMEOUT
#define EVENT_CLOCK_TIMEOUT (EVENT_USER + 0x3002)
#endif

static clock_alarm_ctx_t g_clock_alarm_array[CLOCK_ALARM_NUM];
static uint8_t g_clock_alarm_status = 0;
static uint8_t g_clock_alarm_init = 0;
static clock_alarm_cb_t g_clock_alarm_cb;

static void clock_timer_callback(void *arg);

static void set_kv_config_time(uint8_t id, time_t abs_time, clock_alarm_period_e period)
{
    char name[32] = {0};

    sprintf(name, "%s%02d%s", "clock_alarm_", id, "_time");
    aos_kv_setint(name, (int)abs_time);

    sprintf(name, "%s%02d%s", "clock_alarm_", id, "_period");
    aos_kv_setint(name, (int)period);
}

static void get_kv_config_time(uint8_t id)
{
    char name[32] = {0};
    int period = CLOCK_ALARM_PERIOD_DISABLE;
    int enable = 0;
    int time = 0;
    uint8_t idx = id - 1;

    g_clock_alarm_array[idx].id = id;

    sprintf(name, "%s%02d%s", "clock_alarm_", id, "_time");
    aos_kv_getint(name, &time);
    g_clock_alarm_array[idx].time = (time_t)time;
    if (time == 0) {
        g_clock_alarm_array[idx].id = 0;
        g_clock_alarm_array[idx].period = CLOCK_ALARM_PERIOD_DISABLE;
        g_clock_alarm_array[idx].enable = 0;
        /* no clock alarm */
        return;
    }

    sprintf(name, "%s%02d%s", "clock_alarm_", id, "_period");
    aos_kv_getint(name, &period);
    g_clock_alarm_array[idx].period = (clock_alarm_period_e)period;

    sprintf(name, "%s%02d%s", "clock_alarm_", id, "_enable");
    aos_kv_getint(name, &enable);
    g_clock_alarm_array[idx].enable = enable;
}

static void set_kv_config_enable(uint8_t id, uint8_t enable)
{
    char name[32] = {0};

    sprintf(name, "%s%02d%s", "clock_alarm_", id, "_enable");
    aos_kv_setint(name, (int)enable);
}

static void get_kv_config_enable(uint8_t id)
{
    char name[32] = {0};
    int enable = 0;

    sprintf(name, "%s%02d%s", "clock_alarm_", id, "_enable");
    aos_kv_getint(name, &enable);
    g_clock_alarm_array[id - 1].enable = enable;
}


static struct tm *convert_to_tm(time_t abs_time)
{
    struct tm *tm_now;
    tm_now = gmtime(&abs_time);
    return tm_now;
}

static void update_period(clock_alarm_period_e period, time_t *time)
{
    if (period == CLOCK_ALARM_ONCE) {
        *time = 0;
    } else if (period == CLOCK_ALARM_PERIOD_DAY) {
        *time += ONE_DAY_TIME;
    } else if (period == CLOCK_ALARM_PERIOD_WEEK) {
        *time += ONE_WEEK_TIME;
    } else {
        struct tm *tmp_tm = convert_to_tm(*time);
        if (tmp_tm->tm_wday == 5) {
            //1 day and add week 6 and 7 (72hour)
            *time += ONE_DAY_TIME * 3;
        } else if (tmp_tm->tm_wday == 6) {
            //add week 6 and 7 (48hour)
            *time += ONE_DAY_TIME * 2;
        } else {
            *time += ONE_DAY_TIME;
        }
    }
}

static time_t convert_to_absolute_time(clock_alarm_config_t *cfg_time)
{
    struct tm *tm_now;
    time_t absolute_time = time(NULL);

    if (cfg_time == NULL) {
        return 0;
    }
    if (cfg_time->hour >= 24 || cfg_time->hour < 0) {
        return 0;
    }
    if (cfg_time->min >= 60 || cfg_time->min < 0) {
        return 0;
    }
    if (cfg_time->sec >= 60 || cfg_time->sec < 0) {
        return 0;
    }
    tm_now = gmtime(&absolute_time);
    if (cfg_time->period == CLOCK_ALARM_PERIOD_WORKDAY) {
        if (tm_now->tm_wday == 6) {
            //week 6 set to Monday
            absolute_time += ONE_DAY_TIME * 2;
        } else if (tm_now->tm_wday == 0) {
            //week 7 set to Monday
            absolute_time += ONE_DAY_TIME;
        }
    }

    tm_now = gmtime(&absolute_time);
    // tm_now->tm_wday = cfg_time.week;
    tm_now->tm_hour = cfg_time->hour;
    tm_now->tm_min = cfg_time->min;
    tm_now->tm_sec = cfg_time->sec;
    absolute_time = mktime(tm_now);

    if (absolute_time <= rtc_get_time()) {
        if (cfg_time->period == CLOCK_ALARM_ONCE) {
                absolute_time += ONE_DAY_TIME;
        } else {
            update_period(cfg_time->period, &absolute_time);
        }
    }

    return absolute_time;
}


static void set_clock_alarm_status(uint8_t status)
{
    g_clock_alarm_status = status;
}

uint8_t clock_alarm_get_status(void)
{
#ifdef CONFIG_CLOCK_ALARM_SCHEDULE
    return 0;
#else
    return g_clock_alarm_status;
#endif
}


static uint8_t get_mini_clock_id(void)
{
    uint8_t idx = 0;
    time_t clock_time = 0;
    uint8_t min_id = 0;

    /* get mini enable clock before update */
    for (idx = 0; idx < CLOCK_ALARM_NUM; idx++) {
        if (g_clock_alarm_array[idx].enable) {
            if (clock_time == 0 || clock_time > g_clock_alarm_array[idx].time) {
                clock_time = g_clock_alarm_array[idx].time;
                //set initial value or min value
                min_id = g_clock_alarm_array[idx].id;
            }
        }
    }
    return min_id;
}

static void update_clock_alarm(uint8_t idx)
{
    /* error time info */
    if (g_clock_alarm_array[idx].period == CLOCK_ALARM_PERIOD_DISABLE) {
        set_kv_config_time(idx + 1, 0, CLOCK_ALARM_PERIOD_DISABLE);
        set_kv_config_enable(idx + 1, 0);
        g_clock_alarm_array[idx].id = 0;
        g_clock_alarm_array[idx].time = 0;
        g_clock_alarm_array[idx].period = CLOCK_ALARM_PERIOD_DISABLE;
        g_clock_alarm_array[idx].enable = 0;
        return;
    }

    // LOGI(TAG, "update_clock_alarm id %d: (period = %d)", id, g_clock_alarm_array[idx].period);
    if (g_clock_alarm_array[idx].period == CLOCK_ALARM_ONCE) {
        //delete kv and reset struct
        set_kv_config_time(g_clock_alarm_array[idx].id, 0, CLOCK_ALARM_PERIOD_DISABLE);
        set_kv_config_enable(g_clock_alarm_array[idx].id, 0);
        g_clock_alarm_array[idx].id = 0;
        g_clock_alarm_array[idx].time = 0;
        g_clock_alarm_array[idx].period = CLOCK_ALARM_PERIOD_DISABLE;
        g_clock_alarm_array[idx].enable = 0;
    } else {
        //update clock time
        update_period(g_clock_alarm_array[idx].period, &g_clock_alarm_array[idx].time);

        set_clock_alarm_status(1);
        // store to kv
        set_kv_config_time(g_clock_alarm_array[idx].id, g_clock_alarm_array[idx].time, g_clock_alarm_array[idx].period);
    }
}


static void config_rtc_alarm(time_t set_time, clock_alarm_period_e period)
{
#ifdef CONFIG_CLOCK_ALARM_SCHEDULE
    struct tm *tm_set = convert_to_tm(set_time);
#else
    //ahead 30s
    struct tm *tm_set = convert_to_tm(set_time - 30);
#endif
    // LOGD(TAG, "config_rtc_alarm: time %d, period %d", set_time, period);
    if (period != CLOCK_ALARM_PERIOD_WEEK) {
        tm_set->tm_wday = -1;
    }
    rtc_set_alarm(tm_set);
}

static void update_clock_timer_info(time_t time_rtc_now)
{
#ifndef CONFIG_CLOCK_ALARM_SCHEDULE
    if (clock_alarm_get_status() == 2) {
        return;
    }
#endif

    uint8_t min_id = get_mini_clock_id();
    if (min_id == 0) {
        /* not found */
        return;
    }
    uint8_t idx = min_id - 1;

#ifdef CONFIG_CLOCK_ALARM_SCHEDULE
    config_rtc_alarm(g_clock_alarm_array[idx].time, g_clock_alarm_array[idx].period);
#else
    int delta = g_clock_alarm_array[idx].time - time_rtc_now;
    if (delta < 90) {
        long arg_mini_id = min_id;
        void *arg = (void *)arg_mini_id;
        //save status in order to not enter lpm vad
        set_clock_alarm_status(2);

        if (delta == 0) {
            event_publish(EVENT_CLOCK_TIMEOUT, arg);
        } else {
            event_publish_delay(EVENT_CLOCK_TIMEOUT, arg, delta * 1000);
        }
    } else {
        config_rtc_alarm(g_clock_alarm_array[idx].time, g_clock_alarm_array[idx].period);
    }
#endif
}

static void clock_timer_callback(void *arg)
{
    time_t time_rtc_now = rtc_get_time();
    uint8_t clock_id = (uint8_t)((long)arg);

    if (abs(time_rtc_now - g_clock_alarm_array[clock_id - 1].time) < 3) {
        LOGD(TAG,"clock alarm happen id: %d", clock_id);
        if (g_clock_alarm_cb) {
            g_clock_alarm_cb(clock_id);
        }

        // update_clock_alarm ���� callback ֮�󣬽�� callback �� clock_alarm_get �ò��� clock_id ����
        update_clock_alarm(clock_id - 1);
    }

    for (int i = 0; i < CLOCK_ALARM_NUM; i++) {
        if(g_clock_alarm_array[i].time < time_rtc_now) {
            update_clock_alarm(i);
        }
    }

#ifndef CONFIG_CLOCK_ALARM_SCHEDULE
    set_clock_alarm_status(0);
    //restore status
    for (int idx = 0; idx < CLOCK_ALARM_NUM; idx++) {
        if (g_clock_alarm_array[idx].time) {
            set_clock_alarm_status(1);
            break;
        }
    }
#endif
    update_clock_timer_info(time_rtc_now);
}

static void clock_alarm_handle(void)
{
    uint8_t idx = 0;
    time_t alarm_time = 0;
    uint8_t min_id = 0;
    time_t time_rtc_now = rtc_get_time();

    /* get mini enable clock before update */
    min_id = get_mini_clock_id();
    if (min_id == 0) {
        /* not found */
        return;
    }

    idx = min_id - 1;
    int32_t delta = g_clock_alarm_array[idx].time - time_rtc_now;
#ifdef CONFIG_CLOCK_ALARM_SCHEDULE
    if (delta < 0) {
        /* update all clocks which time < rtc_now */
        for (idx = 0; idx < CLOCK_ALARM_NUM; idx++) {
            alarm_time = g_clock_alarm_array[idx].time;
            if (alarm_time != 0 && alarm_time < time_rtc_now) {
                update_clock_alarm(idx);
            }
        }

        /* get mini enable clock after update */
        min_id = get_mini_clock_id();
        if (min_id == 0) {
            /* not found */
            return;
        }
        idx = min_id - 1;
    }

    delta = g_clock_alarm_array[idx].time - time_rtc_now;
    if (delta == 0) {
        int arg_mini_id = min_id;
        void *arg = (void *)arg_mini_id;
        event_publish(EVENT_CLOCK_TIMEOUT, arg);
    } else {
        config_rtc_alarm(g_clock_alarm_array[idx].time, g_clock_alarm_array[idx].period);
    }
#else
    //delta < 90 sec, start soft time. update clock array
    if (delta < 90)
    {
        /* update all clocks which time < rtc_now */
        for (idx = 0; idx < CLOCK_ALARM_NUM; idx++) {
            alarm_time = g_clock_alarm_array[idx].time;
            if (alarm_time != 0 && alarm_time < time_rtc_now) {
                update_clock_alarm(idx);
            }
        }

        update_clock_timer_info(time_rtc_now);
    } else {
        config_rtc_alarm(g_clock_alarm_array[idx].time, g_clock_alarm_array[idx].period);
    }
#endif
}


static void local_event_cb(uint32_t event_id, const void *param, void *context)
{
    if (event_id == EVENT_CLOCK_ALARM) {
        clock_alarm_handle();
    } else if (event_id == EVENT_CLOCK_TIMEOUT) {
        LOGD(TAG, "EVENT_CLOCK_TIMEOUT");
        clock_timer_callback((void *)param);
    } else {
        LOGE(TAG,"local_event_cb error event_id = 0x%x", event_id);
    }
}

int clock_alarm_init(clock_alarm_cb_t alarm_cb)
{
    if(g_clock_alarm_init != 0) {
        LOGD(TAG, "clock alarm has already inited!");
        return 0;
    }

    if (alarm_cb == NULL) {
        LOGW(TAG, "callback fn not set!");
    }

    rtc_init();

    g_clock_alarm_cb = alarm_cb;

    memset(g_clock_alarm_array, 0x00, sizeof(clock_alarm_ctx_t) * CLOCK_ALARM_NUM);
    //loop clock kv
    for (uint8_t idx = 0; idx < CLOCK_ALARM_NUM; idx++) {
        get_kv_config_time(idx + 1);
        if(g_clock_alarm_array[idx].time == 0) {
            //not set clock alarm
            continue;
        }

        get_kv_config_enable(idx + 1);
    }

    event_subscribe(EVENT_CLOCK_ALARM, local_event_cb, NULL);
    event_subscribe(EVENT_CLOCK_TIMEOUT, local_event_cb, NULL);
    event_publish(EVENT_CLOCK_ALARM, NULL);
    g_clock_alarm_init = 1;
    return 0;
}


// set clock time, output: -2 - repeat clock; -1 - full array; other - clock id
//id: 0 - set new clock; other - modify id clock info
int clock_alarm_set(uint8_t id, clock_alarm_config_t *cfg_time)
{
    uint8_t idx = 0;
    time_t abs_time = convert_to_absolute_time(cfg_time);

    if (id == 0) {
        if (abs_time == 0) {
            /* config time error */
            return -4;
        }
        //new clock
        for (idx = 0; idx < CLOCK_ALARM_NUM; idx++) {
            if (abs_time == g_clock_alarm_array[idx].time && 
                cfg_time->period == g_clock_alarm_array[idx].period) {
                //clock repeat
                return -2;
            }
        }
        //find empty
        for (idx = 0; idx < CLOCK_ALARM_NUM; idx++) {
            if (g_clock_alarm_array[idx].time == 0) {
                break;
            }
        }

        if (idx >= CLOCK_ALARM_NUM) {
            // clock alarm full
            return -1;
        }
        g_clock_alarm_array[idx].enable = 0;
    } else {
        for (idx = 0; idx < CLOCK_ALARM_NUM; idx++) {
            if (abs_time != 0 && abs_time == g_clock_alarm_array[idx].time &&
                cfg_time->period == g_clock_alarm_array[idx].period) {
                //clock repeat
                return -2;
            }
        }
        //modify clock
        idx = id - 1;
        if (idx >= CLOCK_ALARM_NUM) {
            return -3;
        }

        if (cfg_time == NULL) {
            uint8_t mini_id = get_mini_clock_id();

            set_kv_config_time(id, 0, CLOCK_ALARM_PERIOD_DISABLE);
            set_kv_config_enable(id, 0);
            g_clock_alarm_array[idx].id = 0;
            g_clock_alarm_array[idx].time = 0;
            g_clock_alarm_array[idx].period = CLOCK_ALARM_PERIOD_DISABLE;
            g_clock_alarm_array[idx].enable = 0;

            if (mini_id == id) {
#ifndef CONFIG_CLOCK_ALARM_SCHEDULE
                set_clock_alarm_status(0);
                //restore status
                for (int idx = 0; idx < CLOCK_ALARM_NUM; idx++) {
                    if (g_clock_alarm_array[idx].time) {
                        set_clock_alarm_status(1);
                        break;
                    }
                }
#endif
                update_clock_timer_info(rtc_get_time());
            }

            return 0;
        }
        if (abs_time == 0) {
            /* config time error */
            return -4;
        }
    }

    //set value
    g_clock_alarm_array[idx].id = idx + 1;
    g_clock_alarm_array[idx].time = abs_time;
    g_clock_alarm_array[idx].period = cfg_time->period;
    if (clock_alarm_get_status() == 0) {
        set_clock_alarm_status(1);
    }

    //store to kv
    set_kv_config_time(g_clock_alarm_array[idx].id, g_clock_alarm_array[idx].time, g_clock_alarm_array[idx].period);
    return g_clock_alarm_array[idx].id;
}


// set clock time, output: -2 - repeat clock; -1 - full array; other - clock id
//id: 0 - set new clock; other - modify id clock info
int clock_alarm_abstime_set(uint8_t id, time_t abs_time, clock_alarm_period_e period)
{
    uint8_t idx = 0;

    if (abs_time == 0) {
        /* config time error */
        return -4;
    }

    if (id == 0) {
        //new clock
        for (idx = 0; idx < CLOCK_ALARM_NUM; idx++) {
            if (abs_time == g_clock_alarm_array[idx].time) {
                //clock repeat
                return -2;
            }
        }
        //find empty
        for (idx = 0; idx < CLOCK_ALARM_NUM; idx++) {
            if (g_clock_alarm_array[idx].time == 0) {
                break;
            }
        }

        if (idx >= CLOCK_ALARM_NUM) {
            // clock alarm full
            return -1;
        }
        g_clock_alarm_array[idx].enable = 0;
    } else {
        //modify clock
        idx = id - 1;
        if (idx >= CLOCK_ALARM_NUM) {
            return -3;
        }
    }

    //set value
    g_clock_alarm_array[idx].id = idx + 1;
    g_clock_alarm_array[idx].time = abs_time;
    g_clock_alarm_array[idx].period = period;
    if (clock_alarm_get_status() == 0) {
        set_clock_alarm_status(1);
    }

    //store to kv
    set_kv_config_time(g_clock_alarm_array[idx].id, g_clock_alarm_array[idx].time, g_clock_alarm_array[idx].period);
    return g_clock_alarm_array[idx].id;
}

// get the clock alarm context
clock_alarm_ctx_t *clock_alarm_get(uint8_t id)
{
    for (uint8_t idx = 0; idx < CLOCK_ALARM_NUM; idx++) {
        if (id == g_clock_alarm_array[idx].id) {
            return &g_clock_alarm_array[idx];
        }
    }

    return NULL;
}

/* set the clock enabel or not. update rtc alarm */
void clock_alarm_enable(uint8_t id, uint8_t enable)
{
    uint8_t idx = 0;
    time_t alarm_time = 0;
    time_t abs_time_now = rtc_get_time();

    //get the time with id
    for (idx = 0; idx < CLOCK_ALARM_NUM; idx++) {
        if(g_clock_alarm_array[idx].id == id){
            break;
        }
    }
    if (idx == CLOCK_ALARM_NUM) {
        LOGE(TAG, "clock not found");
        return;
    }
    //store enable to kv and struct
    g_clock_alarm_array[idx].enable = enable;
    set_kv_config_enable(id, enable);

    /* update all clocks which time < rtc_now */
    for (idx = 0; idx < CLOCK_ALARM_NUM; idx++) {
        alarm_time = g_clock_alarm_array[idx].time;
        if (alarm_time != 0 && alarm_time <= abs_time_now) {
            update_clock_alarm(idx);
        }
    }

    update_clock_timer_info(abs_time_now);
}
