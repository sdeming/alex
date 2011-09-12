
##
## Alex (Allegro Extras)
## =====================
##
## Requires Allegro by Shawn Hargreaves, and DJGPP by DJ Delorie.
##
##############################################################################
##
## ARG!  Now why does it insist on compiling ALL of the example files every
## time?  Where are their .o files located?  Should be in the obj dir, but
## they don't seem to be there...  I'm overlooking something VERY simple,
## I must be.
##
##############################################################################
##
## Scott Deming
## sdeming@concentric.net
##
##############################################################################

## External programs
RM=del
CP=copy

## DJGPP v2.00
CC=gcc
CFLAGS=-m486 -Wall -Winline $(OPTFLAGS) $(DEFS)
OPTFLAGS=-O2 -fomit-frame-pointer -funroll-all-loops
#OPTFLAGS=-pg
DEFS=

OBJ=obj
EXE=.exe

OBJS=fade.o match.o gui.o display.o sptypes.o tile.o map.o sprite.o


# Library
AR=ar
LIBS=-lm -lpc -lalleg -lalex
LIBOBJ=$(addprefix $(OBJ)/, $(OBJS))
LIB=libalex.a
INC=alex.h
LIBDEST=$(DJDIR)/lib/$(LIB)
INCDEST=$(DJDIR)/include/$(INC)
CPLIBDEST=$(subst /,\,$(LIBDEST))
CPINCDEST=$(subst /,\,$(INCDEST))

# Rules
default: all

all: $(LIB) install examples

$(LIB): $(LIBOBJ)
	$(AR) rs $(LIB) $(LIBOBJ)

install: $(LIBDEST) $(INCDEST)

examples: examples/ex1$(EXE) examples/ex2$(EXE) examples/ex3$(EXE) \
	examples/ex4$(EXE) examples/ex5$(EXE) examples/ex6$(EXE)
	@echo "*** Examples built."

$(LIBDEST): $(LIB)
	$(CP) $(LIB) $(CPLIBDEST)
	@echo "*** Installed $(LIB) to: $(LIBDEST)"

$(INCDEST): $(INC)
	$(CP) $(INC) $(CPINCDEST)
	@echo "*** Installed $(INC) to: $(INCDEST)"

$(OBJ)/%.o: src/%.c $(INCDEST)
	$(CC) -c $(CFLAGS) -o $@ -c $<

$(OBJ)/%.o: examples/%.c $(INCDEST)
	$(CC) -c $(CFLAGS) -o $@ -c $<

#*/%$(EXE): $(OBJ)/%.o $(LIB)
#	 $(CC) $(CFLAGS) -o $@ $< $(LIBS)

examples/%$(EXE): $(OBJ)/%.o $(LIB)
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

##test: examples
test: examples/ex4$(EXE)
