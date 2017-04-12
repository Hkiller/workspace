ifeq ($(OS_NAME),cygwin)
cygwin-path-to-win=$(shell cygpath -w $1)
else
cygwin-path-to-win=$1
endif
