# extra documentation
doc_DATA=	bugs-core.html layout.png readme-core.txt recording.format.txt
docdir=		@DOCDIR@

# man pages
man_MANS=	NeoPop-SDL.6 neopoprc.5
MANFMT=		@MANFMT@
EXTRA_DIST=	NeoPop-SDL.mdoc NeoPop-SDL.man \
		neopoprc.mdoc neopoprc.man \
		${doc_DATA}
DISTCLEANFILES=	NeoPop-SDL.6 neopoprc.5

NeoPop-SDL.6: NeoPop-SDL.${MANFMT} Makefile
	sed "s|DOCDIR|${docdir}|g" ${srcdir}/NeoPop-SDL.${MANFMT} \
		> $@.$$$$ && \
		mv $@.$$$$ $@

neopoprc.5: neopoprc.${MANFMT} Makefile
	cp ${srcdir}/neopoprc.${MANFMT} $@
