COMPILER?=gcc
PROCESS_COUNT?=8
ALGORITHM?=3

build:
	mkdir -p bin input logs
	$(COMPILER) src/process_generator.c -o bin/process_generator.out
	$(COMPILER) src/clk.c -o bin/clk.out
	$(COMPILER) src/scheduler.c -lm -o bin/scheduler.out
	$(COMPILER) src/process.c -o bin/process.out
	$(COMPILER) src/test_generator.c -o bin/test_generator.out

clean:
	rm -rf logs/*.log
	rm -rf bin/*.out  input/processes.txt
	rm -rf debug

all: clean build generate run

generate:
	echo $(PROCESS_COUNT) | bin/test_generator.out

run:
	bin/process_generator.out

debug-build:
	mkdir -p debug
	$(COMPILER) -DDEBUG_SCHEDULER src/process_generator.c -g -o debug/process_generator.out
	$(COMPILER) -DDEBUG_SCHEDULER src/clk.c -g -o debug/clk.out
	$(COMPILER) -DDEBUG_SCHEDULER src/scheduler.c -g -lm -o debug/scheduler.out
	$(COMPILER) -DDEBUG_SCHEDULER src/process.c -g -o debug/process.out
	$(COMPILER) -DDEBUG_SCHEDULER src/test_generator.c -g -o debug/test_generator.out