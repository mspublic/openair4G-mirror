ifeq ($(cygwin),0) 
ETHERNET_TRANSPORT_OBJS  = $(TOP_DIR)/SIMULATION/ETH_TRANSPORT/netlink_init.o
endif
ETHERNET_TRANSPORT_OBJS += $(TOP_DIR)/SIMULATION/ETH_TRANSPORT/multicast_link.o
ETHERNET_TRANSPORT_OBJS += $(TOP_DIR)/SIMULATION/ETH_TRANSPORT/socket.o
ETHERNET_TRANSPORT_OBJS += $(TOP_DIR)/SIMULATION/ETH_TRANSPORT/bypass_session_layer.o
ETHERNET_TRANSPORT_OBJS += $(TOP_DIR)/SIMULATION/ETH_TRANSPORT/emu_transport.o
ETHERNET_TRANSPORT_OBJS += $(TOP_DIR)/SIMULATION/ETH_TRANSPORT/pgm_link.o