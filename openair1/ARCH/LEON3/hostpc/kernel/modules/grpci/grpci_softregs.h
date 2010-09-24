/* Automatically generated based on softregs.h from Grlib . Don't edit. */
/* Thu May 10 12:15:51 CEST 2007 */

#ifndef FROM_GRLIB_SOFT_REGS_H
#define FROM_GRLIB_SOFT_REGS_H

/* Scaler & timers control regs (GPTIMER unit) */
#define FROM_GRLIB_TIMER_ENABLE                0x1
#define FROM_GRLIB_TIMER_DISABLE               0x0
#define FROM_GRLIB_TIMER_AUTO_RELOAD           0x2
#define FROM_GRLIB_TIMER_RELOAD                0x4
#define FROM_GRLIB_TIMER_IRQ_ENABLE            0x8
#define FROM_GRLIB_TIMER_IRQ_DISABLE           0x0
#define FROM_GRLIB_TIMER_IRQ_PENDING           0x10
#define FROM_GRLIB_TIMER_CLEAR_PENDING_IRQ     (~0x10)      /* 'anding a timer control register with this value reset the pending irq. */
#define FROM_GRLIB_TIME_ZERO                   0x00ffffff
#define FROM_GRLIB_TIMER_CAN_BE_FREEZED        0x200
#define FROM_GRLIB_TIMER_SEPARATE_IRQ          0x100
#define FROM_GRLIB_TIMER_IRQ_SHIFT             3

/* Value of the Sparc TRAP BASE REGISTER (%tbr) */
#define FROM_GRLIB_SPARC_TBR          0x00000000

/* Interface with Host PC */
#define FROM_GRLIB_BOOT_HOK                             0x4
#define FROM_GRLIB_BOOT_GOK                             0x2
#define FROM_GRLIB_IRQ_FROM_PCI                         0x1
#define FROM_GRLIB_IRQ_FROM_PCI_ACK                     IRQ_FROM_PCI
#define FROM_GRLIB_IRQ_FROM_PCI_MASK                    0x0000ff00
#define FROM_GRLIB_IRQ_FROM_PCI_IS_DO_NOTHING           0x00000000
#define FROM_GRLIB_IRQ_FROM_PCI_IS_PERFORM_PCIDMA       0x00000100
#define FROM_GRLIB_IRQ_FROM_PCI_IS_CLEAR_BSS            0x00000200
#define FROM_GRLIB_IRQ_FROM_PCI_IS_VERIFY_CHECKSUM      0x00000300
#define FROM_GRLIB_IRQ_FROM_PCI_IS_JUMP_USER_ENTRY      0x00000400
#define FROM_GRLIB_IRQ_FROM_PCI_IS_SET_ADF4108_REG      0x00001000
#define FROM_GRLIB_IRQ_FROM_PCI_IS_INIT_ADF4108         0x00001100
//#define FROM_GRLIB_IRQ_FROM_PCI_IS_SET_ADF4108_N_Cnt    0x00001100
//#define FROM_GRLIB_IRQ_FROM_PCI_IS_SET_ADF4108_Func     0x00001200
#define FROM_GRLIB_IRQ_FROM_PCI_IS_SET_LFSW190410_KHZ   0x00002000
#define FROM_GRLIB_IRQ_FROM_PCI_IS_SET_RF_SWITCH        0x00003000
#define FROM_GRLIB_FIRMWARE_FILE_TAG                    CRDMIMO1FIRMWARE
#define FROM_GRLIB_PCI_MEMORY_WRITE_TYPE_BITPOS         10
#define FROM_GRLIB_PCI_MEMORY_READ_TYPE_BITPOS          9
#define FROM_GRLIB_PCI_MEMORY_WRITE                     (0<<PCI_MEMORY_WRITE_TYPE_BITPOS)
#define FROM_GRLIB_PCI_MEMORY_WRITE_AND_INVALIDATE      (1<<PCI_MEMORY_WRITE_TYPE_BITPOS)
#define FROM_GRLIB_PCI_MEMORY_READ_MULTIPLE             (0<<PCI_MEMORY_READ_TYPE_BITPOS)
#define FROM_GRLIB_PCI_MEMORY_READ_LINE                 (1<<PCI_MEMORY_READ_TYPE_BITPOS)
/* PCI DMAs */
  /* control */
