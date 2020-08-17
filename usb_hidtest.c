#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "usb_hidraw.h"



void usageInfo()
{
    printf("./usb_hidtest  [led]  [state]\n");
    printf("led : %d ->LED_MICMUTE , %d ->LED_RING, %d ->LED_SPK\n", USAGE_LED_MICMUTE, USAGE_LED_RING, USAGE_LED_SPK);
    printf("state: 1 on ,  0 off\n");
}

int main(int argc, char* argv[]){
    tDevInfo* hidDev = NULL;
    if(hidDev = open_hiddev()){
        if(argc == 1){
                usageInfo();
                sendLedUsage(hidDev, USAGE_LED_SPK, 1);
                sleep(3);
                sendLedUsage(hidDev, USAGE_LED_RING, 1);
                sleep(3);
                sendLedUsage(hidDev, USAGE_LED_MICMUTE, 1);
                sleep(3);
                sendLedUsage(hidDev, USAGE_LED_MICMUTE, 0);
                sleep(3);
                sendLedUsage(hidDev, USAGE_LED_RING, 0);
                sleep(3);
                sendLedUsage(hidDev, USAGE_LED_SPK, 0);
                sleep(3);
            }
            else if(argc == 3)
            sendLedUsage(hidDev, strtol(argv[1],NULL,10), strtol(argv[2],NULL,10));

    }    
    close_hiddev(hidDev);
    return 0;
}
