#ifndef __COMMON_API_H__
#define __COMMON_API_H__

#include <utils/Log.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <cutils/properties.h>
#include <sys/wait.h>  
#include <sys/types.h> 
#include <fcntl.h>
#include <utils/Thread.h>
#include <sys/types.h>  
#include <sys/ioctl.h>  
#include <sys/socket.h>  
#include <net/if.h> 


#define MY_LOG_TAG    "csh_debug_monitor" 
	
//#define DEBUG_ENABLE
#ifdef DEBUG_ENABLE
#define MY_LOGI(...)  __android_log_print(ANDROID_LOG_INFO,MY_LOG_TAG, __VA_ARGS__)
#else
#define MY_LOGI(...)
#endif
#define MY_LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,MY_LOG_TAG, __VA_ARGS__)

#pragma pack(1)

typedef struct sProjectInfo{
	const char *mainAppPackageName;
	const char *mainAppPackagePath;
	const char *mainAppInstalledPath;
	const char *mainActivityStartCmd;
	const char *mainAppInstallCmd;
}S_PROJECT_INFO;

typedef enum{
	IFF_NET_UP = 0x00,
	IFF_NET_DOWN = 0x01,
	IFF_NET_UNKOWN = 0xFF
}E_IFF_STATE;

#pragma pack(0)

#if 0


#define PROJECT_COUNT (sizeof(sExsitProject) /sizeof(S_PROJECT_INFO))
#endif

#define IP_RULE "ip rule add from all lookup main pref 9999"

#define SYSTEM_UI_PROCESS_NAME   "com.android.systemui"

#define CMDLINE_LENGT               (256)

#define FIRST_SLEEP_TIME            (30)


#define LOOPER_SLEEP_TINE           (3)
#define APP_CHECK_INTERVAL          (3)


#define MAX_APP_INSTALLING_CHECK_TIMES   (60)
#define SYSTEMUI_CHECK_COUNTS        (200)    //10 minutes


typedef enum {
	E_REBOOT,
	E_FACTORY_RESET
}E_EXCEPTION_TYPE;


typedef enum{
	E_RESET_PROP,
	E_ADD_PROP
}E_EXCEPTION_OP_TYPE;


#define MAX_APP_INSTALLING_TINES    (20)
#define IVALID_PID_VALUE     (-10)
#define UNKOWN_PID_VALUE     (-11)


#define REBOOT_CMD                  "reboot"
#define MSTER_CLEAR_CMD  "rm -rf /cache/recovery/command&&echo --wipe_all  > /cache/recovery/command&&sync&&reboot recovery"

#define EXCEPTION_COUNTS_PROP   "persist.sys.exception_times"
#define EXCEPTION_ENABLE_PROP   "persist.sys.exception_disable"
#define MAX_EXCEPTION_TIMES     (20)

#define MAX_NORMAL_RUN_TIMES    (3600*4)
#define MAX_NORMAL_RUN_TIMES2   (3600*48)

#define SYSTEM_CMD_DEFAULT_RETRY_TIMES   (10)
#define SYSTEM_CMD_DEFAULT_SLEEP_TIMES   (6)


#define IVALID_PID_NOT_EXIT            (-1)
#define IVALID_PID_APP_UNINSTALLED     (-2)

#define PACKAGE_XML_PATH          "/data/system/packages.xml"

#define PM_LIST_CMD_PRE           "pm list package -f"

static int getPropInt(const char* propName)
{
	char propertyStr[PROPERTY_VALUE_MAX]; 
	int ret = 0;
	memset(propertyStr,0,PROPERTY_VALUE_MAX);
	if (property_get(propName, propertyStr, "0") > 0) {
    	ret = atoi(propertyStr);
	}

	return ret;
}

static void setPropInt(const char* propName,int propValue)
{
	char propertyStr[PROPERTY_VALUE_MAX]; 

	memset(propertyStr,0,PROPERTY_VALUE_MAX);
	snprintf(propertyStr,10,"%d",propValue);
	property_set(propName, propertyStr);

	return;
}


