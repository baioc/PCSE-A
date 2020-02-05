$(eval $(call clear-module-vars))
LOCAL_MODULE_PATH := $(call my-dir)

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := test1
LOCAL_PROCESS_SRC := test1.c
$(eval $(call build-test-process))

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := dummy1
LOCAL_PROCESS_SRC := dummy1.c
$(eval $(call build-test-process))

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := dummy2
LOCAL_PROCESS_SRC := dummy2.c
$(eval $(call build-test-process))

$(eval $(call build-test-module))
