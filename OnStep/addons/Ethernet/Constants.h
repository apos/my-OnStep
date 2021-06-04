// -----------------------------------------------------------------------------------
// Constants

// Configuration options

// On/Off, etc.
#define OFF                 -1
#define ON                  -2
#define METRIC              ON
#define IMPERIAL            -3
#define LOCALE_DEFAULT      -4
#define AUTO                -5
#define AUTO_ON             -6
#define AUTO_OFF            -7
#define FWU                 -8
#define VERBOSE             -9
#define REMOTE             -10
#define INVALID        -999999

// encoder types
#define AB 1
#define CWCCW 2
#define BC_BISSC 3

#define DEFAULT_AJAX_RATE "5"        // normally 5 seconds between updates
#define DEFAULT_FAST_AJAX_RATE "1"   // fast update is 1 second/update
#define DEFAULT_AJAX_SHED_TIME "15"  // time before return to normal update rate

// various auxillary features
#define SWITCH 1
#define ANALOG_OUTPUT 2
#define DEW_HEATER 3
#define INTERVALOMETER 4

// set MCU string
#if defined(ESP8266)
  #define MCU_STR "ESP8266"
#elif defined(ESP32)
  #define MCU_STR "ESP32"
#elif defined(_mk20dx128_h_)
  #define MCU_STR "Teensy3.0"
#elif defined(__MK20DX128__)
  #define MCU_STR "Teensy3.1"
#elif defined(__MK20DX256__)
  #define MCU_STR "Teensy3.2"
#elif defined(__IMXRT1052__) || defined(__IMXRT1062__)
  #define MCU_STR "Teensy4.0"
  #define Teensy40
#else
  #define MCU_STR "Unknown"
#endif

// work around for some platform specific code
#if !defined(ESP8266) && !defined(ESP32)
  #define ICACHE_RAM_ATTR
  #define FPSTR
#endif

// special empty string
#ifdef Teensy40
  const char* EmptyStr = "\1";
#else
  const char* EmptyStr = "";
#endif

// Default Serial1, this is the hardware serial port where OnStep is attached
#define Ser      Serial1

// EEPROM contents
#define EE_KEY_HIGH           0   // 2
#define EE_KEY_LOW            2   // 2

#define EE_AP_EN              4   // 2
#define EE_STA_EN             6   // 2
#define EE_DHCP_EN            8   // 2

#define EE_TIMEOUT_WEB       10   // 2
#define EE_TIMEOUT_CMD       12   // 2

#define EE_STA_IP            20   // 4
#define EE_STA_GW            24   // 4
#define EE_STA_SN            28   // 4

#define EE_AP_CH             50   // 2
#define EE_AP_IP             60   // 4
#define EE_AP_GW             70   // 4
#define EE_AP_SN             80   // 4

#define EE_STA_SSID         100   // 40
#define EE_STA_PWD          150   // 40

#define EE_PASSWORD         200   // 40

#define EE_AP_SSID          500   // 40
#define EE_AP_PWD           550   // 40

#define EE_ENC_A1_DIFF_TO   600   // 4
#define EE_ENC_A2_DIFF_TO   604   // 4
#define EE_ENC_RC_STA       608   // 4
#define EE_ENC_RC_LTA       612   // 4
#define EE_ENC_RC_RCOMP     616   // 4
#define EE_ENC_RC_INTP_P    624   // 4
#define EE_ENC_RC_INTP_M    628   // 4
#define EE_ENC_RC_PROP      632   // 4
#define EE_ENC_MIN_GUIDE    636   // 4
#define EE_ENC_A1_ZERO      650   // 4
#define EE_ENC_A2_ZERO      654   // 4
#define EE_ENC_A1_TICKS     658   // 8
#define EE_ENC_A2_TICKS     666   // 8
#define EE_ENC_A1_REV       674   // 2
#define EE_ENC_A2_REV       676   // 2
#define EE_ENC_AUTO_SYNC    678   // 2
