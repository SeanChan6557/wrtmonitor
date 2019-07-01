//file:monitor_service.cpp
//author:chenshouhui
//date:2018-12-03
//version:1.00.00
//usage:monitor main app in intercom devices,power by android os

#include "app_detect.h"

namespace android {

AppDetect::AppDetect():mProjectId(-1)
{
	calcProjectId();
	
	MY_LOGE("mProjectId === %d \n",mProjectId);
}

AppDetect::~AppDetect() 
{
	MY_LOGE("[%s-%d]\n",__FILE__,__LINE__);
}
void AppDetect::firstBootCheckMainAppAndInstall(int projectId)
{
	int systemUiCheckTimes = 0;
	int mainAppInstalledCheckTimes = 0;
	
	mySystem(SYSTEM_CMD_DEFAULT_RETRY_TIMES*2,SYSTEM_CMD_DEFAULT_SLEEP_TIMES,IP_RULE);

    MY_LOGE("[%s-%d]%s start", __FILE__, __LINE__,__FUNCTION__);
	while(true){
		int systemUiPid = getPidByCmdline(SYSTEM_UI_PROCESS_NAME);
		if(checkAppIsRunning(systemUiPid))
			break;
		MY_LOGE("System ui has not running,should retry !");
		sleep(LOOPER_SLEEP_TINE);
		systemUiCheckTimes++;
		if(systemUiCheckTimes >= SYSTEMUI_CHECK_COUNTS){
			exceptionDeal();
		}
   }

	while(!isAppIsInstalled(projectId) ){
		MY_LOGE("The main app is not installed,should install it");
		installAppForException(projectId);
		mainAppInstalledCheckTimes++;

		if(mainAppInstalledCheckTimes >= MAX_EXCEPTION_TIMES){
			exceptionDeal();
		}

	}
}



bool AppDetect::threadLoop()
{
    bool r = true;
    sleep(FIRST_SLEEP_TIME);
	if(mProjectId > -1){
		firstBootCheckMainAppAndInstall(mProjectId);
		mainAppMonitorLooper(mProjectId);
	}
    return r;
}


int AppDetect::getMainAppPid(int projectId)
{
	int mainPid = 0;
	mainPid = getPidByCmdline((char *)mExsitProject[projectId].mainAppPackageName);
	MY_LOGI("mainPid === %d\n",mainPid);
	return mainPid;
}


bool AppDetect::getMainAppCodePath(int projectId,char*appCodePath,int appCodePathLength)
{
	if(NULL == appCodePath){
		return false;
	}

	memset(appCodePath,0,appCodePathLength);

	int fd = open(PACKAGE_XML_PATH, O_RDONLY);
	if(fd < 0)
		return false;

	int filelen= lseek(fd,0L,SEEK_END);
	if(filelen < 1){
		close(fd);
		return false;
	}

	char *tempBuff = (char *)malloc(filelen+1);
	char *initTempBuff = tempBuff;
	char KeyCode[CMDLINE_LENGT] = {0};
	if(NULL == tempBuff){
		close(fd);
		return false;
	}

	memset(tempBuff,0,filelen+1);
	lseek(fd,0L,SEEK_SET);

	read(fd,tempBuff,filelen);

	memset(KeyCode,0,CMDLINE_LENGT);

	snprintf(KeyCode,CMDLINE_LENGT,"package name=\"%s\" codePath=\"",mExsitProject[projectId].mainAppPackageName);

	char *location1 = strstr(tempBuff,KeyCode);

	bool ret = false;
	if(NULL != location1){

		char *location2 = strstr(location1+strlen(KeyCode),"\"");

		if(NULL !=location2){

			memcpy(appCodePath,location1+strlen(KeyCode),strlen(location1)-strlen(KeyCode)-strlen(location2) );
			ret = true;
		}
	}


	if(NULL != initTempBuff){
		free(initTempBuff);
		initTempBuff = NULL;
	}
	return ret;

}


bool AppDetect::isAppIsInstalled(int projectId)
{

	char cmdStr[CMDLINE_LENGT+1];
	char *ret = NULL;
	memset(cmdStr,0,CMDLINE_LENGT+1);
	snprintf(cmdStr,CMDLINE_LENGT,"%s %s",PM_LIST_CMD_PRE,mExsitProject[projectId].mainAppPackageName);
	ret = getShell(cmdStr);
	if(NULL != ret){
		MY_LOGI("ret  === %s\n",ret);
		char *retInit = ret;
		char *location1 = strstr(ret,"package:");
		if(NULL != location1){

			MY_LOGI("location1  === %s\n",location1);
			char *appCodePath = strtok(location1+strlen("package:"), "=");
			if(NULL != appCodePath && 0 == access(appCodePath,F_OK)){

				MY_LOGI("appCodePath  === %s\n",appCodePath);
				return true;
			}

		}

		free(retInit);
		ret == NULL;
	}
	return false;
}

void AppDetect::installAppForException(int projectId)
{
	char curAppCodePath[CMDLINE_LENGT];

	if(getMainAppCodePath(projectId,curAppCodePath,CMDLINE_LENGT) && 
		0 == access(curAppCodePath,F_OK)){

		char cmdStr[CMDLINE_LENGT];
		memset(cmdStr,0,CMDLINE_LENGT);
		snprintf(cmdStr,CMDLINE_LENGT, "pm install -r %s",curAppCodePath);
		mySystem(SYSTEM_CMD_DEFAULT_RETRY_TIMES*2,SYSTEM_CMD_DEFAULT_SLEEP_TIMES,cmdStr);
	}
	else{
		mySystem(SYSTEM_CMD_DEFAULT_RETRY_TIMES*2,SYSTEM_CMD_DEFAULT_SLEEP_TIMES,mExsitProject[projectId].mainAppInstallCmd);
	}

}

int AppDetect::startMainAppRunning(int projectId,int curPid)
{

	if(checkAppIsRunning(curPid)){
		MY_LOGI("app has started,curPid === %d\n",curPid);
		return curPid;
	}

	int reStartPid = getMainAppPid(projectId);
	MY_LOGI("[%s-%d]%s reStartPid === %d\n", __FILE__, __LINE__,__FUNCTION__,reStartPid);

	if(checkAppIsRunning(reStartPid)){

		MY_LOGI("app has died,but restart now !");
		return reStartPid;
	}

	bool ret = isAppIsInstalled(projectId);

	if(ret){
		reStartPid = 0;
		mySystem(SYSTEM_CMD_DEFAULT_RETRY_TIMES,SYSTEM_CMD_DEFAULT_SLEEP_TIMES,mExsitProject[projectId].mainActivityStartCmd);
		reStartPid = getMainAppPid(projectId);
		MY_LOGE("exec cmd:%s,restart pid == %d\n",mExsitProject[projectId].mainActivityStartCmd,reStartPid);
	}else{
		int appUninstalledTimes  = 0 ;
		MY_LOGI("app has been uninstalled !!!!");

		while(true){
			ret = isAppIsInstalled(projectId);
			if(ret)
				return IVALID_PID_VALUE;
			appUninstalledTimes++;
			if(appUninstalledTimes > MAX_APP_INSTALLING_TINES){
				installAppForException(projectId);
				return UNKOWN_PID_VALUE;
			}
			sleep(LOOPER_SLEEP_TINE);
		}
	}

	return reStartPid;
}

void AppDetect::exceptionPropUpdate(E_EXCEPTION_OP_TYPE opType)
{
	char propertyStr[PROPERTY_VALUE_MAX];
	int propValue = 0;

	switch(opType){
		case E_RESET_PROP:
			setPropInt(EXCEPTION_COUNTS_PROP, 0);
			break;
		case E_ADD_PROP:
			propValue = getPropInt(EXCEPTION_COUNTS_PROP);
			propValue++;
			setPropInt(EXCEPTION_COUNTS_PROP, propValue);
			break;
		default:break;
	}
}

void AppDetect::exceptionDeal()
{
	if(1 == getPropInt(EXCEPTION_ENABLE_PROP))  //enable default
		return;
	int exceptionDealTimes = getPropInt(EXCEPTION_COUNTS_PROP);
	MY_LOGE("Something crash happed,should do exception,exceptionDealTimes === %d\n",exceptionDealTimes);
	if(exceptionDealTimes >= MAX_EXCEPTION_TIMES){
		exceptionPropUpdate(E_RESET_PROP);
		mySystem(SYSTEM_CMD_DEFAULT_RETRY_TIMES,SYSTEM_CMD_DEFAULT_SLEEP_TIMES,MSTER_CLEAR_CMD);
	}else{
		exceptionPropUpdate(E_ADD_PROP);
		mySystem(SYSTEM_CMD_DEFAULT_RETRY_TIMES,SYSTEM_CMD_DEFAULT_SLEEP_TIMES,REBOOT_CMD);
	}
}

int AppDetect::mainAppMonitorLooper(int projectId)
{
	int mainAppPid = getMainAppPid(projectId);
	int appRestartTimes = 0;
	int appNormalRunTimes  = 0;

    MY_LOGE("[%s-%d]%s start", __FILE__, __LINE__,__FUNCTION__);
	int prePid = getMainAppPid(projectId);

	while(1){

		int curPid = startMainAppRunning(projectId,mainAppPid);
		if(curPid != mainAppPid){
			if(IVALID_PID_VALUE == curPid){

				MY_LOGE("app has reinstalled,should not calculator times varible");
				mainAppPid = getMainAppPid(projectId);
				exceptionPropUpdate(E_RESET_PROP);
				continue;
			}
			appRestartTimes++;
			appNormalRunTimes = 0;

			MY_LOGE("appRestartTimes == %d", appRestartTimes);
			if(appRestartTimes >= MAX_EXCEPTION_TIMES){
				exceptionDeal();
			}
			mainAppPid = getMainAppPid(projectId);
		}else{
			appNormalRunTimes++;
			if(appNormalRunTimes >= MAX_NORMAL_RUN_TIMES)
				appRestartTimes = 0;

			if(appNormalRunTimes >= MAX_NORMAL_RUN_TIMES2)
				exceptionPropUpdate(E_RESET_PROP);
		}

		sleep(APP_CHECK_INTERVAL);
	}

	return 0;
}

}
