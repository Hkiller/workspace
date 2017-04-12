# {{{ global settings

dev-env-list+=ios

ios.CFLAGS+=
ios.CXXFLAGS+=
ios.MFLAGS+=-pipe -x objective-c
ios.MMFLAGS+=-pipe -x objective-c++

ifneq ($(DEBUG),0)
ios.CFLAGS+=-g
ios.CXXFLAGS+=-g
endif

ios.LDFLAGS.share:=--shared

ios.default-lib-type:=static
ios.make-static-lib-name=lib$1.a
ios.make-dynamic-lib-name=lib$1.dylib
ios.make-executable-name=$1
ios.export-symbols=$(addprefix -u ,$(foreach m,$1,_$m))

ios.env-inc=ios
ios.regular-path=$1

# }}}
# {{{ validate

ios.check=$(call assert-not-null,IOS_PLATFORM_NAME) \
             $(call assert-not-null,IOS_PLATFORM_VERSION) \
             $(call assert-not-null,$(IOS_PLATFORM_NAME).compiler) \
             $(call assert-not-null,$(IOS_PLATFORM_NAME).CFLAGS) \
             $(call assert-not-null,$(IOS_PLATFORM_NAME).CXXFLAGS) \
             $(call assert-not-null,$(IOS_PLATFORM_NAME).MFLAGS) \
             $(call assert-not-null,$(IOS_PLATFORM_NAME).MMFLAGS) \
             $(call assert-not-null,$(IOS_PLATFORM_NAME).LDFLAGS) \
             $(call assert-not-null,$(IOS_PLATFORM_NAME).TARGET_ARCH) \
             $(call assert-file-exists,$(IOS_SDK_PREFIX))

# }}}
# {{{ sdk iPhoneSimulator

ios-os-version-min?=40200

iPhoneSimulator.compiler ?= $(IOS_XCODE_ROOT)/Toolchains/XcodeDefault.xctoolchain/usr/bin/$(if $(filter gcc,$1),clang,$(if $(filter g++,$1),clang++,$1))

iPhoneSimulator.TARGET_ARCH ?= -arch i386

iPhoneSimulator.CPPFLAGS ?= \
                    -D__IPHONE_OS_VERSION_MIN_REQUIRED=$(ios-os-version-min) \
                    -D_APPLE \
                    -DTARGET_OS_IPHONE \
                    -DTARGET_IPHONE_SIMULATOR

iPhoneSimulator.CFLAGS ?= \
                   -std=c99 \
                   -fexceptions \

iPhoneSimulator.CXXFLAGS ?= \
                   -fexceptions \

iPhoneSimulator.MFLAGS ?= \
                   $(iPhoneSimulator.CFLAGS) \
                   -fmessage-length=0 \
                   -fpascal-strings \
                   -fasm-blocks \
                   -fobjc-abi-version=2 \
                   -fobjc-legacy-dispatch \

iPhoneSimulator.MMFLAGS ?= \
                   -fmessage-length=0 \
                   -fpascal-strings \
                   -fasm-blocks \
                   -fobjc-abi-version=2 \
                   -fobjc-legacy-dispatch \

iPhoneSimulator.LDFLAGS ?=  -arch i386\
                            -stdlib=libc++ \
                            -miphoneos-version-min=6.1 \
                            -Xlinker \
                            -objc_abi_version \
                            -Xlinker 2 

iPhoneSimulator.install-dir?=$(HOME)/Library/Application Support/iPhone Simulator/$(IOS_PLATFORM_VERSION)/Applications

# }}}
# {{{ toolset def

IOS_PLATFORM_VERSION_LIST:=10.3 10.2 10.1 10.0 9.3 9.2 9.1 9.0 8.4 8.3 8.2 8.1 8.0 7.1 7.0 6.1 6.0 5.0
IOS_PLATFORM_NAME?=iPhoneSimulator

IOS_XCODE_ROOT:=$(if $(filter mac,$(OS_NAME)),$(shell xcode-select -print-path))

IOS_PLATFORM_PREFIX:=$(IOS_XCODE_ROOT)/Platforms/$(IOS_PLATFORM_NAME).platform
IOS_PLATFORM_BIN_PATH:=$(IOS_PLATFORM_PREFIX)/Developer/usr/bin

ios_platform_sdk_path=$(IOS_XCODE_ROOT)/Platforms/$(IOS_PLATFORM_NAME).platform/Developer/SDKs/$(IOS_PLATFORM_NAME)$1.sdk

IOS_PLATFORM_VERSION?=$(word 1,$(foreach v,$(IOS_PLATFORM_VERSION_LIST),$(if $(wildcard $(call ios_platform_sdk_path,$v)),$v)))

IOS_SDK_PREFIX:=$(call ios_platform_sdk_path,$(IOS_PLATFORM_VERSION))

ios.GCC = $(call $(IOS_PLATFORM_NAME).compiler,gcc)
ios.CXX = $(call $(IOS_PLATFORM_NAME).compiler,g++)
ios.CC = $(ios.GCC)
ios.LD = $(ios.CC)
ios.AR = $(word 1, $(wildcard $(IOS_PLATFORM_BIN_PATH)/ar)  \
                   $(wildcard $(IOS_XCODE_ROOT)/Toolchains/XcodeDefault.xctoolchain/usr/bin/ar))
ios.STRIP = $(IOS_PLATFORM_BIN_PATH)/strip
ios.OBJCOPY = $(IOS_PLATFORM_BIN_PATH)/objcopy
ios.IBTOOL = ibtool

ios.IBFLAGS = \
          --errors \
          --warnings \
          --notices \
          --output-format human-readable-text \
          --sdk $(IOS_SDK_PREFIX)

ios.CPPFLAGS+=\
           -isysroot $(IOS_SDK_PREFIX) \
           -F$(IOS_SDK_PREFIX)/System/Library/Frameworks \
           $($(IOS_PLATFORM_NAME).CPPFLAGS)

ios.CFLAGS += $($(IOS_PLATFORM_NAME).CFLAGS)
ios.CXXFLAGS += $($(IOS_PLATFORM_NAME).CXXFLAGS)
ios.MFLAGS += $($(IOS_PLATFORM_NAME).MFLAGS)
ios.MMFLAGS += $($(IOS_PLATFORM_NAME).MMFLAGS)

ios.TARGET_ARCH += $($(IOS_PLATFORM_NAME).TARGET_ARCH)

ios.LDFLAGS += \
           -isysroot $(IOS_SDK_PREFIX) \
           $($(IOS_PLATFORM_NAME)LDFLAGS) \
           $(addprefix -L,$(sort $(dir $(libraries)))) \
           $($(IOS_PLATFORM_NAME).LDFLAGS)

ios.PLUTILFLAGS += -convert binary1

ios.linker.c:=$(ios.GCC)
ios.linker.cpp:=$(ios.CXX)
ios.linker.obj-c:=$(ios.GCC)
ios.linker.obj-cpp:=$(ios.CXX)

# }}}
