BIN=./bin/
BUILD=./obj/
AUTO=./auto/
SRCD=./src/


ASRC = $(wildcard $(SRCD)*.l) $(wildcard $(SRCD)*.y)

SRC = $(wildcard $(SRCD)*.c) $(addsuffix .c , $(addprefix $(AUTO), $(basename $(notdir $(ASRC))))) 

CSRC = $(notdir  $(filter $(SRCD)%, $(SRC)))

OBJ = $(addsuffix .o, $(notdir $(basename $(SRC))))	

CC = gcc -Wall


#all: piz liz wiz
wiz: $(OBJ) | $(BIN)
	@$(CC) -o $(BIN)$@ $(addprefix $(BUILD), $(OBJ))


piz: piz.o $(OBJ) | $(BIN)
	@$(CC) -o $(BIN)$@ $(addprefix $(BUILD), $^)


liz: piz.o liz.o $(OBJ)  | $(BIN)
	@ $(CC) -o $(BIN)$@ $(addprefix $(BUILD), $^)

piz.c piz.h:  | $(AUTO)
	@bison --debug -v -d $(SRCD)piz.y -o $(AUTO)piz.c

liz.c liz.h: piz.h  | $(AUTO)
	@flex -s -o$(AUTO)liz.c $(SRCD)liz.l

$(CSRC) :

clean:
	/bin/rm -rf $(BUILD) $(AUTO) $(BIN)

$(OBJ): %.o  :  %.c | $(BUILD)
	@echo Analysing $(basename $(@F)).c!
	@gcc  --analyze $(filter  %$(basename $(@F)).c, $(SRC))  -I$(AUTO) -I$(SRCD) -o $(BUILD)$@
	@gcc  -c $(filter  %$(basename $(@F)).c, $(SRC))  -I$(AUTO) -I$(SRCD) -o $(BUILD)$@
	@echo Finished $(basename $(@F)).c!



print:
	@echo build		: $(OBJ)
	@echo sources 	: $(SRC)
	@echo other lang files	: $(ASRC)
	@echo c sources : $(CSRC)

$(AUTO) $(BUILD) $(BIN):
	mkdir -p $@