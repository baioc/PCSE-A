$(eval $(call clear-module-vars))
LOCAL_MODULE_PATH := $(call my-dir)

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := test3
LOCAL_PROCESS_SRC := test3.c
$(eval $(call build-test-process))

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := prio4
LOCAL_PROCESS_SRC := prio4.c
$(eval $(call build-test-process))

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := prio5
LOCAL_PROCESS_SRC := prio5.c
$(eval $(call build-test-process))

$(eval $(call build-test-module))
