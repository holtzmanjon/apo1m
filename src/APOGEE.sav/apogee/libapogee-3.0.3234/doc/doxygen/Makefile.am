docpkg = $(PACKAGE_TARNAME)-doxy-$(PACKAGE_VERSION).tar.gz

doc_DATA = $(docpkg)

#putting the png into the distribution
EXTRA_DIST = ExceptionHandling_small.png

$(docpkg): doxygen.stamp
	tar chof - html | gzip -9 -c >$@
    
doxygen.stamp: zenith 
	$(DOXYGEN) $(DOXYFLAGS) $<
	echo Timestamp > $@
    
install-data-hook:
	cd $(DESTDIR)$(docdir) && tar xf $(docpkg)
   
uninstall-hook:
	cd $(DESTDIR)$(docdir) && rm -rf html
    
CLEANFILES = doxygen.stamp $(docpkg)

clean-local:
	rm -rf html