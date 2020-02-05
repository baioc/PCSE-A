$(eval $(call clear-module-vars))
LOCAL_MODULE_PATH := $(call my-dir)

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := test5
LOCAL_PROCESS_SRC := test5.c
$(eval $(call build-test-process))

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := no_run
LOCAL_PROCESS_SRC := no_run.c
$(eval $(call build-test-process))

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := waiter
LOCAL_PROCESS_SRC := waiter.c
$(eval $(call build-test-process))

$(eval $(call build-test-module))
