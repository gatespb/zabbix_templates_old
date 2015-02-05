More VDisk stats are now available from the command line

Sources: /usr/src/quadstor
Bin: /quadstor/bin

#---# Build:

mv ./utils /usr/src/quadstor/
cd /usr/src/quadstor/utils
make
make install

/quadstor/bin/qstat -h