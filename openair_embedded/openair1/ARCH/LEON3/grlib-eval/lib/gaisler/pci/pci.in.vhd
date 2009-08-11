-- PCI interface
  constant CFG_PCI     	  : integer := CFG_PCITYPE;
  constant CFG_PCIVID     : integer := 16#CONFIG_PCI_VENDORID#;
  constant CFG_PCIDID     : integer := 16#CONFIG_PCI_DEVICEID#;
  constant CFG_PCIDEPTH   : integer := CFG_PCIFIFO;
  constant CFG_PCI_MTF    : integer := CFG_PCI_ENFIFO;
  constant CFG_PCI_HADDR  : integer := 16#CONFIG_PCI_HADDR#;
  constant CFG_PCI_IRQ    : integer := CONFIG_PCI_IRQ;
  constant CFG_PCIDMA_IRQ : integer := CONFIG_PCIDMA_IRQ;

