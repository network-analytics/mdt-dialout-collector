MAINTAINERCLEANFILES = \
	$(top_srcdir)/aclocal.m4 \
	$(top_srcdir)/configure \
	$(top_srcdir)/Makefile.in

SUBDIRS = proto src

#Could be improved..
.PHONY: doc

install-exec-hook:

uninstall-hook:

maintainer-clean-local:
	rm -rf $(top_srcdir)/m4
	rm -rf $(top_srcdir)/build-aux
