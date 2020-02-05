$(eval $(call clear-module-vars))
LOCAL_MODULE_PATH := $(call my-dir)

# Build the test only if semaphore are available.
ifeq ("$(filter WITH_SEM,$(TESTS_OPTIONS))", "WITH_SEM")

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := test17
LOCAL_PROCESS_SRC := test17-sem.c
$(eval $(call build-test-process))

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := proc17_1
LOCAL_PROCESS_SRC := proc17_1-sem.c
$(eval $(call build-test-process))

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := proc17_2
LOCAL_PROCESS_SRC := proc17_2-sem.c
$(eval $(call build-test-process))


# Build the test only if message queues are available.
else ifeq ("$(filter WITH_MSG,$(TESTS_OPTIONS))", "WITH_MSG")

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := test17
LOCAL_PROCESS_SRC := test17-msg.c
$(eval $(call build-test-process))

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := proc_return
LOCAL_PROCESS_SRC := proc_return.c
$(eval $(call build-test-process))


else
$(error "WITH_SEM" or "WITH_MSG" must be defined in TESTS_OPTIONS)
endif

$(eval $(call build-test-module))
