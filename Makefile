all: run futex_test
futex_test: futex_test.cc
	g++ -o futex_test futex_test.cc lock.o -lpthread
run: main.o lock.o
	g++ -o run main.o lock.o -lpthread
main: main.cc
	g++ -c main.cc -lpthread
lock: lock.cc
	g++ -c lock.cc -lpthread
clean:
	rm run futex_test *.o
