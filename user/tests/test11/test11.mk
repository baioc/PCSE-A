$(eval $(call clear-module-vars))
LOCAL_MODULE_PATH := $(call my-dir)

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := test11
LOCAL_PROCESS_SRC := test11.c
$(eval $(call build-test-process))

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := proc_mutex
LOCAL_PROCESS_SRC := proc_mutex.c
$(eval $(call build-test-process))

$(eval $(call build-test-module))
