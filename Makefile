.PHONY: all clean

all:
	python ./build.py projects/led-matrix/random-colors.cpp

clean:
	rm -rf .out/
	rm -rf *.pyc
