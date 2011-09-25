.PHONY: all clean

all:
	python ./build.py projects/recorder/recorder.cpp

clean:
	rm -rf .out/
	rm -rf *.pyc
