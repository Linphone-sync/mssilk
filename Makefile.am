pluginsdir = $(libdir)/mediastreamer/plugins

plugins_LTLIBRARIES=libmssilk.la

libmssilk_la_SOURCES=silk.c

SILK_DIR=$(CURDIR)/downloads/SILK_SDK_SRC_v1.0.8/SILK_SDK_SRC_FIX_v1.0.8

SILK_LIBS = -L$(SILK_DIR) -lSKP_SILK_SDK
libmssilk_la_LIBADD=\
	$(ORTP_LIBS) \
        $(MEDIASTREAMER_LIBS) \
        $(SILK_LIBS)

libmssilk_la_LDFLAGS=-module -no-undefined

SILK_CFLAGS = -I$(SILK_DIR)/interface
AM_CFLAGS=      $(SILK_CFLAGS) \
                $(MEDIASTREAMER_CFLAGS)


