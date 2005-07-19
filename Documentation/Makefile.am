# extra documentation
doc_DATA=	bugs-core.html layout.png readme-core.txt recording.format.txt
docdir=		@DOCDIR@

# man pages
MANFMT=		@MANFMT@
man_MANS=	NeoPop-SDL.6
man5_MANS=	neopoprc.${MANFMT}
EXTRA_DIST=	NeoPop-SDL.mdoc NeoPop-SDL.man \
		neopoprc.mdoc neopoprc.man \
		${doc_DATA}
DISTCLEANFILES=	NeoPop-SDL.6 neopoprc.5

NeoPop-SDL.6: NeoPop-SDL.${MANFMT} Makefile
	sed "s|DOCDIR|${docdir}|g" ${srcdir}/NeoPop-SDL.${MANFMT} \
		> $@.$$$$ && \
		mv $@.$$$$ $@

SUFFIXES=.man .mdoc .html

.mdoc.man:
	mdoc2man $< > $@.$$$$ && diff -I NiH $@.$$$$ $@ || mv $@.$$$$ $@; \
	rm -f $@.$$$$

.mdoc.html:
	nroff -mdoc2html $< | sed -e "s,../html1/,," > $@.$$$$ && mv $@.$$$$ $@
