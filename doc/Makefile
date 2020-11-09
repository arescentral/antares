.PHONY: all
all: interface.pdf

%.pdf: %.dot
	dot -Tpdf -o $@ $<
