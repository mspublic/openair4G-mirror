#ifndef OPENAIR_DEVICE_H
#define OPENAIR_DEVICE_H


// Maximum number of concurrently supported cards
//
#define MAX_CARDS   4
#define INIT_ZEROS {0, 0, 0, 0}

// Vendor and System IDs
//
#define XILINX_VENDOR 0x10ee
#define XILINX_ID 0x0007
//
// PCIe Subsystem Vendor ID
#define EURECOM_VENDOR           0x0001
#define TELECOM_PARISTECH_VENDOR 0x0002
//
// PCIe Subsystem ID = exmimo_id.board_exmimoversion(1,2) (4 bits) | exmimo_id.board_hwrev (4 bits) | exmimo_id.board_swrev (Protocol Revision, 8 bits)
// Board IDs:
//   0x11 => ExpressMIMO-1, first run/HW revision
//   0x12 => ExpressMIMO-1, second run
//   0x21 => ExpressMIMO-2, first run
//   0x22 => ExpressMIMO-2, second run
//
// SW/Protocol revision:
#define BOARD_SWREV_LEGACY   0x07
#define BOARD_SWREV_CMDFIFOS 0x11


// Device IO definitions and operations
//
#define openair_MAJOR 127

//#define openair_writel(dev,offset,val)     pci_write_config_dword(dev,(int)offset,(unsigned int)val)//{writel((uclong)(val),(ulong)(port)); mb();}

//#define openair_readl(dev,offset,val)     pci_read_config_dword(dev,(int)offset,(unsigned int*)val)//{writel((uclong)(val),(ulong)(port)); mb();}

#define openair_IOC_MAGIC         'm'

#define openair_GET_BIGSHMTOPS_KVIRT         _IOR(openair_IOC_MAGIC,1,int)
#define openair_GET_PCI_INTERFACE_BOTS_KVIRT _IOR(openair_IOC_MAGIC,2,int)
#define openair_GET_NUM_DETECTED_CARDS       _IOR(openair_IOC_MAGIC,3,int)

#define openair_DUMP_CONFIG                  _IOR(openair_IOC_MAGIC,18,int)
#define openair_GET_FRAME                    _IOR(openair_IOC_MAGIC,6,int)
#define openair_START_RT_ACQUISITION         _IOR(openair_IOC_MAGIC,28,int)
#define openair_STOP                         _IOR(openair_IOC_MAGIC,5,int)
#define openair_UPDATE_FIRMWARE              _IOR(openair_IOC_MAGIC,40,int)


/* Update firmware commands */
#define UPDATE_FIRMWARE_TRANSFER_BLOCK    0x1
#define UPDATE_FIRMWARE_CLEAR_BSS         0x2
#define UPDATE_FIRMWARE_START_EXECUTION   0x3
#define UPDATE_FIRMWARE_FORCE_REBOOT      0x4
#define UPDATE_FIRMWARE_TEST_GOK          0x5


// mmap page offset vg_pgoff is used to pass arguments to kernel
// bit0..3: memory block: BIGSHM:0, RX:1,3,5,7, TX:2,4,6,8
// bit4..7: card_id
#define openair_mmap_BIGSHM            0
#define openair_mmap_RX(ant) (((ant)<<1)+1)
#define openair_mmap_TX(ant) (((ant)<<1)+2)

#define openair_mmap_getMemBlock(o)  ((o)&0xF)
#define openair_mmap_getAntRX(o) (((o)-1)>>1)
#define openair_mmap_getAntTX(o) (((o)-2)>>1)

#define openair_mmap_Card(c)    ( ((c)&0xF)<<4 )
#define openair_mmap_getCard(o) ( ((o)>>4)&0xF )


#endif /* OPENAIR_DEVICE_H */
