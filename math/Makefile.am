AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++17 -DFORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/node-blue/nodes/math

lib_LTLIBRARIES = scale.la

scale_la_SOURCES = scale/Factory.cpp scale/MyNode.cpp
scale_la_LDFLAGS = -module -avoid-version -shared

math_ladir = $(libdir)
math_la_DATA = package.json scale/scale.hni

locale_endir = $(libdir)/locales/en
locale_en_DATA = locales/en-US/math.json

locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = locales/en-US/math.json

locale_dedir = $(libdir)/locales/de
locale_de_DATA = locales/de-DE/math.json

locale_de_dedir = $(libdir)/locales/de-DE
locale_de_de_DATA = locales/de-DE/math.json

install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/scale.la
