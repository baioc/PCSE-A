$(eval $(call clear-module-vars))
LOCAL_MODULE_PATH := $(call my-dir)

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := test18
LOCAL_PROCESS_SRC := test18.c
$(eval $(call build-test-process))

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := proc18_1
LOCAL_PROCESS_SRC := proc18_1.c
$(eval $(call build-test-process))

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := proc18_2
LOCAL_PROCESS_SRC := proc18_2.c
$(eval $(call build-test-process))

$(eval $(call build-test-module))
