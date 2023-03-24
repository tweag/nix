libraries += libnixc

libnixc_NAME = libnixc

libnixc_DIR := $(d)

libnixc_SOURCES := $(wildcard $(d)/*.cc)

libnixc_CXXFLAGS += -I src/libutil -I src/libstore -I src/libexpr -I src/libmain -I src/libfetchers

libnixc_LDFLAGS = $(EDITLINE_LIBS) $(LOWDOWN_LIBS) -pthread

libnixc_LIBS = libstore libutil libexpr libmain libfetchers

$(eval $(call install-file-in, $(d)/nix-c.pc, $(libdir)/pkgconfig, 0644))
