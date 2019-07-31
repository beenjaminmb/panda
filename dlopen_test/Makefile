all: dlopen

mylib.o: mylib.c
	gcc -fPIC -c mylib.c -o mylib.o

mylib.so: mylib.o
	gcc -shared mylib.o -o mylib.so

dlopen: mylib.so dlopen.c
	gcc dlopen.c -Lmylib -o dlopen -ldl

clean:
	rm -f mylib.o mylib.so dlopen
