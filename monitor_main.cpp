//file:monitor_main.cpp
//author:chenshouhui
//date:2019-04-08
//version:1.00.01
//usage:monitor main app in intercom devices,power by android os


#include "app_detect.h"
#include "ethernet_detect.h"

using namespace android;

int main()
{
	sp<AppDetect> appDetect = new AppDetect() ;
	appDetect->startThread();

	sp<EthDetect> ethDetect = new EthDetect() ;
	ethDetect->startThread();

	while(1){
		sleep(10);
	}
	
    return 0;
}
