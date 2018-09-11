/******************
 
 neohub.h
 
 ******************/
#ifndef _NEOHUB_H
#define _NEOHUB_H

#ifndef DEBUG
#define DEBUG 0 // dont change this - pass it via -DDEBUG=1 at compile
#endif

#define DUMMY_JSON "/Users/ecole/src/neoprobe/neoprobe/neohub.json" // used in debugging

#include <iostream>
//#include </usr/local/include/json/json.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define MYNAME "neoprobe"
#define VERSION "2.0.0"

#define D_SERVER_NAME "neohub"
#define D_PORT 4242
#define SOC_BUFFER_SZ 4096
#define NO_OF_DEVICES 64

struct neohub {
    bool    away;
    bool    cooling;
    bool    cooling_enabled;
    int    cooling_temperature_in_whole_degrees;
    bool    cool_inp;
    struct  {
        int hours;
        int mins;
    }count_down_time;
    bool    cradle_paired_to_remote_sensor;
    bool    cradle_paired_to_stat;
    int    current_floor_temperature;
    float    current_set_temperature;
    float    current_temperature;
    bool    demand;
    int    device_type;
    bool    enable_boiler;
    bool    enable_cooling;
    bool    enable_pump;
    bool    enable_valve;
    bool    enable_zone;
    bool    failsafe_state;
    bool    fail_safe_enabled;
    bool    floor_limit;
    bool    full_partial_lock_available;    // full/partial_lock_available
    bool    heat_cool_mode;            // heat/cool_mode
    bool    heating;
    int    hold_temperature;
    struct    {
        int hours;
        int mins;
    } hold_time;
    bool    holiday;
    int    holiday_days;
    int    humidity;
    bool    lock;
    char    *lock_pin_number;
    bool    low_battery;
    float    max_temperature;
    float    min_temperature;
    int    modulation_level;
    char    *next_on_time;
    bool    offline;
    bool    ouput_delay;
    int    output_delay;
    bool    preheat;
    struct  {
        int hours;
        int mins;
    }preheat_time;
    char    *program_mode;
    bool    pump_delay;
    bool    radiators_or_underfloor;
    char    *sensor_selection;
    int    set_countdown_time;
    bool    standby;
    struct    {
        bool    four_heat_levels;            // 4_heat_levels
        bool    manual_off;
        bool    thermostat;
        bool    timeclock;
    }stat_mode;
    bool    temperature_format;
    bool    temp_hold;
    bool    timeclock_mode;
    bool    timer;
    bool    time_clock_overide_bit;
    int    ultra_version;
    int    version_number;
    int    write_count;
    bool    zone_1paired_to_multilink;
    bool    zone_1_or_2;
    bool    zone_2_paired_to_multilink;
    std::string  device;
    struct  {
        enum day {monday,tuesday,wednesday,thursday,friday,saturday,sunday};
        uint8_t temp;
        uint8_t hour;
        uint8_t min;
    }comfort[4*7];             // 4 comfort levels per day
};

const std::string keys[] = {        // string values
    "device",
    "CURRENT_FLOOR_TEMPERATURE",
    "CURRENT_SET_TEMPERATURE",
    "CURRENT_TEMPERATURE"
};





void error(const char *);
char *getNeohub(char *,char *,int);
char *getNeohubFile(char *,char *,int);
bool readJson(char *, char *);

#endif

