all: editbox_test

editbox_test: editbox.o editbox_test.o
	gcc editbox.o editbox_test.o -o editbox_test
	rm -f editbox.o editbox_test.o

editbox.o: editbox.c editbox.h
	gcc -c editbox.c -o editbox.o

editbox_test.o: editbox_test.c
	gcc -c editbox_test.c -o editbox_test.o

clean:
	rm -f editbox_test.exe