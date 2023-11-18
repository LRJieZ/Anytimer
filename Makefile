obj  = anytimer.c main.c

all :
	gcc -o a.out $(obj)

.PHONY : clean
clean :
	rm a.out