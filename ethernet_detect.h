#ifndef __ETHERNET_DETECT_H__
#define __ETHERNET_DETECT_H__
#include <utils/Thread.h>

#include "common_api.h"

namespace android {

#define ETH_INTERFACE_NAME    "eth0"

class EthDetect : public Thread
{
public:
	EthDetect();
	virtual ~EthDetect();

	
	void startThread()
	{
		run("EthDetectThread");
	}
	
	void stopThread()
	{
		requestExitAndWait();
	}
	virtual bool threadLoop();


};

}

#endif

