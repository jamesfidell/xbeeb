cmake_minimum_required ( VERSION 3.10 )
project ( xbeeb VERSION 1.0.0 )

include ( FindPkgConfig )

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

target_link_libraries( xbeeb
	Xext
	X11
)
