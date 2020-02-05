$(eval $(call clear-module-vars))
LOCAL_MODULE_PATH := $(call my-dir)

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := test8
LOCAL_PROCESS_SRC := test8.c
$(eval $(call build-test-process))

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := suicide_launcher
LOCAL_PROCESS_SRC := suicide_launcher.c
$(eval $(call build-test-process))

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := suicide
LOCAL_PROCESS_SRC := suicide.c
$(eval $(call build-test-process))

$(eval $(call build-test-module))
