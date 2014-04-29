HDR =	wiz.h piz.h ast.h pretty.h std.h missing.h

OBJ =	wiz.o piz.o liz.o ast.o pretty.o

CC = 	gcc -Wall

wiz: $(OBJ)
	$(CC) -o wiz $(OBJ)

piz.c piz.h: piz.y ast.h std.h missing.h
	bison --debug -v -d piz.y -o piz.c

liz.c: liz.l piz.h std.h ast.h
	flex -s -oliz.c liz.l

clean:
	/bin/rm $(OBJ) piz.c piz.h piz.output liz.c

submit:
	submit 90045 1b wiz.h ast.h pretty.h std.h missing.h\
	 	Makefile ast.c liz.l piz.y pretty.c wiz.c

$(OBJ):	$(HDR)
