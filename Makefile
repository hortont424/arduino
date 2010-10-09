.PHONY: all clean

all:
	python ./build.py

clean:
	rm -rf *.o
	rm -rf *.pyc
	rm -rf *.elf
