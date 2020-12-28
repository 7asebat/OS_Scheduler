PROCESS_COUNT?=8

build:
	mkdir -p bin input logs
	rm -rf logs/process_*.txt
	gcc src/process_generator.c -o bin/process_generator.out
	gcc src/clk.c -o bin/clk.out
	gcc src/scheduler.c -o bin/scheduler.out
	gcc src/process.c -o bin/process.out
	gcc src/test_generator.c -o bin/test_generator.out

clean:
	rm -f bin/*.out  input/processes.txt

all: clean build generate run

generate:
	echo $(PROCESS_COUNT) | bin/test_generator.out 

run:
	bin/process_generator.out
