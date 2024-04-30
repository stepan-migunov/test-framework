1. #Install necessary packages:
  sudo apt update
  sudo apt install make g++ qt6-base-dev qml-qt6 qt6-declarative-dev qt6-wayland qml6-module-qtqml-workerscript qml6-module-qtquick qml6-module-qtquick-controls qml6-module-qtquick-layouts qml6-module-qtquick-templates
2. cd ~
3. git clone https://github.com/stepan-migunov/test-framework
4. cd ./test-framework
5. make
6. ./build-viewer/image_viewer
