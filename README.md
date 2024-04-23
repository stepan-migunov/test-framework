Run under QtCreator, otherwise it could not load necessary enironment variables. 
Project "mime_dbus_framework" is required in image_viewer.pro. 
Therefore, you need to check if LIBS and INCLUDEPATH are referring to appropriate directories.
Possble launching pattern is:
1. Clone repo
2. Build mime_dbus_framework
3. Satisfy dependencies by modifying LIBS, INCLUDEPATH and DEPENDPATH
4. Build image_viewer
5. Run it in Debug mode
