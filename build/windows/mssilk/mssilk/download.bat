SET curdir=%CD%
SET srcdir=..\..\..\..\sdk

REM define the need variables
SET silk_version=1.0.9
SET silk_extracted_directory=SILK_SDK_SRC_v%silk_version%
SET silk_arm_dir=SILK_SDK_SRC_ARM_v%silk_version%
SET silk_fix_dir=SILK_SDK_SRC_FIX_v%silk_version%

SET silk_zip=%silk_extracted_directory%.zip
SET silk_url=http://www.belledonne-communications.com/downloads/contrib/%silk_zip%

REM define the tools
SET wget_cmd=wget.exe
SET unzip_cmd=unzip.exe

REM get the sources
IF NOT EXIST %srcdir%\%silk_zip% (
	%wget_cmd% %silk_url% -O %srcdir%\%silk_zip%
)

REM extract the sources
IF NOT EXIST %srcdir%\%silk_extracted_directory% (
	MD %srcdir%\%silk_extracted_directory%
	%unzip_cmd% %srcdir%\%silk_zip% %silk_arm_dir%\* %silk_fix_dir%\* -d %srcdir%\%silk_extracted_directory%
)
