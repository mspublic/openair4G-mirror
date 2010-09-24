#ifndef GRPCI_H
#define GRPCI_H

#include "grpci_softconfig.h"
#include "grpci_softregs.h"

/* Registration of the driver */
#define GRPCI_DEVICE_NAME "grpci"  /* used both for printks banner, major number allocation, and PCI driver registration */
#define GRPCI_DEVICE_FIRMWARE GRPCI_DEVICE_NAME".bin"
                                   /* name of the firmware to download to the card upon its hotplug */
#define GRPCI_MINOR_0 0
#define GRPCI_FIRST_MINOR 0        /* the 1st minor number to request for when calling alloc_chrdev_region() */
#define GRPCI_DEV_COUNT 1          /* the total number of devices to request for when calling alloc_chrdev_region() */

/* printkies */
#define GRPCI_PFX GRPCI_DEVICE_NAME": "

/* firmware download */
#ifndef GRPCI_BIGPHYS_PAGES
#warning "No value specified for Preprocessor symbol GRPCI_BIGPHYS_PAGES. Defaulting to 512"
#define GRPCI_BIGPHYS_PAGES 512
#endif
//#define GRPCI_BIGPHYS_PAGES 512
#define GRPCI_FIRMWARE_ALLOC_BYTE_SIZE (GRPCI_BIGPHYS_PAGES*(1<<PAGE_SHIFT))    /* (1<<PAGE_SHIFT) will yield 4096 on x86 */
#define GRPCI_FIRMWARE_FILETAG_CHAR_OFFSET    0
#define GRPCI_FIRMWARE_FILESIZE_WORD_OFFSET   4
#define GRPCI_FIRMWARE_CHECKSUM_WORD_OFFSET   5
#define GRPCI_FIRMWARE_ENTRYPOINT_WORD_OFFSET 6
#define GRPCI_FIRMWARE_BSSADDR_WORD_OFFSET    7
#define GRPCI_FIRMWARE_BSSSIZE_WORD_OFFSET    8
#define GRPCI_FIRMWARE_STACKPTR_WORD_OFFSET   9
#define GRPCI_FIRMWARE_NBELFSEC_WORD_OFFSET   10
#define GRPCI_FIRMWARE_FIRST_SEC_WORD_OFFSET  11
#define GRPCI_FIRMWARE_MAX_ELFSEC_BEFORE_WARNING  30
#define GRPCI_FIRMWARE_DMA_MAP_SINGLE_SIZE    (10*(1<<PAGE_SHIFT))

/* PCI Handshake with boot firmware */
  /* addresses & offsets */
#define GRPCI_PCICONFIG_STS_CMD                 0x04
#define GRPCI_PCICONFIG_BIST_HEAD_LTIM_CACHE    0x0c
#define GRPCI_PCICONFIG_MAXLAT_MINGNT_INT       0x3c
#define GRPCI_IOCONFIG_CTRL        0x40
#define GRPCI_IOCONFIG_EXPPAGE0    0x48
#define GRPCI_IOCONFIG_SHARED0OFF  0x50
#define GRPCI_IOCONFIG_SHARED0SIZE 0x54
#define GRPCI_IOCONFIG_CTRL0       0x60
#define GRPCI_IOCONFIG_CTRL1       0x64
#define GRPCI_IOCONFIG_CTRL2       0x68
#define GRPCI_IOCONFIG_CTRL3       0x6c
#define GRPCI_TIMEOUT_GOK_SECONDS         10
#define GRPCI_TIMEOUT_CLEARBSS_SECONDS    10
#define GRPCI_TIMEOUT_FIRMDWLD_SECONDS    10
#define GRPCI_TIMEOUT_CHECKSUM_SECONDS    5
#define GRPCI_TIMEOUT_ENTRYPOINT_SECONDS  5
#define GRPCI_PCIMASTER_ENABLE          0x4

/* IOCTL code numbers (see [LnxDrv3] pp137-139) */
  /* Use 0xE0 as magic (8-bit) number (unused according to
   * <linux-2.6.15-rtai33/Documentation/ioctl-number.txt>) */
