AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++11 -DFORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/flows/nodes/template
lib_LTLIBRARIES = template.la
template_la_SOURCES = Factory.cpp MyNode.cpp
template_la_LDFLAGS =-module -avoid-version -shared
template_ladir = $(libdir)
template_la_DATA = template.hni
locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = locales/en-US/template
install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/template.la