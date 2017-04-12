ORIGN_OS_NAME:=$(shell uname -s)

ifeq ($(ORIGN_OS_NAME),Linux)
ifeq ($(shell uname -p),x86_64)
OS_NAME:=linux64
else
OS_NAME:=linux32
endif
endif

ifeq ($(ORIGN_OS_NAME),Darwin)
OS_NAME:=mac
endif

ifeq ($(filter CYGWIN%,$(ORIGN_OS_NAME)),$(ORIGN_OS_NAME))
OS_NAME:=cygwin
endif

ifeq ($(OS_NAME),)
$(error unknown orign os name $(ORIGN_OS_NAME))
endif
