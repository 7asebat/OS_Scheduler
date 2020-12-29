CC?=gcc
PROCESS_COUNT?=8
ALGORITHM?=3

build:
	mkdir -p bin input logs
	rm -rf logs/process_*.txt
	$(CC) src/process_generator.c -o bin/process_generator.out
	$(CC) src/clk.c -o bin/clk.out
	$(CC) src/scheduler.c -o bin/scheduler.out
	$(CC) src/process.c -o bin/process.out
	$(CC) src/test_generator.c -o bin/test_generator.out

clean:
	rm -f logs/*.log
	rm -f bin/*.out  input/processes.txt 

all: clean build generate run

generate:
	echo $(PROCESS_COUNT) | bin/test_generator.out

run:
	echo $(ALGORITHM) | bin/process_generator.out
