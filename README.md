# Qt Player

Simple libmpv wrapper. Built to easily make a playlist with videos/podcasts from different providers and change playback speed.

Based on https://github.com/mpv-player/mpv-examples/tree/master/libmpv/qt_opengl


- Drag link or file to program window to add to playlist.
- Playlist is visible when windowed, hidden on Fullscreen.
- Control bar is always visible.
- Changing video resolution restarts the video and only works for Youtube videos.
- `Space` to toggle Play/Pause.
- `Esc` exit Fullscreen.
- `Double click` on video to toggle Fullscreen.
- `Click` on video to toggle Play/Pause.


### Screenshot:
![Image alt text](screenshot/screenshot2.png)

### Dependencies:
#### Ubuntu
`sudo apt install git cmake g++ pkg-config qtbase5-dev libqt5widgets5 libmpv-dev`

#### Arch
`sudo pacman -S git base-devel cmake pkgconf qt5-base qt5-svg mpv youtube-dl`

### Install:
```
git clone https://github.com/slidinghotdog/qtplayer.git
cd qtplayer
mkdir -p build && cd build
cmake .. && make && sudo make install
```