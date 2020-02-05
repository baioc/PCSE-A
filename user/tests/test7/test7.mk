$(eval $(call clear-module-vars))
LOCAL_MODULE_PATH := $(call my-dir)

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := test7
LOCAL_PROCESS_SRC := test7.c
$(eval $(call build-test-process))

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := timer
LOCAL_PROCESS_SRC := timer.c
$(eval $(call build-test-process))

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := timer1
LOCAL_PROCESS_SRC := timer1.c
$(eval $(call build-test-process))

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := sleep_pr1
LOCAL_PROCESS_SRC := sleep_pr1.c
$(eval $(call build-test-process))

$(eval $(call build-test-module))
