silk_version = 1.0.8
silk_directory = SILK_SDK_SRC_v$(silk_version)
silk_arm_dir = SILK_SDK_SRC_ARM_v$(silk_version)
silk_fix_dir = SILK_SDK_SRC_FIX_v$(silk_version)
silk_flp_dir = SILK_SDK_SRC_FLP_v$(silk_version)

silk_zip = $(silk_directory).zip
silk_url = http://developer.skype.com/silk/$(silk_zip)

downloaded_files = $(silk_zip) $(license)
checkmarks = $(downloaded_files:%=%.check)
BUILT_SOURCES = $(downloaded_files) $(checkmarks) $(silk_directory)

checksum_dir = $(srcdir)/known_checksums
checksum_files = $(addprefix $(checksum_dir)/, $(downloaded_files:%=%.md5))
EXTRA_DIST = $(checksum_files)

$(silk_zip):
	$(WGET) $(silk_url) -O ${silk_zip}


# for check, go to $(srcdir) or ./, depending on where file is
$(checkmarks): %.check: %
	cd $(<D); $(MD5SUM) -c $(checksum_dir)/$(<F).md5
	touch $@

$(silk_directory): $(checkmarks)
	$(UNZIP) $(silk_zip)
	cd $(silk_directory)/$(silk_arm_dir) && $(PATCH) -p0 < ../../patch_pic.diff && make
	cd $(silk_directory)/$(silk_fix_dir) && $(PATCH) -p0 < ../../patch_pic.diff && make
	cd $(silk_directory)/$(silk_flp_dir) && $(PATCH) -p0 < ../../patch_pic.diff && make


DISTCLEANFILES = $(BUILT_SOURCES)
