# Crosshair Overlay
Tired of a simple overlay taking over 100MB of RAM for no reason?
A very light, fast and portable crosshair overlay built with C++.

### On development!
The current version is as minimal as it can get althought it would be nice to add some more optimizations to it. Later on there will be a less minimal version with an included json file to modify the crosshair and maybe even a complete UI.

### Crosshair Customization
For now, if you want the best possible performance, update the source code and compile it yourself (it's easy!)

You will need Cmake (at least 3.10)
```bash
git clone https://github.com/AlmartDev/SimpleCrosshair.git
cd SimpleCrosshair
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config MinSizeRel
```

### CPU/RAM usage
- Executable Size: **198KB**.
- CPU Usage: **0%** (YES really)
- RAM Usage: **9.1 MB**

### How to quit the app?
Windows system tray (or notification area) -> Select CrosshairOverlay -> Exit