#define GRPCI_IOCTRL_MAGIC  0xE0
/* Control of ADF4108 Frequency Synthesizer */
#define GRPCI_IOCTRL_ADF4108_WRITE_REG   _IOW(GRPCI_IOCTRL_MAGIC, 0x1, int)
#define GRPCI_IOCTRL_ADF4108_INIT        _IOW(GRPCI_IOCTRL_MAGIC, 0x2, int)
//#define GRPCI_IOCTRL_ADF4108_WRITE_N_CNT     _IOW(GRPCI_IOCTRL_MAGIC, 0x2, int)
//#define GRPCI_IOCTRL_ADF4108_WRITE_FUNC      _IOW(GRPCI_IOCTRL_MAGIC, 0x3, int)
/* Control of LFSW190410-50 Frequency Synthesizer */
#define GRPCI_IOCTRL_LFSW190410_WRITE_KHZ    _IOW(GRPCI_IOCTRL_MAGIC, 0x10, char*)
#define GRPCI_IOCTRL_RF_SWITCH_CTRL          _IOW(GRPCI_IOCTRL_MAGIC, 0x20, char*)
#define GRPCI_IOCTRL_SETTX_SWITCH_AND_GAINS  _IOW(GRPCI_IOCTRL_MAGIC, 0x30, char*)


#ifdef __KERNEL__
/* Userland has nothing to do with the following material */
static int grpci_probe(struct pci_dev*, const struct pci_device_id*);
static void grpci_remove(struct pci_dev*);
static int grpci_open(struct inode*, struct file*);
static int grpci_release(struct inode*, struct file*);
int grpci_ioctl(struct inode*, struct file*, unsigned int, unsigned long);
void grpci_timeout_gok(unsigned long data);
void grpci_timeout_clearbss(unsigned long data);
void grpci_timeout_firmdwld(unsigned long data);
void grpci_timeout_checksum(unsigned long data);
void grpci_timeout_entrypoint(unsigned long data);

struct struct_firmw {
  unsigned int bytesize;
  unsigned long bssaddr;
  unsigned long bsssize;
  unsigned long stackptr;
  unsigned int  nbelfsec;
  unsigned int  checksum;
  unsigned int  entrypoint;
};

struct struct_RFctrl {
  /* ADF4108 Freq. Synthesizer Registers */
  unsigned int ADF4108_Func0;
  unsigned int ADF4108_Ref_Cnt;
  unsigned int ADF4108_N_Cnt;
  unsigned int ADF4108_Func1;
  unsigned int ADF4108_Init;
  /* LFSW190410-50 Freq. Synthesizer Register */
  char* LFSW190410_KHZ;
  /* Control Switches of RF chain */
  unsigned int RFswitches_onoff;
  unsigned int RFswitches_mask;  
};

struct grpci_device {
  /* PCI features */
  struct pci_dev* pcidev;
  unsigned long resstart;
  unsigned long resend;
  unsigned long reslen;
  unsigned long resflags;
  unsigned int* bar0;
  dma_addr_t dma_addr;
  /* char device features */
  struct cdev cdev;
  dev_t devt;
  int major;
  struct file_operations fops;
  struct firmware* fw;
  /* bigphys related info */
  caddr_t bigphys_ptr;
  dma_addr_t dma_addr_bigphys;
  /* PCI handshake (config space used as I/O) */
  unsigned int gok;
  struct timer_list timeout_timer_gok;
  struct timer_list timeout_timer_clearbss;
  struct timer_list timeout_timer_firmdwld;
  struct timer_list timeout_timer_checksum;
  struct timer_list timeout_timer_entrypoint;
  unsigned int timeout_expired_gok;
  unsigned int timeout_expired_clearbss;
  unsigned int timeout_expired_firmdwld;
  unsigned int timeout_expired_checksum;
  unsigned int timeout_expired_entrypoint;
  unsigned int exppage0, shared0off, shared0size;
  /* PCI registers to ask for DMA */
  unsigned int dma_AHB_baseaddr, dma_PCI_baseaddr;
  unsigned int dma_nbwords, dma_sense;
  /* Firmware related info */
  struct struct_firmw firmw;
  /* RF ctrl related info */
  struct struct_RFctrl RFctrl;
};
#endif /* __KERNEL__ */

#endif /* GRPCI_H */
