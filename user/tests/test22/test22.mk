$(eval $(call clear-module-vars))
LOCAL_MODULE_PATH := $(call my-dir)

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := test22
LOCAL_PROCESS_SRC := test22.c
$(eval $(call build-test-process))

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := malicious
LOCAL_PROCESS_SRC := malicious.c
$(eval $(call build-test-process))

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := pagefault
LOCAL_PROCESS_SRC := pagefault.c
$(eval $(call build-test-process))

$(eval $(call build-test-module))
