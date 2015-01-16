silk_version = 1.0.9
silk_extracted_directory = SILK_SDK_SRC_v$(silk_version)
silk_flavour_dir = SILK_SDK_SRC_$(SILK_FLAVOUR)_v$(silk_version)
silk_src_dir = $(silk_extracted_directory)/${silk_flavour_dir}
silk_lib = libSKP_SILK_SDK.a

BUILT_SOURCES = $(silk_extracted_directory) $(silk_src_dir)

if BUILD_WIN32
#apply the patch fails under mingw, and it is unuseful.
$(silk_src_dir):
else
$(silk_src_dir):
	cp -f $(srcdir)/patch_pic.diff $(silk_src_dir)/.
	cd $(silk_src_dir) && $(PATCH) -p0 < patch_pic.diff
	rm -f $(silk_src_dir)/patch_pic.diff
endif

extract-sources: $(silk_src_dir)


# Call Skype Makefile to build the library
all-local: $(silk_src_dir)
	cd $(silk_src_dir) && $(MAKE) AS="$(AS)" AR="$(AR)" RANLIB="$(RANLIB)" CC="$(CC) $(CFLAGS)" LD="$(LD)" $(SILK_MAKE_OPTIONS) $(AM_MAKEFLAGS) lib
check-local: $(silk_src_dir)
	cd $(silk_src_dir) && $(MAKE) $(AM_MAKEFLAGS) test
clean-local: $(silk_src_dir)
	cd $(silk_src_dir) && $(MAKE) $(AM_MAKEFLAGS) clean

install-data-local: $(silk_src_dir)
	$(INSTALL) -D $(silk_src_dir)/$(silk_lib) $(DESTDIR)$(prefix)/lib/$(silk_lib)

DISTCLEANFILES = $(BUILT_SOURCES)
