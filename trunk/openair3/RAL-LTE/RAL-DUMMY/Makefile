# lines starting with the pound sign are comments.
#
# These things are options that you might need
# to tweak.


# You can modify the below as well, but probably
# won't need to.
INTERFACE_DIR=../INTERFACE-802.21
CORE_DIR=SRC/CORE
MIH_LINK_DIR=SRC/MIH_LINK

CC = gcc

CFLAGS = -Wall -g -ISRC  -I$(INTERFACE_DIR)/INCLUDE -I$(CORE_DIR) -I$(MIH_LINK_DIR) -DMIH_C_MEDIEVAL_EXTENSIONS



MIH_LINK_OBJS = $(MIH_LINK_DIR)/mRALlteInt_mih_msg.o

CORE_OBJS  = $(CORE_DIR)/mRALlteInt_mih_fsm.o
CORE_OBJS  += $(CORE_DIR)/mRALlteInt_thresholds.o
CORE_OBJS  += $(CORE_DIR)/mRALlteInt_parameters.o
CORE_OBJS  += $(CORE_DIR)/mRALlteInt_action.o
CORE_OBJS  += $(CORE_DIR)/mRALlteInt_get.o


OBJS = SRC/mRALlteInt_main.o   $(INTERFACE_OBJS) $(CORE_OBJS) $(MIH_LINK_OBJS)

EXECUTABLE = mRALlteInt

# "all" is the default target. Simply make it point to myprogram.

all: $(EXECUTABLE)

mRALlteInt : $(OBJS)
	$(CC) -o mRALlteInt $(OBJS) -lrt -L$(INTERFACE_DIR)/LIB -lmih_c-802.21


$(OBJS): %.o : %.c
	@echo Compiling $<
	@$(CC) -c $(CFLAGS) $(EXTRA_CFLAGS)  -o $@ $<

%.o: %.c
	$(COMPILE) -o $@ $<

clean:
	-find . -name "*.o" -delete
	-find . -name "*.*~" -delete
	-find . -name "*~" -delete