#define FROM_GRLIB_DMA_FROM_HOSTPC_TO_CARD    0
#define FROM_GRLIB_DMA_FROM_CARD_TO_HOSTPC    1
#define FROM_GRLIB_DMA_GO                     0x001
#define FROM_GRLIB_DMA_READ_FROM_PCI          0x000
#define FROM_GRLIB_DMA_WRITE_TO_PCI           0x002
#define FROM_GRLIB_DMA_MEMORY_CYCLES          0x080
#define FROM_GRLIB_DMA_IO_CYCLES              0x040
#define FROM_GRLIB_DMA_IRQ_ENABLE             0x100
#define FROM_GRLIB_DMA_BUSY                   1
#define FROM_GRLIB_DMA_NOT_BUSY               0
#define FROM_GRLIB_DMA_BUSY_ERROR             -1
#define FROM_GRLIB_DMA_RUNNING                0
#define FROM_GRLIB_DMA_ERR_BITPOS             3
#define FROM_GRLIB_DMA_ERR_SET_VALUE          1
#define FROM_GRLIB_DMA_ERR_MASK               (1<<DMA_ERR_BITPOS)
#define FROM_GRLIB_DMA_RESET_ERR              ((~DMA_ERR_SET_VALUE)<<DMA_ERR_BITPOS)

  /* status */
#define FROM_GRLIB_DMA_READY                  

/* Leds on rf0 and rf1 registers */
  /* leds 0-3 (rf0) */
#define FROM_GRLIB_RF0_LED0                   0x00700000
#define FROM_GRLIB_RF0_LED1                   0x00680000
#define FROM_GRLIB_RF0_LED2                   0x00580000
#define FROM_GRLIB_RF0_LED3                   0x00380000
#define FROM_GRLIB_RF0_NO_LED_0_TO_3          0x00780000
#define FROM_GRLIB_RF0_LEDS_0_TO_3_MASK       0x00780000
  /* leds 4-5 (rf1) */
#define FROM_GRLIB_RF1_LED4                   0x00700000
#define FROM_GRLIB_RF1_LED5                   0x00680000
#define FROM_GRLIB_RF1_LED6                   0x00580000
#define FROM_GRLIB_RF1_LED7                   0x00380000
#define FROM_GRLIB_RF1_NO_LED_4_TO_7          0x00780000
#define FROM_GRLIB_RF1_LEDS_4_TO_7_MASK       0x00780000
#define FROM_GRLIB_LED_DIRECTION_LEFT         0
#define FROM_GRLIB_LED_DIRECTION_RIGHT        (~LED_DIRECTION_LEFT)

/* CMIMO1: Freq. Synthesizers on rf0 and rf1 registers */
#define FROM_GRLIB_RF0_GET_TX1_BITS           (0x00001c00)
#define FROM_GRLIB_RF0_MASK_TX1_BITS          (~RF0_GET_TX1_BITS)
#define FROM_GRLIB_RF0_GET_TX1_RX1_BITS       (0x00021c00)
#define FROM_GRLIB_RF0_MASK_TX1_RX1_BITS      (~RF0_GET_TX1_RX1_BITS)
  /* The TX1 bits contain the CLK, DATA1 and LATCH1 bits */
#define FROM_GRLIB_RF0_TX1_CLK_BITPOS         12
#define FROM_GRLIB_RF0_TX1_CLK_BIT            (1<<RF0_TX1_CLK_BITPOS) /* 0x00001000 */
#define FROM_GRLIB_RF0_TX1_DATA_BITPOS        11
#define FROM_GRLIB_RF0_TX1_DATA_BIT           (1<<RF0_TX1_DATA_BITPOS) /* 0x00000800 */
#define FROM_GRLIB_RF0_TX1_LATCH1_BITPOS      10
#define FROM_GRLIB_RF0_TX1_LATCH1_BIT         (1<<RF0_TX1_LATCH1_BITPOS) /* 0x00000400 */
  /* The RX1 bits contain the LATCH2 bit */
#define FROM_GRLIB_RF0_RX1_LATCH2_BITPOS      17
#define FROM_GRLIB_RF0_RX1_LATCH2_BIT         (1<<RF0_RX1_LATCH2_BITPOS) /* 0x00020000 */
  /* SPI buses & RF control switches on RF2 register */
    /* bit positions */
