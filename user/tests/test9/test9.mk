$(eval $(call clear-module-vars))
LOCAL_MODULE_PATH := $(call my-dir)

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := test9
LOCAL_PROCESS_SRC := test9.c
LOCAL_CFLAGS := -O0
$(eval $(call build-test-process))

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := nothing
LOCAL_PROCESS_SRC := nothing.c
$(eval $(call build-test-process))

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := test_regs2
LOCAL_PROCESS_SRC := test_regs2.c
$(eval $(call build-test-process))

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := test_eax
LOCAL_PROCESS_SRC := test_eax.c
$(eval $(call build-test-process))


$(eval $(call build-test-module))
