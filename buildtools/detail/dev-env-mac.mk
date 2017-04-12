# {{{ global settings

dev-env-list+=mac

mac.CFLAGS+=-pipe
mac.CXXFLAGS+=-pipe
mac.MFLAGS+=-pipe -x objective-c
mac.MMFLAGS+=-pipe -x objective-c++

ifneq ($(DEBUG),0)
mac.CFLAGS+=-g
mac.CXXFLAGS+=-g
endif

mac.LDFLAGS.share:=--shared

mac.default-lib-type:=static
mac.make-static-lib-name=lib$1.a
mac.make-dynamic-lib-name=lib$1.so
mac.make-executable-name=$1
mac.export-symbols=-u $1
mac.env-inc=mac
mac.regular-path=$1

# }}}
# {{{ validate

mac.check=$(call assert-not-null,MAC_PLATFORM_NAME) \
             $(call assert-not-null,MAC_PLATFORM_VERSION) \
             $(call assert-not-null,$(MAC_PLATFORM_NAME).compiler) \
             $(call assert-not-null,$(MAC_PLATFORM_NAME).CFLAGS) \
             $(call assert-not-null,$(MAC_PLATFORM_NAME).CXXFLAGS) \
             $(call assert-not-null,$(MAC_PLATFORM_NAME).MFLAGS) \
             $(call assert-not-null,$(MAC_PLATFORM_NAME).MMFLAGS) \
             $(call assert-not-null,$(MAC_PLATFORM_NAME).LDFLAGS) \
             $(call assert-file-exists,$(MAC_SDK_PREFIX))

# }}}
# {{{ sdk MAC

MAC_XCODE_ROOT:=$(if $(filter mac,$(OS_NAME)),$(shell xcode-select -print-path))
MAC_PLATFORM_VERSION_LIST:=10.12 10.11 10.10 10.9 10.8 10.7 10.6 10.5
MAC_PLATFORM_NAME?=MacOSX

mac_platform_sdk_patk=$(MAC_XCODE_ROOT)/Platforms/$(MAC_PLATFORM_NAME).platform/Developer/SDKs/$(MAC_PLATFORM_NAME)$1.sdk


MAC_PLATFORM_VERSION?=$(word 1,$(foreach v,$(MAC_PLATFORM_VERSION_LIST),$(if $(wildcard $(call mac_platform_sdk_patk,$v)),$v)))

MAC_PLATFORM_BIN_PATH:=$(MAC_XCODE_ROOT)/Toolchains/XcodeDefault.xctoolchain/usr/bin

MacOSX.compiler ?= $(MAC_PLATFORM_BIN_PATH)/$(if $(filter gcc,$1),clang,$(if $(filter g++,$1),clang++,$1))

MacOSX.CPPFLAGS ?= -D_APPLE -DTARGET_OS_MAC

MacOSX.CFLAGS ?= \
                   -std=c99 \
                   -fexceptions \

MacOSX.CXXFLAGS ?= \
                   -fexceptions \

MacOSX.MFLAGS ?= \
                   $(MacOSX.CFLAGS) \
                   -fmessage-length=0 \
                   -fpascal-strings \
                   -fasm-blocks \
                   -fobjc-abi-version=2 \
                   -fobjc-legacy-dispatch \

MacOSX.MMFLAGS ?= \
                   -fmessage-length=0 \
                   -fpascal-strings \
                   -fasm-blocks \
                   -fobjc-abi-version=2 \
                   -fobjc-legacy-dispatch \

MacOSX.LDFLAGS ?=  -mmacosx-version-min=$(MAC_PLATFORM_VERSION) \
                   -Xlinker \
                   -objc_abi_version \
                   -Xlinker 2 

# }}}
# {{{ toolset def

MAC_SDK_PREFIX:=$(call mac_platform_sdk_patk,$(MAC_PLATFORM_VERSION))

mac.GCC ?= $(call $(MAC_PLATFORM_NAME).compiler,gcc)
mac.CXX ?= $(call $(MAC_PLATFORM_NAME).compiler,g++)
mac.CC = $(mac.GCC)
mac.LD = $(mac.CC)
mac.AR = $(MAC_PLATFORM_BIN_PATH)/ar
mac.STRIP = $(MAC_PLATFORM_BIN_PATH)/strip
mac.OBJCOPY = $(MAC_PLATFORM_BIN_PATH)/objcopy
mac.IBTOOL = ibtool

mac.IBFLAGS = \
          --errors \
          --warnings \
          --notices \
          --output-format human-readable-text \
          --sdk $(MAC_SDK_PREFIX)

mac.CPPFLAGS+=\
           -isysroot $(MAC_SDK_PREFIX) \
           -F$(MAC_SDK_PREFIX)/System/Library/Frameworks \
           $($(MAC_PLATFORM_NAME).CPPFLAGS)

mac.CFLAGS += $($(MAC_PLATFORM_NAME).CFLAGS)
mac.CXXFLAGS += $($(MAC_PLATFORM_NAME).CXXFLAGS)
mac.MFLAGS += $($(MAC_PLATFORM_NAME).MFLAGS)
mac.MMFLAGS += $($(MAC_PLATFORM_NAME).MMFLAGS)

mac.LDFLAGS += \
           -isysroot $(MAC_SDK_PREFIX) \
           $($(MAC_PLATFORM_NAME)LDFLAGS) \
           $(addprefix -L,$(sort $(dir $(libraries)))) \

mac.PLUTILFLAGS += -convert binary1

mac.linker.c:=$(mac.GCC)
mac.linker.cpp:=$(mac.CXX)
mac.linker.obj-c:=$(mac.GCC)
mac.linker.obj-cpp:=$(mac.CXX)

# }}}
