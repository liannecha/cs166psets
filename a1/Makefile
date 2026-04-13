# Recursively build everything here
TARGET_CPPS := RunTests.cpp TimeTests.cpp
CPP_FILES := $(filter-out $(TARGET_CPPS),$(shell find . -name '*.cpp' | sed "s|^\./||"))
HDR_FILES := $(shell find . -name '*.h')
OBJ_FILES := $(CPP_FILES:.cpp=.o)

CPP_FLAGS = --std=c++17 -Wall -Werror -Wpedantic -O0 -g

all: run-tests time-tests

$(OBJ_FILES) $(TARGET_CPPS:.cpp=.o): Makefile $(HDR_FILES)

run-tests: $(OBJ_FILES) RunTests.o
	g++ -o $@ $^

time-tests: $(OBJ_FILES) TimeTests.o
	g++ -o $@ $^

%.o: %.cpp
	g++ -c $(CPP_FLAGS) -o $@ $<

.PHONY: clean

clean:
	rm -f run-tests time-tests *~ && find . -name '*.o' -delete
