# Build pngs from xml files

all: pilot.png select.png target.png

%.png: %.xml gamepad.xsl gamepad.svg
	xsltproc gamepad.xsl $< | rsvg-convert -z 0.5 /dev/stdin >$@
