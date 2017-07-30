all:
	make -C src/libs/fmt
	make -C src/libs/cityhash
	make -C src/libs/zlib
	make -C src
clean:
	make -C src/libs/fmt clean
	make -C src/libs/cityhash clean
	make -C src/libs/zlib clean
	make -C src clean