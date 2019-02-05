/*
 * README
 * 1.) change name file on config.h
 * 2.) set SSID and Password for Wifi
 * 3.) set Username, Password and Path for ATO upload firmware
 * 4.) set pipes address, must be 5 letter string
 */
#define WIFI_SSID ""
#define WIFI_PASS ""

#define UPDATE_USERNAME ""
#define UPDATE_PASSWORD ""
#define UPDATE_PATH ""

/*-------CHANGE RADIO CODE FOR EACH LIGHT ------------*/
#define LIGHT_1_ON 1111
#define LIGHT_1_OFF 1111
#define LIGHT_2_ON 1111
#define LIGHT_2_OFF 1111
#define LIGHT_3_ON 1111
#define LIGHT_3_OFF 1111
#define LIGHT_4_ON 1111
#define LIGHT_4_OFF 1111
#define LIGHT_5_ON 1111
#define LIGHT_5_OFF 1111
#define LIGHT_6_ON 1111
#define LIGHT_6_OFF 1111
#define ALL_LIGHTS_BEDROOM_ON 1111
#define ALL_LIGHTS_BEDROOM_OFF 1111

/*
 * PIPES/ADDRESS FOR READING AND WRITINGON NRF2401
 */
#define READING_PIPE "00001"
#define WRITING_PIPE "00002"
