all: dlopen

#CFLAGS=-fno-gnu-unique
CFLAGS=-g

mylib.o: mylib.c
	gcc -fPIC $(CFLAGS) -c mylib.c -o mylib.o

mylib.so: mylib.o
	gcc -shared $(CFLAGS) mylib.o -o mylib.so

dlopen: mylib.so dlopen.c
	gcc dlopen.c $(CFLAGS) -Lmylib -o dlopen -ldl

panda_dlopen: panda_dlopen.c
	gcc panda_dlopen.c $(CFLAGS) -Lmylib -o panda_dlopen -ldl

clean:
	rm -f mylib.o mylib.so dlopen
