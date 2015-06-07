# Build pngs from xml files

all: \
	pilot.svg \
	pilot.png \
	pilot@2x.png \
	select.svg \
	select.png \
	select@2x.png \
	target.svg \
	target.png \
	target@2x.png

%.svg: %.xml gamepad.xsl gamepad.svg
	xsltproc gamepad.xsl $< > $@
	ls $@

%@2x.png: %.svg
	ls $<
	rsvg-convert -w 1440 $< -o $@
	ls $<
	optipng -quiet -o5 $@
	ls $<

%.png: %.svg
	rsvg-convert -w 720 $< -o $@
	optipng -quiet -o5 $@
