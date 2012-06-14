# lines starting with the pound sign are comments.
#
# These things are options that you might need
# to tweak.


# You can modify the below as well, but probably
# won't need to.
INTERFACE_DIR=../INTERFACE-802.21
INCLUDE_DIR=INCLUDE
SRC_DIR=SRC


CC = gcc

CFLAGS = -Wall -g -I$(SRC_DIR)  -I$(INTERFACE_DIR)/INCLUDE  -I$(INCLUDE_DIR) -DMIH_C_MEDIEVAL_EXTENSIONS -DMSCGEN_PYTOOL

SRC_OBJS   = $(SRC_DIR)/mRALlte_mih_msg.o
SRC_OBJS  += $(SRC_DIR)/mRALlte_thresholds.o
SRC_OBJS  += $(SRC_DIR)/mRALlte_parameters.o
SRC_OBJS  += $(SRC_DIR)/mRALlte_action.o
SRC_OBJS  += $(SRC_DIR)/mRALlte_get.o
SRC_OBJS  += $(SRC_DIR)/mRALlte_subscribe.o
SRC_OBJS  += $(SRC_DIR)/mRALlte_main.o
SRC_OBJS  += $(SRC_DIR)/mRALlte_NAS.o
SRC_OBJS  += $(SRC_DIR)/mRALlte_process.o


OBJS =   $(SRC_OBJS)

EXECUTABLE = mRALlteDummy NASUEdummy

# "all" is the default target. Simply make it point to myprogram.

all: $(EXECUTABLE)

mRALlteDummy : $(OBJS)
	$(CC) -o mRALlteDummy $(OBJS) -lrt -L$(INTERFACE_DIR)/LIB -lmih_c-802.21

NASUEdummy : $(SRC_DIR)/nasUEdummy.o
	$(CC) -o NASUEdummy $(SRC_DIR)/nasUEdummy.o

$(OBJS): %.o : %.c
	@echo Compiling $<
	@$(CC) -c $(CFLAGS) $(EXTRA_CFLAGS)  -o $@ $<

%.o: %.c
	$(CC) -c $(CFLAGS) $(EXTRA_CFLAGS)  -o $@ $<

clean:
	-find . -name "*.o" -delete
	-find . -name "*.*~" -delete
	-find . -name "*~" -delete
	-rm -f mRALlteDummy NASUEdummy



