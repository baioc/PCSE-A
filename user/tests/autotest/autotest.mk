$(eval $(call clear-module-vars))
LOCAL_MODULE_PATH := $(call my-dir)

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := autotest
LOCAL_PROCESS_SRC := main.c
$(eval $(call build-test-process))
$(eval $(call build-test-module))
