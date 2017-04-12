client.output:=$(if $(client.chanel),client-$(client.chanel),client)
client.ut:=0

server.deploy?=dev
server.output:=$(if $(server.deploy),server-$(server.deploy),server)
server.default-lib-type:=static

editor.env=$(OS_NAME)
editor.output=editor
editor.default-lib-type:=static
editor.ut:=0

using-domain-list=tools

ifeq ($(OS_NAME),mac)
client.env=ios
using-domain-list+=client editor
endif

ifeq ($(OS_NAME),cygwin)
client.env=mingw-native
editor.env=mingw-native
using-domain-list+=client 
endif

ifeq ($(OS_NAME),linux32)
using-domain-list+=server 
endif

ifeq ($(OS_NAME),linux64)
using-domain-list+=server 
endif

ifeq ($(NDK),)
client.ignore-types+=android
else
using-domain-list+=client client-android
endif

ifeq ($(emscripten),1)
client.env=emscripten
client.output:=$(client.output)-emscripten
endif

ifeq ($(flex),0)
client.env=flex
client.output:=$(client.output)-flex-d
endif

ifeq ($(flex),1)
client.env=flex
client.output:=$(client.output)-flex
endif

ifeq ($(flex),2)
client.env=flex
client.output:=$(client.output)-flex-r
endif


