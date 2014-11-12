INSTALL_TOP = /usr/local
INSTALL_BIN = $(INSTALL_TOP)/bin

MKDIR = mkdir -p -m 0755
RMDIR = rmdir -p --ignore-fail-on-non-empty

INSTALL = install -p
INSTALL_EXEC = $(INSTALL) -m 0755
INSTALL_DATA = $(INSTALL) -m 0644

TO_BIN = gpx2garmin kml2garmin

all clean:
	cd src && $(MAKE) $@

install:
	cd src && $(MKDIR) $(INSTALL_BIN)
	cd src && $(INSTALL_EXEC) $(TO_BIN) $(INSTALL_BIN)

uninstall:
	cd src && cd $(INSTALL_BIN) && $(RM) $(TO_BIN)
	cd src && $(RMDIR) $(INSTALL_BIN)

.PHONY: all clean install uninstall
