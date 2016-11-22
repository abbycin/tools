ver = debug

ifeq ($(ver), release)
	FLAG = Release
else ifeq ($(ver), minsize)
	FLAG = MinSizeRel
else
	FLAG = Debug
endif

ifeq ($(JOBS),)
	JOBS := $(shell grep -c ^processor /proc/cpuinfo 2>/dev/null)
	ifeq ($(JOBS),)
		JOBS := 1
	endif
endif

all:
	@if [ ! -d build ]; then mkdir build; fi
	cd build && cmake -DCMAKE_BUILD_TYPE=$(FLAG) .. && cmake --build . -- -j$(JOBS)

clean:
	@rm -rf bin build lib
