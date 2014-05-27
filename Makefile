HDR =	wiz.h piz.h ast.h oztree.h pretty.h std.h missing.h helper.h bbst.h\
        symbol.h analyse.h codegen.h error_printer.h wizoptimiser.h array_access.h

OBJ =	wiz.o piz.o liz.o ast.o pretty.o helper.o bbst.o symbol.o analyse.o\
        codegen.o oztree.o error_printer.o wizoptimiser.o array_access.o

CC = 	gcc -Wall -Wextra

wiz: $(OBJ)
	$(CC) -o wiz $(OBJ)

piz.c piz.h: piz.y ast.h std.h missing.h helper.h
	bison --debug -v -d piz.y -o piz.c

liz.c: liz.l piz.h std.h ast.h
	flex -s -oliz.c liz.l

clean:
	/bin/rm $(OBJ) piz.c piz.h piz.output liz.c

submit:
	submit 90045 3b wiz.h ast.h pretty.h std.h missing.h helper.h\
	 	Makefile ast.c liz.l piz.y pretty.c wiz.c helper.c README\
	 	array_access.h array_access.c analyse.c analyse.h bbst.c bbst.h\
	 	codegen.c codegen.h error_printer.c error_printer.h oztree.c\
	 	oztree.h symbol.c symbol.h wizoptimiser.c wizoptimiser.h

$(OBJ):	$(HDR)
