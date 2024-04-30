QMAKE=qmake6#/home/stepan/Qt/6.2.4/gcc_64/bin/qmake

all: clean framework viewer files

framework:
	mkdir build-framework && cd build-framework && $(QMAKE) ../mime_dbus_framework && make staticlib

viewer:
	mkdir build-viewer && cd build-viewer && $(QMAKE) ../image_viewer && make

clean:
	rm -rf build-framework build-viewer
files: 
	if ! ls ${HOME}/.local 1>/dev/null 2>&1; then mkdir ${HOME}/.local; fi;
	if ! ls ${HOME}/.local/share 1>/dev/null 2>&1; then mkdir ${HOME}/.local/share; fi;
	if ! ls ${HOME}/.local/share/applications 1>/dev/null 2>&1; then mkdir ${HOME}/.local/share/applications; fi;
	if ! ls ${HOME}/.local/share/dbus-1 1>/dev/null 2>&1; then mkdir ${HOME}/.local/share/dbus-1; fi;
	if ! ls ${HOME}/.local/share/dbus-1/services 1>/dev/null 2>&1; then mkdir ${HOME}/.local/share/dbus-1/services; fi;
