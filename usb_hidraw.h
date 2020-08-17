#ifndef _USB_HIDRAW_H_
#define _USB_HIDRAW_H_


#ifdef __cplusplus 
extern "C" { 
#endif 

enum {
  HID_PLT_BLACKWIRE_5220,
  HID_JABRA_2400,
  HID_MAX_NUM
};

typedef enum{
  USAGE_LED_MICMUTE = 0,
  USAGE_LED_RING,
  USAGE_LED_SPK,
  USAGE_MAX_NUM
}eLed;

typedef struct{
    unsigned int vid;
    unsigned int pid;
}tDevId;

typedef struct{
    int fd;
    int model;
}tDevInfo;

typedef struct{
    int reportIndex;
    int fieldIndex;
    int usageIndex;
}tHidUsageInfo;

tDevInfo* open_hiddev();
int sendLedUsage(tDevInfo* devInfo, eLed led , int state);
void close_hiddev(tDevInfo* dev_info);


#ifdef __cplusplus 
} 
#endif 

#endif
