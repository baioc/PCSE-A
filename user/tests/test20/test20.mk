$(eval $(call clear-module-vars))
LOCAL_MODULE_PATH := $(call my-dir)

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := test20
LOCAL_PROCESS_SRC := test20.c
$(eval $(call build-test-process))

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := launch_philo
LOCAL_PROCESS_SRC := launch_philo.c
$(eval $(call build-test-process))

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := philosophe
LOCAL_PROCESS_SRC := philosophe.c
$(eval $(call build-test-process))

$(eval $(call build-test-module))
