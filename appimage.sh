#!/bin/bash
mkdir -p build
rm -r build/*
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr .. && make && make install DESTDIR=appdir
cp ../res/qtplayer.desktop appdir/
cp ../res/qtplayer.svg appdir/
wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
chmod +x linuxdeploy-x86_64.AppImage
chmod +x linuxdeploy-plugin-qt-x86_64.AppImage
./linuxdeploy-x86_64.AppImage --output appimage --plugin qt --appdir appdir
