cmake_minimum_required ( VERSION 3.10 )
project ( xbeeb VERSION 1.0.0 )

include ( FindPkgConfig )

#find_package(Qt5 REQUIRED COMPONENTS Core Widgets Gui Charts)
#
#if ( PKG_CONFIG_FOUND )
#  pkg_search_module ( LIBTIFF4 libtiff-4 )
#  pkg_search_module ( LIBRAW libraw )
#  pkg_search_module ( LIBCFITSIO cfitsio )
#  pkg_search_module ( LIBEXIV2 exiv2 )
#else()
#  message ( FATAL_ERROR "pkg-config not found" )
#endif()
#
#if ( NOT LIBTIFF4_FOUND )
#	message ( FATAL_ERROR "libtiff required for build" )
#endif()
#if ( NOT LIBRAW_FOUND )
#	message ( FATAL_ERROR "libraw required for build" )
#endif()
#if ( NOT LIBCFITSIO_FOUND )
#	message ( FATAL_ERROR "libcfitsio required for build" )
#endif()
#if ( NOT LIBEXIV2_FOUND )
#	message ( FATAL_ERROR "libexiv2 required for build" )
#endif()

add_executable ( xbeeb
	6522Via.c
	Acia.c
	ADC.c
	Beeb.c
	Bitmap.c
	Compat.c
	Crtc.c
	Disk.c
	Display.c
	Econet.c
	EFS.c
	Fred.c
	InfFS.c
	InfoWindow.c
	Jim.c
	Keyboard.c
	KeymapLegend.c
	KeymapStrict.c
	Memory.c
	Modes.c
	Options.c
	RomSelect.c
	Screen.c
	SerialUla.c
	Sheila.c
	Sound.c
	SoundOSS.c
	SystemVia.c
	Teletext.c
	TubeUla.c
	UserVia.c
	VideoUla.c
)

#target_include_directories ( xbeeb
#		PRIVATE
#		${Qt5Gui_INCLUDE_DIRS}
#		${Qt5Wisgets_INCLUDE_DIRS}
#		${Qt5Core_INCLUDE_DIRS}
#)

#target_compile_options( DeepSkyStacker PRIVATE -fPIC)

target_link_libraries( xbeeb
#  Qt::Core
	Xext
	X11
)
