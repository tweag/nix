check: libstore-tests_RUN

programs += libstore-tests

libstore-tests_DIR := $(d)

libstore-tests_INSTALL_DIR :=

libstore-tests_SOURCES := $(wildcard $(d)/*.cc)

libstore-tests_CXXFLAGS += -I src/libutil -I src/libstore -I src/libexpr -I src/libmain

libstore-tests_LIBS = libexpr libmain libstore libutil

libstore-tests_LDFLAGS := -pthread $(GTEST_LIBS)
