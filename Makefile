# Requested compiler support: c17, c++20
# REquested dependencies: cpp-gsl, proj
# PROJ is a generic coordinate transformation library: https://proj.org/
# GSL - Guidelines Support Library: https://github.com/microsoft/GSL

CXX = g++-11
CC = gcc-11

egg_TARGET = gravpart

egg_SOURCES = geoproject.c gravitypartition.cc main.cc
egg_SOURCE_PATH = src

egg_FLAGS += -Wall
egg_RELEASE_FLAGS := -O3
egg_DEBUG_FLAGS += -g

egg_CFLAGS += -std=c17
egg_CXXFLAGS += -std=c++20

# MACOSX

ifeq ($(shell uname -s),Darwin)
egg_FLAGS += -I/usr/local/opt/proj/include/
egg_FLAGS += -I/usr/local/include
egg_LDFLAGS += -L/usr/local/opt/proj/lib/
egg_LDLIBS += -lproj
egg_FRAMEWORKS += -framework CoreFoundation
GENERIC = false
endif

# generic UNIX
# this assumes that gsl and proj includes and libraries are already in the system paths.
ifeq ($(GENERIC),true)
egg_LDLIBS += -lproj
endif

include ./eggmakelib/engine.mk
