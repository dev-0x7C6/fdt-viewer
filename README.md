# FDT Viewer
Flattened Device Tree Viewer written in Qt.

![](https://github.com/dev-0x7C6/fdt-viewer/workflows/Continuous%20integration/badge.svg)

#### Supported files
* \*.dtb - devicetree blob
* \*.dtbo - devicetree overlay blob
* \*.itb - fit image container

#### Features
* Quick search for single or multiple device-trees
* Show embedded inner device-tree data

#### Command line usage
```
Usage: ./fdt-viewer [options]

Options:
  -h, --help                   Displays help on commandline options.
  --help-all                   Displays help including Qt specific options.
  -v, --version                Displays version information.
  -f, --file <file>            open file.
  -d, --directory <directory>  open directory.
```

#### Installation
```console
user@host # git clone --recursive https://github.com/dev-0x7C6/fdt-viewer.git
user@host # cd fdt-viewer
user@host # cmake . -DCMAKE_INSTALL_PREFIX=/usr
user@host # make -j$(nproc)
root@host # make install
```

#### Packaging with Docker
Create a Debian package of ftd-viewer in a Docker container and install it to the host system:
```console
user@host # git clone --recursive https://github.com/dev-0x7C6/fdt-viewer.git
user@host # cd fdt-viewer
user@host # cmake .
user@host # make docker
root@host # dpkg -i "fdt-viewer*.deb"
```

#### Known bugs
- presentation of data-types is under development

![image](https://devwork.space/wp-content/uploads/2020/12/fdt_viewer_v050.png)
![image](https://devwork.space/wp-content/uploads/2020/12/fdt_viewer_v041.png)
