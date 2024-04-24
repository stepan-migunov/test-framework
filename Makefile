QMAKE=qmake6#/home/stepan/Qt/6.2.4/gcc_64/bin/qmake

all: clean framework viewer

framework:
	mkdir build-framework && cd build-framework && $(QMAKE) ../mime_dbus_framework && make

viewer:
	mkdir build-viewer && cd build-viewer && $(QMAKE) ../image_viewer && make
	
clean:
	rm -rf build-framework build-viewer
