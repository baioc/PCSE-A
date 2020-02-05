$(eval $(call clear-module-vars))
LOCAL_MODULE_PATH := $(call my-dir)

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := test2
LOCAL_PROCESS_SRC := test2.c
$(eval $(call build-test-process))

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := procKill
LOCAL_PROCESS_SRC := procKill.c
$(eval $(call build-test-process))

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := procExit
LOCAL_PROCESS_SRC := procExit.c
$(eval $(call build-test-process))

$(eval $(call build-test-module))
