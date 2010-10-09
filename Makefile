.PHONY: all clean

all:
	python ./build.py random-colors.cpp

clean:
	rm -rf out/
	rm -rf *.pyc
