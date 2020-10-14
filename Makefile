all: run main.cc
run: main.cc
	g++ -o run main.cc -lpthread
clean:
	rm run
