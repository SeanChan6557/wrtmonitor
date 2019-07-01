#include "ethernet_detect.h"
#include <netutils/ifc.h>

namespace android {

	EthDetect::EthDetect()
	{

	}

	EthDetect::~EthDetect() 
	{
	}


	bool EthDetect::threadLoop()
	{
	    bool r = true;
		while(1){
			E_IFF_STATE netIfState = getNetIfState(ETH_INTERFACE_NAME);
			
			if(IFF_NET_UP != netIfState){
				MY_LOGE("eth0 has down,should up it !!");
				ifc_enable(ETH_INTERFACE_NAME);
			}
			sleep(2);
		}
	    return r;
	}
}
