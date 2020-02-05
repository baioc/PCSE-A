$(eval $(call clear-module-vars))
LOCAL_MODULE_PATH := $(call my-dir)

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := test4
LOCAL_PROCESS_SRC := test4.c
$(eval $(call build-test-process))

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := busy1
LOCAL_PROCESS_SRC := busy1.c
$(eval $(call build-test-process))

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := busy2
LOCAL_PROCESS_SRC := busy2.c
$(eval $(call build-test-process))

$(eval $(call build-test-module))
