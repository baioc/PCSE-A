$(eval $(call clear-module-vars))
LOCAL_MODULE_PATH := $(call my-dir)

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := test21
LOCAL_PROCESS_SRC := test21.c
$(eval $(call build-test-process))

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := shm_checker
LOCAL_PROCESS_SRC := shm_checker.c
$(eval $(call build-test-process))

$(eval $(call build-test-module))