#define FROM_GRLIB_RF2_LATCH1_BITPOS             0
#define FROM_GRLIB_RF2_LATCH2_BITPOS             1
#define FROM_GRLIB_RF2_CLK_BITPOS                2
#define FROM_GRLIB_RF2_DATA_BITPOS               3
#define FROM_GRLIB_RF2_SW_TX_BITPOS              4
#define FROM_GRLIB_RF2_SW_ANT_BITPOS             5
#define FROM_GRLIB_RF2_SW_RXIN_BITPOS            6
#define FROM_GRLIB_RF2_SW_RXDC2G_BITPOS          7
#define FROM_GRLIB_RF2_SW_RX27G_BITPOS           8
#define FROM_GRLIB_RF2_SW_RXOUT_BITPOS           9
#define FROM_GRLIB_RF2_LATCH3_BITPOS             10
#define FROM_GRLIB_RF2_LATCH4_BITPOS             11
#define FROM_GRLIB_RF2_SW_RXDC4G_BITPOS          12
#define FROM_GRLIB_RF2_SW_LO_BITPOS              14
  /* and values */
#define FROM_GRLIB_RF2_LATCH1_MASK               (1<<RF2_LATCH1_BITPOS)
#define FROM_GRLIB_RF2_LATCH2_MASK               (1<<RF2_LATCH2_BITPOS)
#define FROM_GRLIB_RF2_CLK_MASK                  (1<<RF2_CLK_BITPOS)
#define FROM_GRLIB_RF2_DATA_MASK                 (1<<RF2_DATA_BITPOS)
#define FROM_GRLIB_RF2_SW_TX_MASK                (1<<RF2_SW_TX_BITPOS)
#define FROM_GRLIB_RF2_SW_ANT_MASK               (1<<RF2_SW_ANT_BITPOS)
#define FROM_GRLIB_RF2_SW_RXIN_MASK              (1<<RF2_SW_RXIN_BITPOS)
#define FROM_GRLIB_RF2_SW_RXC2G_MASK             (1<<RF2_SW_RXC2G_BITPOS)
#define FROM_GRLIB_RF2_SW_RX27G_MASK             (1<<RF2_SW_RX27G_BITPOS)
#define FROM_GRLIB_RF2_SW_RXOUT_MASK             (1<<RF2_SW_RXOUT_BITPOS)
#define FROM_GRLIB_RF2_LATCH3_MASK               (1<<RF2_LATCH3_BITPOS)
#define FROM_GRLIB_RF2_LATCH4_MASK               (1<<RF2_LATCH4_BITPOS)
#define FROM_GRLIB_RF2_SW_RXDC4G_MASK            (3<<RF2_SW_RXDC4G_BITPOS)
#define FROM_GRLIB_RF2_SW_LO_MASK                (3<<RF2_SW_LO_BITPOS)

/* CMIMO1: ADAC registers */
#define FROM_GRLIB_CMIMO1_SWITCH_OFFSET_INIT  19
/* Bit position in ADAC_CONFIG register */
#define FROM_GRLIB_CMIMO1_SOFT_RESET_BITPOS   31
#define FROM_GRLIB_CMIMO1_RX_MODE_BITPOS      26
#define FROM_GRLIB_CMIMO1_SLOT_ALLOC_BITPOS_0 24
#define FROM_GRLIB_CMIMO1_SLOT_ALLOC_BITPOS_1 25
#define FROM_GRLIB_CMIMO1_CYC_PFX_LGT_BITPOS  4
#define FROM_GRLIB_CMIMO1_LOG2_SYM_SZ_BITPOS  0
#define FROM_GRLIB_CMIMO1_DO_SOFT_RESET       1
#define FROM_GRLIB_CMIMO1_UNDO_SOFT_RESET     0
#define FROM_GRLIB_CMIMO1_SLOT_ALLOC_DEFAULT  3
#define FROM_GRLIB_CMIMO1_CYC_PFX_LGT_DEFAULT 63
#define FROM_GRLIB_CMIMO1_LOG2_SYM_SZ_DEFAULT 8
/* SPI Tranfers */
#define FROM_GRLIB_CMIMO1_SPI_START           0x1

/* Significant values for registers content */
#define FROM_GRLIB_CMIMO1_ODD_SYMBOL_RX   (1<<CMIMO1_SLOT_ALLOC_BITPOS_1)
#define FROM_GRLIB_CMIMO1_EVEN_SYMBOL_RX  (1<<CMIMO1_SLOT_ALLOC_BITPOS_0)
#define FROM_GRLIB_CMIMO1_ODD_SYMBOL_TX   (~CMIMO1_ODD_SYMBOL_RX)
#define FROM_GRLIB_CMIMO1_EVEN_SYMBOL_TX  (~CMIMO1_EVEN_SYMBOL_RX)

#endif /* FROM_GRLIB_SOFT_REGS_H */
