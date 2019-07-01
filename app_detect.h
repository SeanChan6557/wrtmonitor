//add by chenshouhui,20181012
#ifndef __APP_DETECT_H__
#define __APP_DETECT_H__


#include "common_api.h"


namespace android {



class AppDetect:public Thread
{
public:
	AppDetect();
	virtual ~AppDetect();

	
	void startThread()
	{
		run("AppDetectThread");
	}
	
	void stopThread()
	{
		requestExitAndWait();
	}
	virtual bool threadLoop();

private:
	int mProjectId;
	S_PROJECT_INFO mExsitProject[2];

	void calcProjectId(){
		mExsitProject[0].mainAppPackageName = "com.wrtsz.intercom.master";
		mExsitProject[0].mainAppPackagePath = "/data/data/com.wrtsz.intercom.master";
		mExsitProject[0].mainAppInstalledPath = "/system/WrtszIntercomMaster/WrtszIntercomMasterSigned.apk";
		mExsitProject[0].mainActivityStartCmd = "am start -n com.wrtsz.intercom.master/.view.main.MainActivity";
		mExsitProject[0].mainAppInstallCmd = "pm install -r /system/WrtszIntercomMaster/WrtszIntercomMasterSigned.apk";

		mExsitProject[1].mainAppPackageName = "com.wrtsz.indoordevice";
		mExsitProject[1].mainAppPackagePath = "/data/data/com.wrtsz.indoordevice";
		mExsitProject[1].mainAppInstalledPath = "/system/WrtszIntecomSlave/WrtszIndoorDeviceSigned.apk";
		mExsitProject[1].mainActivityStartCmd = "am start -n com.wrtsz.indoordevice/.view.home.MainActivity";
		mExsitProject[1].mainAppInstallCmd = "pm install -r /system/WrtszIntecomSlave/WrtszIndoorDeviceSigned.apk";

		int projectCount = (int)sizeof(mExsitProject) /sizeof(S_PROJECT_INFO);
		for(int i=0 ; i< projectCount; i++){
			if(0 == access(mExsitProject[i].mainAppInstalledPath,F_OK)){
				mProjectId =  i;
				break;
			}
		}
		
	}
	void firstBootCheckMainAppAndInstall(int projectId);
	int getMainAppPid(int projectId);
	bool getMainAppCodePath(int projectId,char*appCodePath,int appCodePathLength);
	bool isAppIsInstalled(int projectId);
	void installAppForException(int projectId);	
	int startMainAppRunning(int projectId,int curPid);	
	void exceptionPropUpdate(E_EXCEPTION_OP_TYPE opType);	
	void exceptionDeal();	
	int mainAppMonitorLooper(int projectId);
};

};
#endif
