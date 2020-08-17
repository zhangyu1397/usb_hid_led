#define LOG_TAG "usb_hid"
//#include <utils/Log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/hid.h>
#include <linux/hidraw.h>
#include <linux/hiddev.h>
#include "usb_hidraw.h"

#define USE_HIDDEV
#define ALOGE printf

#ifdef USE_HIDDEV
#define HID_DEV_PATH "/dev/hiddev0"
#else
#define HID_DEV_PATH "/dev/hidraw0"
#endif

#ifndef makedev
#define makedev(maj, min) (((maj) << 8) + (min))
#endif





static tHidUsageInfo devLedUsageList[HID_MAX_NUM][USAGE_MAX_NUM] = {
    [HID_PLT_BLACKWIRE_5220]   =   {
                                            {.reportIndex =9, .fieldIndex = 0, .usageIndex = 0}, //mute led
                                            {.reportIndex =0x17, .fieldIndex = 0, .usageIndex = 0}, //ring led
                                            {.reportIndex =0x18, .fieldIndex = 0, .usageIndex = 0}//call led
                                        }, 
    [HID_JABRA_2400]  =  {
                                            {.reportIndex =2, .fieldIndex = 0, .usageIndex = 1}, //mute led
                                            {.reportIndex =2, .fieldIndex = 0, .usageIndex = 2}, //ring led
                                            {.reportIndex =2, .fieldIndex = 0, .usageIndex = 0}//call led
                                        },
};

static tDevId devlist[HID_MAX_NUM] = {

    [HID_PLT_BLACKWIRE_5220] = {.vid = 0x047f, .pid = 0xc053},
    [HID_JABRA_2400] = {.vid = 0x0b0e, .pid = 0x2324},

};

static int match_dev(unsigned int vid, unsigned int pid){
    int ret = -1, i = 0;
    tDevId * dev;
    for(i = 0; i < HID_MAX_NUM; i++){
        dev = &(devlist[i]);
        if(dev->vid == (vid & 0xffff) && dev->pid == (pid & 0xffff)){
             ret = i;
             break;
        }
    }
    return ret;
}


static int getUsageInfo(tDevInfo* devInfo, tHidUsageInfo* usageInfo, eLed led)
{
    if(NULL == devInfo || devInfo->model >= HID_MAX_NUM || devInfo->model < 0 || led >= USAGE_MAX_NUM || led < 0)
        return -1;

    *usageInfo = devLedUsageList[devInfo->model][led];
    return 0;
}

tDevInfo* open_hiddev(){
    int ret = -1;
    unsigned int vid = 0,pid = 0;
    tDevInfo* dev_info;
    struct hiddev_devinfo dinfo;
    if(access(HID_DEV_PATH, F_OK) !=  0){
		if(access("/dev/usb/", F_OK) !=  0){
			mkdir("dev/usb",666);
		}
        mknod(HID_DEV_PATH, S_IFCHR|0666, makedev(180,96));
    }

    dev_info = malloc(sizeof(tDevInfo));
    if(dev_info)
        memset(dev_info, 0 ,sizeof(tDevInfo));
    else
        return NULL;
    
    if(dev_info->fd > 0){
        ALOGE(" %s already opened!\n", HID_DEV_PATH);
        goto fail;
    }


    dev_info->fd = open(HID_DEV_PATH, O_RDWR);
    if(dev_info->fd <= 0){
        ALOGE("open_hiddev (%s) fail!\n", HID_DEV_PATH);
        goto fail;
    }

    ret = ioctl(dev_info->fd, HIDIOCGDEVINFO, &dinfo);
    vid = dinfo.vendor;
    pid = dinfo.product;
    
    if(ret != 0){
       ALOGE("can't get dev (%s) info.\n",HID_DEV_PATH);
       goto fail;
    }

    if((dev_info->model = match_dev(vid, pid)) < 0){
       ALOGE("dev(0x%x,0x%x) not match,error!\n",vid,pid);
       goto fail;
    }
    return dev_info;
fail:
    if(dev_info->fd > 0) 
        close_hiddev(dev_info);
    return NULL;
}

int sendLedUsage(tDevInfo* devInfo, eLed led , int state){
    //unsigned char buf[2] = {0x00,0x00};
    int report_id = 0;
    tHidUsageInfo usageInfo;

    memset(&usageInfo, 0, sizeof(usageInfo));
    
    if(devInfo->fd <= 0 || led >= USAGE_MAX_NUM || led < 0) 
    {
        ALOGE("%s  info error\n",__func__);
        return -1;
    }

    //if(state) buf[1] = 0x01;
    if(getUsageInfo(devInfo, &usageInfo, led) >= 0){
	
		struct hiddev_usage_ref uref;
		struct hiddev_report_info rinfo;
		memset(&uref, 0 ,sizeof(struct hiddev_usage_ref));
		memset(&rinfo, 0 ,sizeof(struct hiddev_report_info));
		
		uref.report_type  = HID_REPORT_TYPE_OUTPUT;
        uref.field_index   = usageInfo.fieldIndex;
        uref.report_id      = usageInfo.reportIndex;
        uref.usage_index = usageInfo.usageIndex;

		uref.value = !!state;
		if(ioctl(devInfo->fd,HIDIOCSUSAGE, &uref) < 0)
		{
			ALOGE("ioctl-setout HIDIOCSUSAGE error!\n");
			return -1;
		}
		
		rinfo.report_type = HID_REPORT_TYPE_OUTPUT;
        rinfo.report_id     = usageInfo.reportIndex;
		if(ioctl(devInfo->fd,HIDIOCSREPORT,&rinfo) < 0)
		{
			ALOGE("ioctl-setout HIDIOCSREPORT error!\n");
			return -1;
		}
    }
    else
    {
        printf("failed\n");
    }
    
    return 0;
}

void close_hiddev(tDevInfo* dev_info)
{
    if(NULL == dev_info)
        return;
    
    if(dev_info->fd)
        close(dev_info->fd);

    free(dev_info);
}

