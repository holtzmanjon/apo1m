L2HFLAGS=-local_icons -info "0" -no_navigation
HTMLDIR=public_html

all: 1m newinst man

1m:1m-ps 1m-html
1m-html: 1m.tex
	latex2html -dir $(HTMLDIR)/1m -split +1 $(L2HFLAGS) -t "NMSU 1m telescope" 1m
1m-ps: 1m.tex
	latex 1m
	latex 1m
	dvips 1m

1m-install:
#	tar cvf - 1m | (cd $(HTMLDIR) ; tar xvf -)
	cp *.gif $(HTMLDIR)/1m
	cp *.jpg $(HTMLDIR)/1m
	-gzip 1m.ps
	cp 1m.ps.gz $(HTMLDIR)
	
newinst:newinst-ps newinst-html
newinst-html: newinst.tex
	latex2html -dir $(HTMLDIR)/newinst $(L2HFLAGS) -t "NMSU 1m telescope" newinst
newinst-ps: newinst.tex
	latex newinst
	latex newinst
	dvips newinst

newinst-install:
#	tar cvf - newinst | (cd $(HTMLDIR) ; tar xvf -)
	
man:man-ps man-html
man-html: man.tex
	latex2html -dir $(HTMLDIR)/man $(L2HFLAGS)  -split +1 -t "1m manual" man
man-ps: man.tex
	latex man
	latex man
	dvips man

man-install:
#	tar cvf - man | (cd $(HTMLDIR) ; tar xvf -)
	-gzip man.ps
	cp man.ps.gz $(HTMLDIR)
	
robotinst:robotinst-ps robotinst-html
robotinst-html: robotinst.tex
	latex2html -dir $(HTMLDIR)/robotinst $(L2HFLAGS) -t "1m robotinst" robotinst
robotinst-ps: robotinst.tex
	latex robotinst
	latex robotinst
	dvips robotinst

robotinst-install:
#	tar cvf - robotinst | (cd $(HTMLDIR) ; tar xvf -)
	
systems:systems-ps systems-html
systems-html: systems.tex
	latex2html -split +1 -dir $(HTMLDIR)/systems $(L2HFLAGS) -t "1m systems" systems
systems-ps: systems.tex
	latex systems
	latex systems
	dvips systems
systems-install:
#	tar cvf - systems | (cd $(HTMLDIR) ; tar xvf -)
	-gzip -f systems.ps
	cp systems.ps.gz $(HTMLDIR)
	
tertiary:tertiary-ps tertiary-html
tertiary-html: tertiary.tex
	latex2html -split 0 -dir $(HTMLDIR)/tertiary $(L2HFLAGS) -t "1m tertiary" tertiary
tertiary-ps: tertiary.tex
	latex tertiary
	latex tertiary
	dvips tertiary
tertiary-install:
#	tar cvf - tertiary | (cd $(HTMLDIR) ; tar xvf -)
	-gzip tertiary.ps
	cp tertiary.ps.gz $(HTMLDIR)
	
site:site-ps site-html
site-html: site.tex
	latex2html -split 0 -dir $(HTMLDIR)/site $(L2HFLAGS) -t "1m site" site
site-ps: site.tex
	latex site
	latex site
	dvips site

site-install:
	tar cvf - site | (cd $(HTMLDIR) ; tar xvf -)
	
slitview:slitview-ps slitview-html
slitview-html: slitview.tex
	latex2html -dir $(HTMLDIR)/slitview -split 0 $(L2HFLAGS) -t "slitview" slitview
slitview-ps: slitview.tex
	latex slitview
	latex slitview
	dvips slitview
	ps2pdf slitview.ps slitview.pdf
slitview-install:
#	tar cvf - slitview | (cd $(HTMLDIR) ; tar xvf -)
	-gzip slitview.ps
	cp slitview.ps.gz $(HTMLDIR)
	cp slitview.pdf $(HTMLDIR)
	
throughput:throughput-ps throughput-html
throughput-html: throughput.tex
	latex2html -dir $(HTMLDIR)/throughput -split 0 $(L2HFLAGS) -t "throughput" throughput
throughput-ps: throughput.tex
	latex throughput
	latex throughput
	dvips throughput
	ps2pdf throughput.ps throughput.pdf
throughput-install:
#	tar cvf - throughput | (cd $(HTMLDIR) ; tar xvf -)
	-gzip throughput.ps
	cp throughput.rmk $(HTMLDIR)
	cp throughput.ps.gz $(HTMLDIR)
	cp throughput.pdf $(HTMLDIR)
	
rivmos:rivmos-ps rivmos-html
rivmos-html: rivmos.tex
	latex2html -dir $(HTMLDIR)/rivmos -split 0 $(L2HFLAGS) -t "rivmos" rivmos
rivmos-ps: rivmos.tex
	latex rivmos
	latex rivmos
	dvips rivmos
	ps2pdf rivmos.ps rivmos.pdf
rivmos-install:
#	tar cvf - rivmos | (cd $(HTMLDIR) ; tar xvf -)
	-gzip rivmos.ps
	cp rivmos.ps.gz $(HTMLDIR)
	cp rivmos.pdf $(HTMLDIR)
	cp rivmos.html $(HTMLDIR)
	cp OpticImage.jpg $(HTMLDIR)
	
btc:btc-ps btc-html
btc-html: btc.tex
	latex2html -dir $(HTMLDIR)/btc -split 0 $(L2HFLAGS) -t "btc" btc
btc-ps: btc.tex
	latex btc
	latex btc
	dvips btc
	ps2pdf btc.ps btc.pdf
btc-install:
#	tar cvf - btc | (cd $(HTMLDIR) ; tar xvf -)
	-gzip btc.ps
	cp btc.ps.gz $(HTMLDIR)
	cp btc.pdf $(HTMLDIR)
	
mar01:mar01-ps mar01-html
mar01-html: mar01.tex
	latex2html -dir $(HTMLDIR)/mar01 -split 0 $(L2HFLAGS) -t "mar01" mar01
mar01-ps: mar01.tex
	latex mar01
	latex mar01
	dvips mar01
	ps2pdf mar01.ps mar01.pdf
mar01-install:
#	tar cvf - mar01 | (cd $(HTMLDIR) ; tar xvf -)
	-gzip mar01.ps
	cp mar01.ps.gz $(HTMLDIR)
	cp mar01.pdf $(HTMLDIR)
	cp t.jpg $(HTMLDIR)
	cp oldt.jpg $(HTMLDIR)
	
install:1m-install newinst-install man-install systems-install tertiary-install

clean:
	-rm *.dvi *.aux *.log *.idx *.ps.gz *.pdf.gz *.ps *.pdf *.toc
backup:
	cp -a *.tex *.html /home/avalon2/holtz/apo/1m
