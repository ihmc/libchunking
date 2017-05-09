#
# Makefile.in
# author: Giacomo Benincasa	(gbenincasa@ihmc.us)
#

ARCH = $(shell sh ../../scripts/guessArch.sh)
MakefileArch=../../scripts/Makefile.$(ARCH)
include $(MakefileArch)

NOMADS_HOME = ../../../..
UTIL_HOME = $(NOMADS_HOME)/util

cpp = g++
ARCH_FLAGS+=-DUNIX -DLITTLE_ENDIAN_SYSTEM
LD_FLAGS = -lpthread

#options=-Wall -Wextra -Werror 
options=

INCLUDES=-I.. -I../ -I../../../externals/include

cppflags= -g -fPIC $(ARCH_FLAGS) $(LD_FLAGS) -DENABLE_DEBUG -DERROR_CHECKING $(options) $(INCLUDES)
sources = $(wildcard ../*.cpp)
sources += $(wildcard ../*.c)
objects = $(sources:../%.cpp=%.o)
objects += $(sources:../%.c=%.o)
LIBS=$(EXTERNALS_LIB)
