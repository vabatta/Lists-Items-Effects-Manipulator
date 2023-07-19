set(VERSION 4_019)

vcpkg_download_distfile(
	ARCHIVE
	URLS "https://www.akenotsuki.com/misc/srell/srell${VERSION}.zip"
	FILENAME "srell${VERSION}.zip"
	SHA512 5bc333adf6c8f2ec1ac853bca3af78978e6d0146fb08567cc9c6f27066673a1b618e9098ff047093946fbf54341284790411f10df8dfdef2fdcdbfedfef37cfd
)

vcpkg_extract_source_archive(
	SOURCE_PATH
	ARCHIVE "${ARCHIVE}"
	NO_REMOVE_ONE_LEVEL
)

file(INSTALL
	"${SOURCE_PATH}/srell.hpp"
	"${SOURCE_PATH}/srell_ucfdata2.hpp"
	"${SOURCE_PATH}/srell_updata2.hpp"
	DESTINATION "${CURRENT_PACKAGES_DIR}/include"
)

file(INSTALL "${SOURCE_PATH}/license.txt" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)