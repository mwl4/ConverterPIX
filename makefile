all:
	make -C src/libs/fmt
	make -C src
clean:
	make -C src/libs/fmt clean
	make -C src clean