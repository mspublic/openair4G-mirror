MEM_DIR = $(OPENAIR2_TOP)/UTIL/MEM
LIST_DIR = $(OPENAIR2_TOP)/UTIL/LISTS
MATH_DIR = $(OPENAIR2_TOP)/UTIL/MATH
TIMER_DIR = $(OPENAIR2_TOP)/UTIL/TIMER
LOG_DIR=$(OPENAIR2_TOP)/UTIL/LOG
OCG_DIR=$(OPENAIR2_TOP)/UTIL/OCG
OPT_DIR=$(OPENAIR2_TOP)/UTIL/OPT
OMG_DIR=$(OPENAIR2_TOP)/UTIL/OMG
CLI_DIR=$(OPENAIR2_TOP)/UTIL/CLI

LIST_OBJ =  $(LIST_DIR)/list.o
LIST_OBJ +=  $(LIST_DIR)/list2.o

TIMER_OBJ +=  $(TIMER_DIR)/umts_timer.o

MEM_OBJ +=  $(MEM_DIR)/mem_block.o

LOG_OBJS =  $(LOG_DIR)/log.o

OCG_OBJS  =  $(OCG_DIR)/OCG.o
OCG_OBJS +=  $(OCG_DIR)/OCG_create_dir.o
OCG_OBJS +=  $(OCG_DIR)/OCG_detect_file.o
OCG_OBJS +=  $(OCG_DIR)/OCG_generate_report.o
OCG_OBJS +=  $(OCG_DIR)/OCG_parse_filename.o
OCG_OBJS +=  $(OCG_DIR)/OCG_parse_XML.o
OCG_OBJS +=  $(OCG_DIR)/OCG_save_XML.o

OPT_OBJS =  $(OPT_DIR)/probe.o

OMG_OBJS =  $(OMG_DIR)/omg.o
OMG_OBJS +=  $(OMG_DIR)/common.o
OMG_OBJS +=  $(OMG_DIR)/job.o
OMG_OBJS +=  $(OMG_DIR)/static.o
OMG_OBJS +=  $(OMG_DIR)/rwp.o
OMG_OBJS +=  $(OMG_DIR)/rwalk.o
OMG_OBJS +=  $(OMG_DIR)/hashtable.o
OMG_OBJS +=  $(OMG_DIR)/mobility_parser.o
OMG_OBJS +=  $(OMG_DIR)/trace.o

CLI_OBJ  =  $(CLI_DIR)/cli_server.o
CLI_OBJ  +=  $(CLI_DIR)/cli.o
CLI_OBJ  +=  $(CLI_DIR)/cli_cmd.o


UTIL_OBJ = $(LIST_OBJ) $(TIMER_OBJ) $(MEM_OBJ) $(LOG_OBJS) $(OCG_OBJS)  $(OMG_OBJS) $(CLI_OBJ) #-I$(OPT_DIR)
CFLAGS += -I$(MEM_DIR) -I$(LIST_DIR) -I$(OCG_DIR) -I$(LOG_DIR) -I$(MATH_DIR) -I$(TIMER_DIR) -I$(OMG_DIR) -I$(CLI_DIR)
UTIL_incl = -I$(MEM_DIR) -I$(LIST_DIR) -I$(OCG_DIR) -I$(LOG_DIR) -I$(MATH_DIR) -I$(TIMER_DIR) -I$(OMG_DIR) -I$(CLI_DIR)
