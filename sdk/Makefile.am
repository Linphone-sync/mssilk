silk_version = 1.0.9
silk_extracted_directory = SILK_SDK_SRC_v$(silk_version)
silk_flavour_dir = SILK_SDK_SRC_$(SILK_FLAVOUR)_v$(silk_version)
silk_src_dir = $(silk_extracted_directory)/$(silk_flavour_dir)
silk_lib = libSKP_SILK_SDK.a


# For some reason a directory target with no dependency is never executed..
# Hence, this empty task 'silk_src_dir_echo' fixes the issue.
silk_src_dir_echo:
$(silk_src_dir): silk_src_dir_echo
	mkdir -p $(silk_extracted_directory)
	cp -r $(srcdir)/$(silk_src_dir) $(silk_extracted_directory)

# Call Skype Makefile to build the library
all-local: $(silk_src_dir)
	cd $(silk_src_dir) && $(MAKE) AS="$(AS)" AR="$(AR)" RANLIB="$(RANLIB)" CC="$(CC) $(CFLAGS)" LD="$(LD)" $(SILK_MAKE_OPTIONS) $(AM_MAKEFLAGS) lib
check-local: $(silk_src_dir)
	cd $(silk_src_dir) && $(MAKE) $(AM_MAKEFLAGS) test
clean-local: $(silk_src_dir)
	cd $(silk_src_dir) && $(MAKE) $(AM_MAKEFLAGS) clean

install-data-local: $(silk_src_dir)
	$(INSTALL) -D $(silk_src_dir)/$(silk_lib) $(DESTDIR)$(prefix)/lib/$(silk_lib)

