# AppImageQt5run

To run `usr/bin/MyQt5Executable` 
```bash
cd AppImage.AppDir
cp /some/path/appimage.qt5run usr/bin/MyQt5Executable.qt5run
ln -s usr/bin/MyQt5Executable.qt5run AppRun
```

This is c++ wrapper linked with Qt 5.1.1.
Wrapper make call to  `QCoreApplication::libraryPaths()` function and return system path.
Executable must be compiled without RPATH/RUNPATH
