$(eval $(call clear-module-vars))
LOCAL_MODULE_PATH := $(call my-dir)

ifeq ("$(filter CONS_READ_LINE,$(TESTS_OPTIONS))", "CONS_READ_LINE")

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := test19
LOCAL_PROCESS_SRC := test19-crl.c
$(eval $(call build-test-process))

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := cons_rd0
LOCAL_PROCESS_SRC := cons_rd0-crl.c
$(eval $(call build-test-process))

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := cons_rdN
LOCAL_PROCESS_SRC := cons_rdN-crl.c
$(eval $(call build-test-process))


else ifeq ("$(filter CONS_READ_CHAR,$(TESTS_OPTIONS))", "CONS_READ_CHAR")

# This implementation of the test 19 needs message queues to work.
ifeq ("$(filter WITH_MSG,$(TESTS_OPTIONS))", "")
$(error Test 19: to build with "CONS_READ_CHAR" option, the test requires "WITH_MSG")
endif

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := test19
LOCAL_PROCESS_SRC := test19-crc.c
$(eval $(call build-test-process))

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := cons_reader
LOCAL_PROCESS_SRC := cons_reader.c
$(eval $(call build-test-process))


else
$(error "WITH_SEM" or "WITH_MSG" must be defined in TESTS_OPTIONS)
endif

$(eval $(call build-test-module))