static  void mySystem(const unsigned int retryMaxTimes,const unsigned int sleepTimes,const char* format, ...)
{
	char cmdstr[CMDLINE_LENGT];
	unsigned int retryTimes = 0;
    va_list   args;

    memset(cmdstr, 0, sizeof(cmdstr));

    va_start(args,format);
    vsnprintf(cmdstr, sizeof(cmdstr)-1, format,   args);
    va_end(args);

	while (true)
	{
		int stat = system(cmdstr);
		if(WIFEXITED(stat) && WEXITSTATUS(stat) == 0)
			break;
		sleep(1);
		retryTimes++;
		if(retryTimes >= retryMaxTimes)
			break;
	}
	sleep(sleepTimes);
}

static char *_strdup(const char *s)
{
	char *ptr = NULL;
	ptr = strdup(s);
	if(NULL == ptr)
	{
		return NULL;
	}

	return ptr;
}

 
static char *getShell(const char *cmd)
{
	if(cmd == NULL)
		return NULL;
 
	FILE *fp = NULL;
	char *ptr = NULL;
	char *tag = NULL;
	char buf[CMDLINE_LENGT] = {0};
	memset(buf,0,CMDLINE_LENGT);
	if(NULL != (fp=popen(cmd,"r")))
	{
		if(NULL != fgets(buf,CMDLINE_LENGT-1,fp))
		{
			tag = buf;
 
			while( ('\n'!= *tag)&&('\0'!=*tag))
			{
				tag++;
			}
			*tag = '\0';
 
			ptr = _strdup(buf);
		}else{
			MY_LOGE("cmd:%s, err\n",cmd);
		}
		pclose(fp);
		return ptr;
	}else{
		return NULL;
	}
}


static int getPidByCmdline(const char *cmdLine)
{
	if(NULL == cmdLine)
		return IVALID_PID_NOT_EXIT;
	DIR *dir;
    struct dirent *ptr;
    int fd;
    char filepath[CMDLINE_LENGT];
    char filetext[CMDLINE_LENGT];
    dir = opendir("/proc");
    if (NULL != dir)
    {
        while ((ptr = readdir(dir)) != NULL){
            if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0))
				continue;
            if (DT_DIR != ptr->d_type)
				continue;
 
            sprintf(filepath, "/proc/%s/cmdline", ptr->d_name);
            fd = open(filepath, O_RDONLY);

            if (fd > 0)
            {
            	memset(filetext,0,CMDLINE_LENGT);
                read(fd, filetext, CMDLINE_LENGT-1);
				
				if(0 == strcmp(filetext,cmdLine)){
					close(fd);
					closedir(dir);						
					MY_LOGI("ptr->d_name === %s\n",ptr->d_name);
					return atoi(ptr->d_name);
				}
				close(fd);
            }
 
        }
        closedir(dir);
    }
	return IVALID_PID_NOT_EXIT;
}
static bool checkAppIsRunning(int pid)
{

	char cmdLine[CMDLINE_LENGT] = {0};
	memset(cmdLine,0,CMDLINE_LENGT);
	snprintf(cmdLine,CMDLINE_LENGT,"/proc/%d",pid);
	MY_LOGI("cmdLine === %s\n",cmdLine);

	if(0 == access(cmdLine,F_OK))
		return true;
	return false;
}

static E_IFF_STATE getNetIfState(const char* net_name)
{
	int skfd = 0;
	struct ifreq ifr;

	if(NULL == net_name)
			return IFF_NET_UNKOWN;
	
	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd < 0) {
	     printf("%s:%d Open socket error!\n", __FILE__, __LINE__);
	     return IFF_NET_UNKOWN;
	}

	strcpy(ifr.ifr_name, net_name);

	if(ioctl(skfd, SIOCGIFFLAGS, &ifr) <0 ) {
	      close(skfd);
	      return IFF_NET_UNKOWN;
	}

	if(ifr.ifr_flags & IFF_UP ) {
		close(skfd);
	    return IFF_NET_UP;
	} 
	
	close(skfd);
	return IFF_NET_DOWN;
}

#endif

