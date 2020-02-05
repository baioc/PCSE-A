$(eval $(call clear-module-vars))
LOCAL_MODULE_PATH := $(call my-dir)

# Build the test only if semaphore are available.
ifeq ("$(filter WITH_SEM,$(TESTS_OPTIONS))", "WITH_SEM")

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := test14
LOCAL_PROCESS_SRC := test14-sem.c
$(eval $(call build-test-process))

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := proc14_1
LOCAL_PROCESS_SRC := proc14_1-sem.c
$(eval $(call build-test-process))

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := proc14_2
LOCAL_PROCESS_SRC := proc14_2-sem.c
$(eval $(call build-test-process))


# Build the test only if message queues are available.
else ifeq ("$(filter WITH_MSG,$(TESTS_OPTIONS))", "WITH_MSG")

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := test14
LOCAL_PROCESS_SRC := test14-msg.c
$(eval $(call build-test-process))

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := psender1
LOCAL_PROCESS_SRC := psender1-msg.c
$(eval $(call build-test-process))

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := psender2
LOCAL_PROCESS_SRC := psender2-msg.c
$(eval $(call build-test-process))

else
$(error "WITH_SEM" or "WITH_MSG" must be defined in TESTS_OPTIONS)
endif

$(eval $(call build-test-module))
