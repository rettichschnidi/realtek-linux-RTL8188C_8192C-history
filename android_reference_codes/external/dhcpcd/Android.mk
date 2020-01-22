# Copyright 2006 The Android Open Source Project
ifneq ($(TARGET_SIMULATOR),true)
LOCAL_PATH:= $(call my-dir)

etc_dir := $(TARGET_OUT)/etc/dhcpcd
hooks_dir := dhcpcd-hooks
hooks_target := $(etc_dir)/$(hooks_dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := common.c dhcp.c dhcpcd.c net.c \
	signals.c configure.c if-linux.c lpf.c
#ifeq ($(CUSTOM_WIFI_VENDOR),atheros)
ifeq ($(BOARD_USES_ATH_WIFI),true)# check fryo
LOCAL_SRC_FILES += logger_atheros.c client_atheros.c
else ifeq ($(BOARD_USES_REALTEK_WIFI),true)
LOCAL_SRC_FILES += logger_realtek.c client_realtek.c
else
LOCAL_SRC_FILES += logger.c client.c
endif

LOCAL_C_INCLUDES := $(KERNEL_HEADERS)
LOCAL_SHARED_LIBRARIES := libc libcutils
ifeq ($(BOARD_USES_REALTEK_WIFI),true)
LOCAL_SHARED_LIBRARIES += libhardware_legacy
endif
LOCAL_MODULE = dhcpcd
LOCAL_MODULE_TAGS := user
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := showlease.c
LOCAL_C_INCLUDES := $(KERNEL_HEADERS)
LOCAL_SHARED_LIBRARIES := libc
LOCAL_MODULE = showlease
LOCAL_MODULE_TAGS := debug
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := dhcpcd.conf
LOCAL_MODULE_TAGS := user
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(etc_dir)
LOCAL_SRC_FILES := android.conf
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := dhcpcd-run-hooks
LOCAL_MODULE_TAGS := user
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_PATH := $(etc_dir)
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := 20-dns.conf
LOCAL_MODULE_TAGS := user
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(hooks_target)
LOCAL_SRC_FILES := $(hooks_dir)/$(LOCAL_MODULE)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := 95-configured
LOCAL_MODULE_TAGS := user
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(hooks_target)
LOCAL_SRC_FILES := $(hooks_dir)/$(LOCAL_MODULE)
include $(BUILD_PREBUILT)

endif
