# FDT Viewer
Flattened Device Tree Viewer written in Qt.

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

#### Installation process
```console
user@host # git clone https://github.com/dev-0x7C6/fdt-viewer.git
user@host # cd fdt-viewer
user@host # cmake . -DCMAKE_INSTALL_PREFIX=/usr
user@host # make -j$(nproc)
root@host # make install
```

#### Known bugs
- presentation of data-types is under development

![image](https://devwork.space/wp-content/uploads/2020/12/fdt_viewer_v041.png)
