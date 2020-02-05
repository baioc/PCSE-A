$(eval $(call clear-module-vars))
LOCAL_MODULE_PATH := $(call my-dir)

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := test6
LOCAL_PROCESS_SRC := test6.c
$(eval $(call build-test-process))

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := proc6_1
LOCAL_PROCESS_SRC := proc6_1.c
$(eval $(call build-test-process))

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := proc6_2
LOCAL_PROCESS_SRC := proc6_2.c
$(eval $(call build-test-process))

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := proc6_3
LOCAL_PROCESS_SRC := proc6_3.c
$(eval $(call build-test-process))

$(eval $(call build-test-module))
