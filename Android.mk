LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

FILE_LIST := $(wildcard $(LOCAL_PATH)/*.cpp)
LOCAL_SRC_FILES := $(FILE_LIST:$(LOCAL_PATH)/%=%)

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	liblog \
	libutils \
	libbinder \
    libnetutils 


LOCAL_MODULE:= wrtmonitor

include $(BUILD_EXECUTABLE)
