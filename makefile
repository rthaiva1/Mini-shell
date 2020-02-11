all: assignment2

run: assignment2
	./assignment2

assignment1:
	gcc -o assignment2 assignment2.c

clean :
	rm -rf assignment2
