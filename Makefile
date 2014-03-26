BIN=./bin/
BUILD=./obj/
AUTO=./auto/
SRCD=./src/
TESTD=./tests/
LIBD=./lib/


ASRC = $(wildcard $(SRCD)*.l) $(wildcard $(SRCD)*.y)

SRC = $(wildcard $(SRCD)*.c) $(addsuffix .c , $(addprefix $(AUTO), $(basename $(notdir $(ASRC)))))  

CSRC = $(notdir  $(filter $(SRCD)%, $(SRC)))

OBJ = $(addsuffix .o, $(notdir $(basename $(SRC))))	


TESTSRC = $(wildcard $(TESTD)*.c) $(wildcard $(LIBD)seatest/*.c)

TESTS =  $(notdir $(TESTSRC))

TESTOBJ = $(addsuffix .o,$(basename $(TESTS))) 

LIBPATHS = $(addprefix -I, $(wildcard $(LIBD)*))



CC = gcc -Wall


#all: piz liz wiz

wiz: $(OBJ) | $(BIN)
	$(CC) -o $(BIN)$@ $(addprefix $(BUILD), $(OBJ))

run_tests: $(TESTOBJ) | $(BIN)
	$(CC) -o $(BIN)$@ $(addprefix $(BUILD), $^)
	$(BIN)$@


piz: piz.o $(OBJ) | $(BIN)
	$(CC) -o $(BIN)$@ $(addprefix $(BUILD), $^)


liz: piz.o liz.o $(OBJ)  | $(BIN)
	 $(CC) -o $(BIN)$@ $(addprefix $(BUILD), $^)

piz.c piz.h:  | $(AUTO)
	bison --debug -v -d $(SRCD)piz.y -o $(AUTO)piz.c

liz.c liz.h: piz.h  | $(AUTO)
	flex -s -o$(AUTO)liz.c $(SRCD)liz.l

$(CSRC) $(TESTS):

clean:
	/bin/rm -rf $(BUILD) $(AUTO) $(BIN)

$(OBJ) $(TESTOBJ): %.o  :  %.c | $(BUILD)
	gcc  -c $(filter  %$(basename $(@F)).c, $(SRC) $(TESTSRC)) $(LIBPATHS)  -I$(AUTO) -I$(SRCD) -o $(BUILD)$@


print:
	@echo build		: $(OBJ)
	@echo sources 	: $(SRC)
	@echo other lang files	: $(ASRC)
	@echo c sources : $(CSRC)
	@echo Test sources : $(TESTSRC)
	@echo Test Obj: $(TESTOBJ)
	@echo LibPaths : $(LIBPATHS) 

$(AUTO) $(BUILD) $(BIN):
	mkdir -p $@
