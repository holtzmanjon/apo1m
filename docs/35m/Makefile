L2HFLAGS=-local_icons -info "0" -no_navigation
HTMLDIR=/loki/httpd/html/3.5m

all: slitview procedure rivmos woodgate btc mar01

install:slitview-install procedure-install rivmos-install btc-install mar01-install woodgate-install

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
	
procedure:procedure-ps procedure-html
procedure-html: procedure.tex
	latex2html -dir $(HTMLDIR)/procedure -split 0 $(L2HFLAGS) -t "procedure" procedure
procedure-ps: procedure.tex
	latex procedure
	latex procedure
	dvips procedure
	ps2pdf procedure.ps procedure.pdf
procedure-install:
#	tar cvf - procedure | (cd $(HTMLDIR)/throughput ; tar xvf -)
	-gzip procedure.ps
	cp throughput.rmk $(HTMLDIR)/throughput
	cp procedure.ps.gz $(HTMLDIR)/throughput
	cp procedure.pdf $(HTMLDIR)/throughput
	
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

woodgate:woodgate-ps woodgate-html
woodgate-html: woodgate.tex
	latex2html -dir $(HTMLDIR)/woodgate -split 0 $(L2HFLAGS) -t "woodgate" woodgate
woodgate-ps: woodgate.tex
	latex woodgate
	latex woodgate
	dvips woodgate
	ps2pdf woodgate.ps woodgate.pdf
woodgate-install:
#	tar cvf - woodgate | (cd $(HTMLDIR) ; tar xvf -)
	-gzip woodgate.ps
	cp woodgate.ps.gz $(HTMLDIR)
	cp woodgate.pdf $(HTMLDIR)
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
#	tar cvf - mar01 | (cd $(HTMLDIR)/throughput ; tar xvf -)
	-gzip mar01.ps
	cp mar01.ps.gz $(HTMLDIR)/throughput
	cp mar01.pdf $(HTMLDIR)/throughput
	cp t.jpg $(HTMLDIR)/throughput
	cp oldt.jpg $(HTMLDIR)/throughput
	
clean:
	-rm *.dvi *.aux *.log *.idx *.ps *.pdf *.ps.gz *.pdf.gz *.toc
backup:
	cp -a *.tex *.html /home/avalon2/holtz/apo/1m
