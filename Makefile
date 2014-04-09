BIN=./bin/
BUILD=./obj/
AUTO=./auto/
SRCD=./src/
TESTD=./tests/
LIBD=./lib/
SUBD=./tmp/
CURD=./


ASRC = $(wildcard $(SRCD)*.l) $(wildcard $(SRCD)*.y)

SRC = $(wildcard $(SRCD)*.c) $(addsuffix .c , $(addprefix $(AUTO), $(basename $(notdir $(ASRC)))))  

CSRC = $(notdir  $(filter $(SRCD)%, $(SRC)))

OBJ = $(addsuffix .o, $(notdir $(basename $(SRC))))	


TESTSRC = $(wildcard $(TESTD)*.c) $(wildcard $(LIBD)seatest/*.c)

TESTS =  $(notdir $(TESTSRC))

TESTOBJ = $(addsuffix .o,$(basename $(TESTS))) 

LIBPATHS = $(addprefix -I, $(wildcard $(LIBD)*))

TARFILE=source.tar.gz

CC = gcc -Wall


#all: piz liz wiz
wizuni : extract
	make -C $(CURD)  wiz
	cp $(BIN)wiz ./

wiz: $(OBJ) | $(BIN)
	$(CC) -o $(BIN)$@ $(addprefix $(BUILD), $(OBJ))

wizdb :  $(addsuffix .c, $(notdir $(basename $(SRC))))	| $(BIN)
	$(CC) -g -o $(BIN)$@ $(SRC)  $(LIBPATHS)  -I$(AUTO) -I$(SRCD) 


run_tests: $(TESTOBJ) $(filter-out wiz.%, $(OBJ)) | $(BIN)
	$(CC) -o $(BIN)$@ $(addprefix $(BUILD), $^)
	$(BIN)$@


piz: piz.o $(OBJ) | $(BIN)
	$(CC) -o $(BIN)$@ $(addprefix $(BUILD), $^)


liz: piz.o liz.o $(OBJ)  | $(BIN)
	 $(CC) -o $(BIN)$@ $(addprefix $(BUILD), $^)

piz.c piz.h:  | $(AUTO)
	bison --debug -v -d $(SRCD)piz.y -o $(AUTO)piz.c

liz.c liz.h: piz.h  | $(AUTO)
	flex -s -o$(AUTO)liz.c --header-file=$(AUTO)liz.h $(SRCD)liz.l

$(CSRC) $(TESTS):

clean:
	/bin/rm -rf $(BUILD) $(AUTO) $(BIN) $(SUBD) wiz

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

$(SUBD) $(AUTO) $(BUILD) $(BIN):
	mkdir -p $@

submit : | $(SUBD)
	tar cfvz $(SUBD)$(TARFILE) $(SRCD) $(LIBD) $(TESTD) 
	cp Makefile $(SUBD)
	@echo *****The files to submit are in $(SUBD)

extract : 
ifeq ($(wildcard $(SRCD)),) 
		tar xfvz $(TARFILE) 
endif

