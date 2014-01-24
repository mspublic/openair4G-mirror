
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
library grlib;
use grlib.amba.all;
use grlib.stdlib.all;
use grlib.devices.all;
library techmap;
use techmap.gencomp.all;
library eurecom;
use eurecom.cmimo1lib.all;
-- pragma translate_off
library unisim;
use unisim.vcomponents.all; -- this is for DCM simulation model
-- pragma translate_on


entity cmimo1 is
  generic (
    memtech   : integer range 0 to NTECH := 0;
    slvndx    : integer := 0;
    ioaddr    : integer := 16#000#;
    iomask    : integer := 16#FFF#;
    rfiooff   : integer := 16#00#;
    adaciooff : integer := 16#20#;
    spiiooff  : integer := 16#40#;
    idspiiooff: integer := 16#80#;
    haddr     : integer := 16#000#;
    hmask     : integer := 16#F00#;
    adc0hoff  : integer := 16#0#;
    adc1hoff  : integer := 16#1#;
    dac0hoff  : integer := 16#2#;
    dac1hoff  : integer := 16#3#;
    adc0size  : integer := 2;
    adc1size  : integer := 2;
    dac0size  : integer := 2;
    dac1size  : integer := 2;
    adc0res   : integer := 12;
    adc1res   : integer := 12;
    pirq      : integer := 0;
    boardvers : integer := 1
  );
  port (
    rstn         : in  std_logic;
    clk          : in  std_logic;
    clk26mhz     : in  std_logic;
    clk_15_36mhz : in  std_logic;
    ahbsi        : in  ahb_slv_in_type;
    ahbso        : out ahb_slv_out_type;
    wmodemi      : in  wmodem_in_type;
    wmodemo      : out wmodem_out_type;
    osc1_01      : out std_logic
  );                    
end entity cmimo1;

architecture rtl of cmimo1 is 

  component DCM
    generic(CLKDV_DIVIDE : real:= 1.0; -- (1.5, 2.0, 2.5, 3.0, 4.0, 5.0, 8.0, 16.0)
            CLKFX_DIVIDE : integer := 1; -- (1 to 4096)
            CLKFX_MULTIPLY : integer := 2; -- (1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 5.0, 5.5,
                                           --  6.0, 6.5, 7.0, 7.5, 8.0, 9.0, 10.0, 11.0,
                                           --  12.0, 13.0, 14.0, 15.0, 16.0)
            CLKIN_PERIOD : real := 0.0;
            CLKIN_DIVIDE_BY_2 : boolean := FALSE; -- (TRUE, FALSE)
            CLKOUT_PHASE_SHIFT : string := "NONE";
            CLK_FEEDBACK : string := "1X";
            DESKEW_ADJUST : string := "SYSTEM_SYNCHRONOUS"; 
            DFS_FREQUENCY_MODE : string := "LOW"; 
            DLL_FREQUENCY_MODE : string := "LOW"; 
            DUTY_CYCLE_CORRECTION : Boolean := TRUE; -- (TRUE, FALSE)
            DSS_MODE : string := "NONE"; 
            PHASE_SHIFT : integer := 0;
            STARTUP_WAIT : boolean := FALSE); -- (TRUE, FALSE)
    port (CLK0 : out STD_ULOGIC;
          CLK180 : out STD_ULOGIC;
          CLK270 : out STD_ULOGIC;
          CLK2X : out STD_ULOGIC;
          CLK2X180 : out STD_ULOGIC;
          CLK90 : out STD_ULOGIC;
          CLKDV : out STD_ULOGIC;
          CLKFX : out STD_ULOGIC;
          CLKFX180 : out STD_ULOGIC;
          LOCKED : out STD_ULOGIC;
          PSDONE : out STD_ULOGIC;
          STATUS : out STD_LOGIC_VECTOR (7 downto 0);
          CLKFB : in STD_ULOGIC;
          CLKIN : in STD_ULOGIC;
          DSSEN : in STD_ULOGIC;
          PSCLK : in STD_ULOGIC;
          PSEN : in STD_ULOGIC;
          PSINCDEC : in STD_ULOGIC;
          RST : in STD_ULOGIC);
  end component;

constant REVISION : amba_version_type := 0;
constant AHB_IO_BAR_INDEX  : integer := 4;
constant AHB_MEM_BAR_INDEX : integer := 5;

constant hconfig : ahb_config_type := (
  0 => ahb_device_reg ( VENDOR_GAISLER, GAISLER_CANAHB, 0, REVISION, pirq),            -- legal terms : eventualy modify THIS
  AHB_IO_BAR_INDEX => ahb_iobar(ioaddr, iomask),
  AHB_MEM_BAR_INDEX => ahb_membar(haddr, '0', '0', hmask),
  others => zero32);

-- Constants for AHB Address Offsets of RF Control Register file
constant RF_IO_OFFSET  : integer := (rfiooff / 16);
constant WDNS_RFCTL0   : integer := (RF_IO_OFFSET * 16) + 0 ;
constant WDNS_RFCTL1   : integer := (RF_IO_OFFSET * 16) + 4;
constant IDROMEL_RFCTL : integer := (RF_IO_OFFSET * 16) + 8;
-- Constants for AHB Address Offsets of ADAC Control Register file
constant ADAC_IO_OFFSET    : integer := (adaciooff / 16);
constant ADAC_CONFIG       : integer := (ADAC_IO_OFFSET * 16) + 0;
constant FRAME_LEN         : integer := (ADAC_IO_OFFSET * 16) + 4;
constant FRAME_STRT        : integer := (ADAC_IO_OFFSET * 16) + 8;
constant SWITCH_OFFSET     : integer := (ADAC_IO_OFFSET * 16) + 12;
constant ADAC_INTR_COUNTER : integer := (ADAC_IO_OFFSET * 16) + 16;
-- Constants for AHB Address Offsets of ADAC Sample Memories
constant ADC0_MEM_OFFSET : integer := adc0hoff;
constant ADC1_MEM_OFFSET : integer := adc1hoff;
constant DAC0_MEM_OFFSET : integer := dac0hoff;
constant DAC1_MEM_OFFSET : integer := dac1hoff;
constant MAX_ADAC_ADDR : integer := 14;  -- Bus width to address a 64 Kbytes memory on a word basis
-- Constants for AHB Address Offsets of SPI Control/Config Register file
constant SPI_IO_OFFSET : integer := (spiiooff / 16);
constant SPI_DATA0   : integer := (SPI_IO_OFFSET * 16) + (0 * 4) ; -- +0x00
constant SPI_DATA1   : integer := (SPI_IO_OFFSET * 16) + (1 * 4) ; -- +0x04
constant SPI_DATA2   : integer := (SPI_IO_OFFSET * 16) + (2 * 4) ; -- +0x08
constant SPI_DATA3   : integer := (SPI_IO_OFFSET * 16) + (3 * 4) ; -- +0x0c
constant SPI_DATA4   : integer := (SPI_IO_OFFSET * 16) + (4 * 4) ; -- +0x10
constant SPI_DATA5   : integer := (SPI_IO_OFFSET * 16) + (5 * 4) ; -- +0x14
--constant SPI_DATA6   : integer := (SPI_IO_OFFSET * 16) + (6 * 4) ; -- +0x18
--constant SPI_DATA7   : integer := (SPI_IO_OFFSET * 16) + (7 * 4) ; -- +0x1c
constant SPI_ENABLES : integer := (SPI_IO_OFFSET * 16) + (8 * 4) ; -- +0x20
constant SPI_SIZES   : integer := (SPI_IO_OFFSET * 16) + (9 * 4) ; -- +0x24
constant SPI_START   : integer := (SPI_IO_OFFSET * 16) + (10 * 4) ; -- +0x28
-- Constants for AHB Address Offsets of Idromel SPI Control/Config Register file
constant SPI_IDROMEL_IO_OFFSET : integer := (idspiiooff / 16);
constant SPI_ADF4108_FUNC : integer := (SPI_IDROMEL_IO_OFFSET * 16) + (0 * 4) ; -- +0x00
constant SPI_ADF4108_NCNT : integer := (SPI_IDROMEL_IO_OFFSET * 16) + (1 * 4) ; -- +0x04
constant SPI_ADF4108_RCNT : integer := (SPI_IDROMEL_IO_OFFSET * 16) + (2 * 4) ; -- +0x08
constant SPI_LFSW_CMD     : integer := (SPI_IDROMEL_IO_OFFSET * 16) + (4 * 4) ; -- +0x10
constant SPI_LFSW_KHZ_0   : integer := (SPI_IDROMEL_IO_OFFSET * 16) + (5 * 4) ; -- +0x14
constant SPI_LFSW_KHZ_1   : integer := (SPI_IDROMEL_IO_OFFSET * 16) + (6 * 4) ; -- +0x18
constant SPI_SETTX        : integer := (SPI_IDROMEL_IO_OFFSET * 16) + (8 * 4) ; -- +0x20
constant SPI_SETRX        : integer := (SPI_IDROMEL_IO_OFFSET * 16) + (9 * 4) ; -- +0x24
constant SPI_CTRL         : integer := (SPI_IDROMEL_IO_OFFSET * 16) + (12 * 4) ; -- +0x30
-- Miscellaneous
constant ONE_AS_AN_9BITS_UNSIGNED : unsigned(8 downto 0) := "000000001";
constant ZERO_AS_AN_INTEGER : integer := 0;
constant ONE_AS_AN_INTEGER : integer := 1;
constant NUMBER_1023_AS_A_10BITS_UNSIGNED : unsigned(9 downto 0) := "1111111111";
constant ZERO_AS_A_32BITS_STD_LOGIC_VECTOR : std_logic_vector(31 downto 0) := (others => '0');
-- To delay the reset of the 2nd DCM (the one generating clk_7_68mhz)
constant NCYCLES_DELAY_NRSTN : integer := 64;
constant LOG2_NCYCLES_DELAY_NRSTN : integer := log2(NCYCLES_DELAY_NRSTN);
-- For SPI interface of Idromel new RF
constant SPI_LOG2_MAX_DELAY : integer range 0 to     14    :=     14;
constant SPI_MAX_DELAY      : integer range 0 to (2**14)-1 := (2**14)-1;
constant SPI_ID_ADF4108     : std_logic_vector(1 downto 0) := "00";
constant SPI_ID_LFSW        : std_logic_vector(1 downto 0) := "01";
constant SPI_ID_SETTX       : std_logic_vector(1 downto 0) := "10";
constant SPI_ID_SETRX       : std_logic_vector(1 downto 0) := "11";
constant SPI_MAX_BITS       : integer := 72;
constant SPI_MAX_BITS_POW2  : integer := (2**log2(SPI_MAX_BITS));

type access_range_type is (IO, MEM);
type adac_mux_type is (ADC0, ADC1, DAC0, DAC1);
type spi_regfile_type is array(5 downto 0) of std_logic_vector(23 downto 0);
type spi_state_type is (SPI_IDLE, SPI_WRITE);
type adac_intr_resync_type is array(0 to 1) of std_logic;

-- Structured type for all registers of the AHB clock domain
type regs_ahb is record
  -- AHB interface
  --hready               : std_ulogic;
  --hresp                : std_logic_vector(1 downto 0);
  hrdata               : std_logic_vector(31 downto 0);
  --hsize                : std_logic_vector(2 downto 0);
  adac_addr            : std_logic_vector(MAX_ADAC_ADDR - 1 downto 0);
  write                : boolean;  -- Needed for the ADAC memories: if read   they must latch address in the cycle (on-the-fly),
                                   --                           and if write, they must latch the address one cycle after it is driven on the AHB bus.
  iooffset             : natural range 0 to 255;
  access_range         : access_range_type;
  -- ADAC intr
  adac_intr_resync     : adac_intr_resync_type;
  adac_intr            : std_logic; -- resynchronization of adac_intr from clk_7_68mhz clock domain to ahb clock domain
  adac_intr_prevcycle  : std_logic;
  -- ADAC intr reset
  adac_intr_reset_resync : adac_intr_resync_type;
  adac_intr_reset      : std_logic; -- resynchronization of adac_intr_reset from clk_7_68mhz clock domain to ahb clock domain
  adac_intr_reset_prevcycle : std_logic;
  -- ADAC intr counter
  adac_intr_counter    : unsigned(8 downto 0);
  -- RF Control Register File
  rf0                  : std_logic_vector(31 downto 0);
  rf1                  : std_logic_vector(31 downto 0);
  rf_swt_lo_f          : std_logic;
  rf_swt_rx_tx         : std_logic;
  rf_reset_lfsw        : std_logic;
  rf_swt_tx3_1         : std_logic;
  rf_pwrdwn_adf        : std_logic;
  rf_swt_tx3_2         : std_logic;
  rf_swt_tx3_3         : std_logic;
  rf_swt_lo1           : std_logic;
  rf_swt_lo2           : std_logic;
  -- ADAC Control Register File
  soft_reset           : std_logic;
  slot_alloc           : std_logic_vector(1 downto 0);
  prefix_remove        : boolean;
  log2_symbol_size     : natural range 0 to 15;
  cyclic_prefix_length : natural range 0 to 255;
  frame_length         : natural range 0 to 16777215;  -- 24 bit range
  frame_start          : natural range 0 to 16777215;  -- 24 bit range
  switch_offset        : unsigned(7 downto 0);
  -- ADAC Memories Interface
  adac_we              : std_ulogic_vector(3 downto 0);
  hrdatamux            : adac_mux_type;
  -- SPI interface (for Widens native RF)
  spi_regfile_ahb      : spi_regfile_type; -- this one is the one accessed on AHB interface...
  spi_regfile          : spi_regfile_type; -- ...and is latched into this one when starting SPI transfer.
  spi_enables_ahb      : std_logic_vector(5 downto 0); -- this one is the one accessed on AHB interface...
  spi_enables          : std_logic_vector(5 downto 0); -- ...and is latched into this one when starting SPI transfer.
  spi_sizes_ahb        : std_logic_vector(5 downto 0); -- this one is the one accessed on AHB interface...
  spi_sizes            : std_logic_vector(5 downto 0); -- ...and is latched into this one when starting SPI transfer.
  spi_active           : std_ulogic;
  spi_state            : spi_state_type;
  spi_reg_len          : natural range 0 to 20;
  spi_xfer_len         : natural range 0 to 20;
  spi_clk              : std_logic;
  spi_enable_spiclk    : boolean;
  spi_clk_cnt          : unsigned(3 downto 0);
  -- SPI interface (for Idromel new RF)
  rf_spi_le             : std_logic_vector(4 downto 1);
  rf_spi_clk            : std_logic;
  rf_spi_data           : std_logic;
  rf_spi_cnt            : unsigned(SPI_LOG2_MAX_DELAY-1 downto 0);
  rf_spi_data2clk_cnten : boolean;
  rf_spi_data2data_cnten: boolean;
  rf_spi_clkwidth_cnten : boolean;
  rf_spi_regtype        : std_logic_vector(1 downto 0);
  rf_spi_adf_step       : unsigned(1 downto 0);
  rf_spi_active         : std_logic;
  rf_spi_adf4108_func   : std_logic_vector(23 downto 2);
  rf_spi_adf4108_ncnt   : std_logic_vector(23 downto 2);
  rf_spi_adf4108_rcnt   : std_logic_vector(23 downto 2);
  rf_spi_txbits         : std_logic_vector(71 downto 0);
  rf_spi_bit_ndx        : unsigned(log2(SPI_MAX_BITS_POW2)-1 downto 0);
  rf_spi_clklevel       : std_logic;
  rf_spi_adf_turnaround : boolean;
  rf_spi_log2_bit_width : integer range 0 to SPI_LOG2_MAX_DELAY-1;
end record;

-- Types for resynchronization registers from AHB clock domain to ADAC clock domain
type slot_alloc_resync_type is array(0 to 1) of std_logic_vector(1 downto 0);
type prefix_remove_resync_type is array(0 to 1) of boolean;
type log2_symbol_size_resync_type is array(0 to 1) of natural range 0 to 15;
type cyclic_prefix_length_resync_type is array(0 to 1) of natural range 0 to 255;
type frame_length_resync_type is array(0 to 1) of natural range 0 to 16777215;
type frame_start_resync_type is array(0 to 1) of natural range 0 to 16777215;
type switch_offset_resync_type is array(0 to 1) of unsigned(7 downto 0);
type soft_reset_resync_type is array(0 to 1) of std_logic;

-- Structured type for all registers of the clk_7_68mhz clock domain
type regs_clk_7_68mhz is record
  dac_sample_counter        : unsigned(8 downto 0);
  adc_sample_counter        : unsigned(9 downto 0);
  adc_sample_counter2       : unsigned(23 downto 0);
  adac_intr                 : std_logic;                   -- adac_intr, when asserted, means "generate an AHB irq (to Leon) and increments intr counter".
  adac_intr_reset           : std_logic;                   -- adac_intr_reset, when asserted, means "generate an AHB irq (to Leon) and reset intr counter to 0".
                                                           -- TODO: Shall 'adac_intr' be asserted during ONLY ONE cycle, or until it is handled ?
  --adac_intr_counter         : unsigned(8 downto 0);
  counter                   : unsigned(9 downto 0);
  dac_sample_counter_offset : unsigned(8 downto 0);
  mode_int                  : std_logic;
  mode                      : std_logic;
  doing_prefix              : boolean;
  in_prefix                 : boolean;
  -- registers coming from the AHB clock domain
  slot_alloc                : std_logic_vector(1 downto 0);
  prefix_remove             : boolean;
  log2_symbol_size          : natural range 0 to 15;
  cyclic_prefix_length      : natural range 0 to 255;
  frame_length              : natural range 0 to 16777215;  -- 24 bit range
  frame_start               : natural range 0 to 16777215;  -- 24 bit range
  switch_offset             : unsigned(7 downto 0);
  soft_reset                : std_logic;
  -- Resynchronization registers from AHB clock domain to ADAC clock domain
  slot_alloc_resync           : slot_alloc_resync_type;
  prefix_remove_resync        : prefix_remove_resync_type;
  log2_symbol_size_resync     : log2_symbol_size_resync_type;
  cyclic_prefix_length_resync : cyclic_prefix_length_resync_type;
  frame_length_resync         : frame_length_resync_type;
  frame_start_resync          : frame_start_resync_type;
  switch_offset_resync        : switch_offset_resync_type;
  soft_reset_resync           : soft_reset_resync_type;
end record;

signal r, rin     : regs_ahb;
signal rad, radin : regs_clk_7_68mhz;
signal adc_counter : std_logic_vector(log2(adc0size) + 7 downto 0); -- TODO: is it sure that ADC0 and ADC1 rams always share the same address bus on port B ??
signal dac_counter : std_logic_vector(log2(dac0size) + 7 downto 0); -- TODO: same question for DAC0 and DAC1 ??
signal adc0_dataout_portA, adc1_dataout_portA : std_logic_vector(31 downto 0);
signal adc0_dataout_portB_void, adc1_dataout_portB_void : std_logic_vector(31 downto 0);
signal dac0_dataout_portA, dac1_dataout_portA : std_logic_vector(31 downto 0);
signal dac0_dataout_portB, dac1_dataout_portB : std_logic_vector(31 downto 0);
signal clkfb0_in, clkfb0_out : std_logic;
signal clkfb1_in, clkfb1_out : std_logic;
--signal clkfb2_in, clkfb2_out : std_logic;
signal clk_7_68mhz : std_logic;
--signal clk_7_68mhz_90deg : std_logic;
signal adc0data, adc1data : std_logic_vector(31 downto 0);
signal nrstn : std_logic;
signal nrstn_delayed : std_logic;
signal nrstn_counter : unsigned(LOG2_NCYCLES_DELAY_NRSTN downto 0);
signal adac_addr : std_logic_vector(MAX_ADAC_ADDR - 1 downto 0);
--signal not_rad_mode_int : std_logic;
signal osc1_01_sig: std_logic;
signal sample_I_notQ : boolean;

function bool_to_std(bool: boolean) return std_logic is
begin
  if (bool) then return '1'; else return '0'; end if;
end function bool_to_std;

constant LOG2_SYMBOL_SIZE_RESET_VALUE : natural := 8;
constant CYCLIC_PREFIX_LENGTH_RESET_VALUE : natural := 63;
constant FRAME_LENGTH_RESET_VALUE : natural := 20479;
constant FRAME_START_RESET_VALUE : natural := 0;
constant SWITCH_OFFSET_RESET_VALUE : unsigned(7 downto 0) := to_unsigned(19, 8);
constant PREFIX_REMOVE_RESET_VALUE : boolean := FALSE;
constant SLOT_ALLOC_RESET_VALUE : std_logic_vector(1 downto 0) := "11";

component BUFG port (O : out std_logic; I : in std_logic); end component;
component BUFGCE port (O : out std_logic; I : in std_logic ; CE : in std_logic); end component;

begin

  -- ================                  ===
  -- AHB clock domain (clock signal is clk)
  -- ================                  ===
  comb_ahb : process(r, rstn, ahbsi, rad.adac_intr, rad.adac_intr_reset) --, rad.adac_intr_counter)
    variable v : regs_ahb;
    variable xirq : std_logic_vector(NAHBIRQ-1 downto 0);
    --variable v_spi_start : boolean;
    variable v_uns_iooffset : unsigned(7 downto 0);
  begin

    v := r;
    --v.hready := '1'; v.hresp := HRESP_OKAY;
    --v.write := FALSE;
    v.adac_addr := ahbsi.haddr(MAX_ADAC_ADDR + 1 downto 2);
    xirq := (others => '0');
    -- Resynchronization of register 'adac_intr'
    v.adac_intr_resync(1) := rad.adac_intr;          -- Adding 2 resynchronization "barrier" registers for adac_intar
    v.adac_intr_resync(0) := r.adac_intr_resync(1);  -- for its passage from the clk_7_68mhz clock domain to the AHB
    v.adac_intr := r.adac_intr_resync(0);            -- clock domain
    -- Resynchronization of register 'adac_intr_reset'
    v.adac_intr_reset_resync(1) := rad.adac_intr_reset;
    v.adac_intr_reset_resync(0) := r.adac_intr_reset_resync(1);
    v.adac_intr_reset := r.adac_intr_reset_resync(0);
    -- Detecting rising edges of 'adac_intr' and 'adac_intr_reset'
    v.adac_intr_prevcycle := r.adac_intr; -- An AHB interruption shall be raised when 'r.adac_intr' is high while 'r.adac_intr_prevcycle' is low.
                                          -- Indeed, it is an important assumption that, since ADAC_clk is low compared to AHB clock (7,68 MHz vs 52 MHz),
                                          -- each rising edge of 'rad.adac_intr' shall be seen in the AHB clock domain... (so TODO if ADAC_clk period is
                                          -- to change one day or another).
    v.adac_intr_reset_prevcycle := r.adac_intr_reset;  -- An AHB interrupt is also raised upon a rising edge of 'r.adac_intr_reset',
                                                       -- except in this case, r.adac_intr_counter is reset to 0.
    --v_spi_start := FALSE; -- This is to trigger SPI output transfers on the rising edge of SPI_START bit without having to use any actual register
    --                      -- for it (that is, we start the SPI output transfer 'on the fly' when software writes the SPI_START bit of the SPI
    --                      -- configuration register asserted, while SPI state machine is in the idle state).
    v_uns_iooffset := to_unsigned(r.iooffset, 8);

    -- AHB interface (I/O Writes management)
    if r.write then
      if (r.access_range = IO) then -- Write is in I/O space
        if (v_uns_iooffset = to_unsigned(WDNS_RFCTL0, 8)) then
          v.rf0 := ahbsi.hwdata;
        elsif (v_uns_iooffset = to_unsigned(WDNS_RFCTL1, 8)) then
          v.rf1 := ahbsi.hwdata;
        elsif (v_uns_iooffset = to_unsigned(IDROMEL_RFCTL, 8)) then
          v.rf_swt_lo_f   := ahbsi.hwdata(4);
          v.rf_swt_rx_tx  := ahbsi.hwdata(5);
          v.rf_reset_lfsw := ahbsi.hwdata(6);
          v.rf_swt_tx3_1  := ahbsi.hwdata(7);
          v.rf_pwrdwn_adf := ahbsi.hwdata(8);
          v.rf_swt_tx3_2  := ahbsi.hwdata(9);
          v.rf_swt_tx3_3  := ahbsi.hwdata(13);
          v.rf_swt_lo1    := ahbsi.hwdata(14);
          v.rf_swt_lo2    := ahbsi.hwdata(15);
        elsif (v_uns_iooffset = to_unsigned(ADAC_CONFIG, 8)) then
          v.soft_reset := ahbsi.hwdata(31);
          if (ahbsi.hwdata(26) = '1') then v.prefix_remove := TRUE; else v.prefix_remove := FALSE; end if;
          v.slot_alloc := ahbsi.hwdata(25 downto 24);
          v.cyclic_prefix_length := to_integer(unsigned(ahbsi.hwdata(11 downto 4)));
          v.log2_symbol_size :=     to_integer(unsigned(ahbsi.hwdata( 3 downto 0)));
        elsif (v_uns_iooffset = to_unsigned(FRAME_LEN, 8)) then
          v.frame_length := to_integer(unsigned(ahbsi.hwdata(23 downto 0)));
        elsif (v_uns_iooffset = to_unsigned(FRAME_STRT, 8)) then
          v.frame_start := to_integer(unsigned(ahbsi.hwdata(23 downto 0)));
        elsif (v_uns_iooffset = to_unsigned(SWITCH_OFFSET, 8)) then
          v.switch_offset := unsigned(ahbsi.hwdata(7 downto 0));
        elsif (v_uns_iooffset = to_unsigned(SPI_ENABLES, 8)) then
          v.spi_enables_ahb := ahbsi.hwdata(5 downto 0);
        elsif (v_uns_iooffset = to_unsigned(SPI_SIZES, 8)) then
          v.spi_sizes_ahb := ahbsi.hwdata(5 downto 0);
        elsif (v_uns_iooffset = to_unsigned(SPI_START, 8)) then
          if ((ahbsi.hwdata(0) = '1') and (r.spi_state = SPI_IDLE)) then
            --v_spi_start := TRUE;
            v.spi_active := '1';
          end if; -- Triggering SPI output transfers
        elsif (v_uns_iooffset = to_unsigned(SPI_CTRL, 8)) then
          v.rf_spi_regtype := ahbsi.hwdata(9 downto 8);
          v.rf_spi_log2_bit_width := to_integer(unsigned(ahbsi.hwdata(16+SPI_LOG2_MAX_DELAY-1 downto 16)));
          if (ahbsi.hwdata(0) = '1' and r.rf_spi_active = '0') then
            v.rf_spi_active := '1';
          end if;
        elsif (v_uns_iooffset = to_unsigned(SPI_ADF4108_FUNC, 8)) then
          v.rf_spi_adf4108_func := ahbsi.hwdata(23 downto 2);
        elsif (v_uns_iooffset = to_unsigned(SPI_ADF4108_NCNT, 8)) then
          v.rf_spi_adf4108_ncnt := ahbsi.hwdata(23 downto 2);
        elsif (v_uns_iooffset = to_unsigned(SPI_ADF4108_RCNT, 8)) then
          v.rf_spi_adf4108_rcnt := ahbsi.hwdata(23 downto 2);
        elsif (v_uns_iooffset = to_unsigned(SPI_LFSW_CMD, 8)) then
          for i in 0 to 7 loop
            v.rf_spi_txbits(i) := ahbsi.hwdata(7-i);
          end loop;
        elsif (v_uns_iooffset = to_unsigned(SPI_LFSW_KHZ_0, 8)) then
          for i in 8 to 39 loop
            v.rf_spi_txbits(i) := ahbsi.hwdata(39-i);
          end loop;
        elsif (v_uns_iooffset = to_unsigned(SPI_LFSW_KHZ_1, 8)) then
          for i in 40 to 71 loop
            v.rf_spi_txbits(i) := ahbsi.hwdata(71-i);
          end loop;
        elsif (v_uns_iooffset = to_unsigned(SPI_SETTX, 8)) then
          for i in 0 to 14 loop
            v.rf_spi_txbits(i) := ahbsi.hwdata(i+17);
          end loop;
        elsif (v_uns_iooffset = to_unsigned(SPI_SETRX, 8)) then
          for i in 0 to 14 loop
            v.rf_spi_txbits(i) := ahbsi.hwdata(i+17);
          end loop;
        elsif (v_uns_iooffset = to_unsigned(SPI_DATA0, 8)) then v.spi_regfile_ahb(0) := ahbsi.hwdata(23 downto 0);
        elsif (v_uns_iooffset = to_unsigned(SPI_DATA1, 8)) then v.spi_regfile_ahb(1) := ahbsi.hwdata(23 downto 0);
        elsif (v_uns_iooffset = to_unsigned(SPI_DATA2, 8)) then v.spi_regfile_ahb(2) := ahbsi.hwdata(23 downto 0);
        elsif (v_uns_iooffset = to_unsigned(SPI_DATA3, 8)) then v.spi_regfile_ahb(3) := ahbsi.hwdata(23 downto 0);
        elsif (v_uns_iooffset = to_unsigned(SPI_DATA4, 8)) then v.spi_regfile_ahb(4) := ahbsi.hwdata(23 downto 0);
        elsif (v_uns_iooffset = to_unsigned(SPI_DATA5, 8)) then v.spi_regfile_ahb(5) := ahbsi.hwdata(23 downto 0);
        --elsif (v_uns_iooffset = to_unsigned(SPI_DATA6, 8)) then v.spi_regfile_ahb(6) := ahbsi.hwdata(23 downto 0);
        --elsif (v_uns_iooffset = to_unsigned(SPI_DATA7, 8)) then v.spi_regfile_ahb(7) := ahbsi.hwdata(23 downto 0);
        end if;
      end if;
    end if;

    -- AHB interface (continuing)
    if ((ahbsi.hready and ahbsi.hsel(slvndx)) = '1') then
      if (ahbsi.htrans = HTRANS_NONSEQ or ahbsi.htrans = HTRANS_SEQ) then -- NONSEQ or SEQ
        --v.hsize := ahbsi.hsize; -- Nevertheless, the only non-32bit support is for the configuration register of SPI interface
        --                        -- (that is, the size latched in r.hsize will only be used for the config register of SPI interface)
        --                        -- For all other accesses, we simply ignore the size, wo/ generating error, considering ths whole 32bits on data bus.
        --                        -- TODO: Mind software interface!
        --                        -- Otherwise, for a stronger policy, this is the code to generate error:
        --                        -- if (ahbsi.hsize /= HSIZE_WORD) then v.hready := '0'; -- two-cycles ERROR response
        --                        --                                     v.hresp := HRESP_ERROR;
        --                        --                                     v.adac_we := "0000"; end if;
        if (ahbsi.hwrite = '1') then -- write
          v.write := TRUE;
          if (ahbsi.hmbsel(AHB_IO_BAR_INDEX-4) = '1') then -- I/O BAR
            v.access_range := IO;
            v.iooffset := to_integer(unsigned(ahbsi.haddr(7 downto 0)));
            v.adac_we := "0000";
          elsif (ahbsi.hmbsel(AHB_MEM_BAR_INDEX-4) = '1') then -- Memory BAR
            v.access_range := MEM;
            if    (unsigned(ahbsi.haddr(19 downto 16)) = to_unsigned(ADC0_MEM_OFFSET, 4)) then v.adac_we := "1000";
            elsif (unsigned(ahbsi.haddr(19 downto 16)) = to_unsigned(ADC1_MEM_OFFSET, 4)) then v.adac_we := "0100";
            elsif (unsigned(ahbsi.haddr(19 downto 16)) = to_unsigned(DAC0_MEM_OFFSET, 4)) then v.adac_we := "0010";
            elsif (unsigned(ahbsi.haddr(19 downto 16)) = to_unsigned(DAC1_MEM_OFFSET, 4)) then v.adac_we := "0001";
            end if;
          else -- Not any defined BAR
            null; -- We don't need to drive the 4 adc0access, adc1access, dac0access & dac1access signals
                  -- in case of any other BAR, because: if any other BAR, HSEL should not be asserted by the AHB arbiter.
          end if;
        else -- read
          v.write := FALSE;
          v.adac_we := "0000";
          if (ahbsi.hmbsel(AHB_IO_BAR_INDEX-4) = '1') then -- Decoding Bank Address Space of AHB I/Os
              -- case unsigned(ahbsi.haddr(7 downto 0))
              --  when others => null;
              --end case;
            v.access_range := IO;
            -- RF Control Register File
            if    (unsigned(ahbsi.haddr(7 downto 0)) = to_unsigned(WDNS_RFCTL0, 8)) then v.hrdata := r.rf0;
            elsif (unsigned(ahbsi.haddr(7 downto 0)) = to_unsigned(WDNS_RFCTL1, 8)) then v.hrdata := r.rf1;
            elsif (unsigned(ahbsi.haddr(7 downto 0)) = to_unsigned(IDROMEL_RFCTL, 8)) then
              v.hrdata     := (others => '0');
              v.hrdata(4)  := r.rf_swt_lo_f;
              v.hrdata(5)  := r.rf_swt_rx_tx;
              v.hrdata(6)  := r.rf_reset_lfsw;
              v.hrdata(7)  := r.rf_swt_tx3_1;
              v.hrdata(8)  := r.rf_pwrdwn_adf;
              v.hrdata(9)  := r.rf_swt_tx3_2;
              v.hrdata(13) := r.rf_swt_tx3_3;
              v.hrdata(14) := r.rf_swt_lo1;
              v.hrdata(15) := r.rf_swt_lo2;
            -- ADAC Control Register File
            elsif (unsigned(ahbsi.haddr(7 downto 0)) = to_unsigned(ADAC_CONFIG, 8)) then
              v.hrdata := "00000"  -- bits 31 downto 27 (unused, read as 0)
                        & bool_to_std(r.prefix_remove)                                       -- bit  26
                        & r.slot_alloc                                                       -- bits 25 downto 24
                        & "000000000000"                                                     -- bits 23 downto 12 (unused, read as 0)
                        & std_logic_vector(to_unsigned(r.cyclic_prefix_length, 8))           -- bits 11 downto 4
                        & std_logic_vector(to_unsigned(r.log2_symbol_size, 4));              -- bits  3 downto 0
            elsif (unsigned(ahbsi.haddr(7 downto 0)) = to_unsigned(FRAME_LEN, 8)) then
              v.hrdata := "00000000"                                         -- bits 31 downto 24 (unused, read as 0)
                        & std_logic_vector(to_unsigned(r.frame_length, 24)); -- bits 23 downto 0
            elsif (unsigned(ahbsi.haddr(7 downto 0)) = to_unsigned(FRAME_STRT, 8)) then
              v.hrdata := "00000000"                                         -- bits 31 downto 24 (unused, read as 0)
                        & std_logic_vector(to_unsigned(r.frame_start,  24)); -- bits 23 downto 0
            elsif (unsigned(ahbsi.haddr(7 downto 0)) = to_unsigned(SWITCH_OFFSET, 8)) then
              v.hrdata := "000000000000000000000000"                         -- bits 31 downto 8 (unused, read as 0)
                        & std_logic_vector(r.switch_offset);                 -- bits  7 downto 0
            elsif (unsigned(ahbsi.haddr(7 downto 0)) = to_unsigned(ADAC_INTR_COUNTER, 8)) then
              v.hrdata := "00000000000000000000000"                          -- bits 31 downto 9 (unused, read as 0)
                        & std_logic_vector(r.adac_intr_counter);             -- bits  8 downto 0
            -- SPI Control Register File
            elsif (unsigned(ahbsi.haddr(7 downto 0)) = to_unsigned(SPI_START, 8)) then
              v.hrdata := "0000000000000000000000000000000"                  -- bits 31 downto 1 (unused, read as 0)
                        & std_logic(r.spi_active);                           -- bit   0
            elsif (unsigned(ahbsi.haddr(7 downto 0)) = to_unsigned(SPI_CTRL, 8)) then
              --v.hrdata := r.rf_spi_bit_width & "000000" & r.rf_spi_regtype & "0000000" & r.rf_spi_active;
              v.hrdata := (others => '0');
              v.hrdata(16+SPI_LOG2_MAX_DELAY-1 downto 16) := std_logic_vector(to_unsigned(r.rf_spi_log2_bit_width, SPI_LOG2_MAX_DELAY));
              v.hrdata(9 downto 8) := r.rf_spi_regtype;
              v.hrdata(0) := r.rf_spi_active;
            end if;
          elsif (ahbsi.hmbsel(AHB_MEM_BAR_INDEX-4) = '1') then  -- Decoding Bank Address Space of AHB Memory
            v.access_range := MEM;
            if    (unsigned(ahbsi.haddr(19 downto 16)) = to_unsigned(ADC0_MEM_OFFSET, 4)) then v.hrdatamux := ADC0;
            elsif (unsigned(ahbsi.haddr(19 downto 16)) = to_unsigned(ADC1_MEM_OFFSET, 4)) then v.hrdatamux := ADC1;
            elsif (unsigned(ahbsi.haddr(19 downto 16)) = to_unsigned(DAC0_MEM_OFFSET, 4)) then v.hrdatamux := DAC0;
            elsif (unsigned(ahbsi.haddr(19 downto 16)) = to_unsigned(DAC1_MEM_OFFSET, 4)) then v.hrdatamux := DAC1;
            end if;
          else -- Not any defined BAR
            null;
          end if;
        end if;
      else -- HREADY and HSEL asserted, but HTRANS_BUSY or HTRANS_IDLE
        v.adac_we := "0000";
        v.write := FALSE;
        -- No need to drive HRESP_OKAY, it is done by default
      end if;
    else -- HREADY or HSEL not asserted
      v.write := FALSE;
      v.adac_we := "0000";
    end if;

    -- =============================
    -- Irq generation on the AHB bus
    -- =============================
    if (r.adac_intr_reset = '1' and r.adac_intr_reset_prevcycle = '0') then
      v.adac_intr_counter := (others => '0');
      xirq(pirq) := '1';
    elsif (r.adac_intr = '1' and r.adac_intr_prevcycle = '0') then
      v.adac_intr_counter := r.adac_intr_counter + 1;
      xirq(pirq) := '1';
    end if;
    -- drive output
    ahbso.hirq   <= xirq;

    -- =============
    -- SPI interface
    -- =============
    -- Triggering start of SPI output transfers
    if (r.spi_active = '0' and v.spi_active = '1') then
    --if v_spi_start then
      v.spi_reg_len := 0;
      v.spi_enables := r.spi_enables_ahb;
      v.spi_sizes := r.spi_sizes_ahb;
      v.spi_regfile := r.spi_regfile_ahb;
      if (r.spi_sizes_ahb = "000000") then
        v.spi_xfer_len := 16;
      else
        v.spi_xfer_len := 20;
      end if;
      v.spi_state := SPI_WRITE;
    end if;

    -- SPI transfers mode
    if (r.spi_state = SPI_WRITE) then
      if (r.spi_clk_cnt = "1111") then
        -- shift register
        for i in 0 to 5 loop
          v.spi_regfile(i) := '0' & r.spi_regfile(i)(23 downto 1);
        end loop;
        v.spi_reg_len := r.spi_reg_len + 1;
        if (v.spi_reg_len = r.spi_xfer_len) then
          v.spi_state := SPI_IDLE;
          v.spi_active := '0';
        end if;
      end if;
      v.spi_enable_spiclk := TRUE;
    else
      v.spi_clk_cnt := (others => '0');
      v.spi_enable_spiclk := FALSE;
    end if;
    
    -- SPI clock generation
    if r.spi_enable_spiclk then
      v.spi_clk_cnt := r.spi_clk_cnt + 1;
      v.spi_clk := r.spi_clk_cnt(3);
    else
      v.spi_clk := '0';
    end if;

    -- =============
    -- SPI interface (new Idromel RF)
    -- =============
    if (r.rf_spi_active = '0' and v.rf_spi_active = '1') then
      -- Which register to latch
      case v.rf_spi_regtype is
        when SPI_ID_ADF4108 =>
          for i in 0 to 21 loop v.rf_spi_txbits(i)  := r.rf_spi_adf4108_func(23-i); end loop;
                                v.rf_spi_txbits(22) := '1';
                                v.rf_spi_txbits(23) := '1'; -- because INIT reg has address "11"
          v.rf_spi_le(2) := '0';
        when SPI_ID_LFSW => 
          -- nothing to do for r.rf_spi_txbits (the reg values are already stored from AHB access)
          v.rf_spi_le(1) := '0';
        when SPI_ID_SETTX => null;
          -- same remark as for LFSW
          v.rf_spi_le(3) := '0';
        when SPI_ID_SETRX => null;
          -- same remark as for LFSW
          v.rf_spi_le(4) := '0';
        when others => null;
      end case;
      v.rf_spi_bit_ndx := (others => '0');
      v.rf_spi_clklevel := '0';
      v.rf_spi_data2data_cnten := TRUE;
    end if;

    if (r.rf_spi_active = '1') then v.rf_spi_cnt := r.rf_spi_cnt + 1; end if;

    if r.rf_spi_active = '1' then
      -- Describing one BIT transfer
              --if (r.rf_spi_cnt(SPI_DEFAULT_LOG2_DELAY-1) = '1' and v.rf_spi_cnt(SPI_DEFAULT_LOG2_DELAY-1) = '0') then -- counter overflow
      if (r.rf_spi_cnt(natural(r.rf_spi_log2_bit_width)) = '1' and v.rf_spi_cnt(natural(r.rf_spi_log2_bit_width)) = '0') then -- counter overflow
        if (r.rf_spi_data2data_cnten) then -- end of 'len-2-data' OR 'data-2-data' OR 'data-2-len' phase
          if (r.rf_spi_adf_turnaround) then
            v.rf_spi_adf_turnaround := FALSE;
            v.rf_spi_le(2) := '0';
            v.rf_spi_bit_ndx := (others => '0');
          else
            --v.rf_spi_data2data_cnten := FALSE;
            for i in 70 downto 0 loop
              v.rf_spi_txbits(i) := r.rf_spi_txbits(i+1); -- shift register
            end loop;
            case r.rf_spi_regtype is
              when SPI_ID_ADF4108 => 
                if (r.rf_spi_adf_step = "00" and r.rf_spi_bit_ndx = to_unsigned(24, log2(SPI_MAX_BITS_POW2))) then
                  for i in 0 to 21 loop
                    v.rf_spi_txbits(i)  := r.rf_spi_adf4108_func(23-i); -- reload FUNC reg into rf_spi_txbits
                  end loop;
                  v.rf_spi_txbits(22) := '1';
                  v.rf_spi_txbits(23) := '0'; -- because FUNC reg has address "10"
                  v.rf_spi_adf_turnaround := TRUE;
                  v.rf_spi_le(2) := '1';
                  v.rf_spi_adf_step := "01";
                elsif (r.rf_spi_adf_step = "01" and r.rf_spi_bit_ndx = to_unsigned(24, log2(SPI_MAX_BITS_POW2))) then
                  for i in 0 to 21 loop
                    v.rf_spi_txbits(i)  := r.rf_spi_adf4108_ncnt(23-i); -- load NCNT reg into rf_spi_txbits
                  end loop;
                  v.rf_spi_txbits(22) := '0';
                  v.rf_spi_txbits(23) := '1'; -- because NCNT reg has address "01"
                  v.rf_spi_adf_turnaround := TRUE;
                  v.rf_spi_le(2) := '1';
                  v.rf_spi_adf_step := "10";
                elsif (r.rf_spi_adf_step = "10" and r.rf_spi_bit_ndx = to_unsigned(24, log2(SPI_MAX_BITS_POW2))) then
                  for i in 0 to 21 loop
                    v.rf_spi_txbits(i)  := r.rf_spi_adf4108_rcnt(23-i); -- reload RCNT reg into rf_spi_txbits
                  end loop;
                  v.rf_spi_txbits(22) := '0';
                  v.rf_spi_txbits(23) := '0'; -- because RCNT reg has address "00"
                  v.rf_spi_adf_turnaround := TRUE;
                  v.rf_spi_le(2) := '1';
                  v.rf_spi_adf_step := "11";
                elsif (r.rf_spi_adf_step = "11" and r.rf_spi_bit_ndx = to_unsigned(24, log2(SPI_MAX_BITS_POW2))) then
                  v.rf_spi_le(2) := '1';
                  v.rf_spi_data2data_cnten := FALSE;
                  v.rf_spi_adf_step := "00"; -- reset to INIT reg (for next transfer!)
                  v.rf_spi_active := '0'; -- end of SPI transfer for ADF4108 (complete 4 x LEs, each one 24 bits long)
                else
                  v.rf_spi_data := r.rf_spi_txbits(0);
                  v.rf_spi_bit_ndx := r.rf_spi_bit_ndx + 1;
                  v.rf_spi_data2data_cnten := FALSE;
                  v.rf_spi_data2clk_cnten := TRUE;
                end if;
              when SPI_ID_LFSW =>
                if (r.rf_spi_bit_ndx = to_unsigned(72, log2(SPI_MAX_BITS_POW2))) then
                  v.rf_spi_le(1) := '1';
                  v.rf_spi_data2data_cnten := FALSE;
                  v.rf_spi_active := '0'; -- end of SPI transfer for LFSW (one complete LE, 72 bits long)
                else
                  v.rf_spi_data := r.rf_spi_txbits(0);
                  v.rf_spi_bit_ndx := r.rf_spi_bit_ndx + 1;
                  v.rf_spi_data2data_cnten := FALSE;
                  v.rf_spi_data2clk_cnten := TRUE;
                end if;
              when SPI_ID_SETTX =>
                if (r.rf_spi_bit_ndx = to_unsigned(15, log2(SPI_MAX_BITS_POW2))) then
                  v.rf_spi_le(3) := '1';
                  v.rf_spi_data2data_cnten := FALSE;
                  v.rf_spi_active := '0'; -- end of SPI transfer for SETTX (one complete LE, 22 bits long)
                else
                  v.rf_spi_data := r.rf_spi_txbits(0);
                  v.rf_spi_bit_ndx := r.rf_spi_bit_ndx + 1;
                  v.rf_spi_data2data_cnten := FALSE;
                  v.rf_spi_data2clk_cnten := TRUE;
                end if;
              when SPI_ID_SETRX =>
                if (r.rf_spi_bit_ndx = to_unsigned(15, log2(SPI_MAX_BITS_POW2))) then
                  v.rf_spi_le(4) := '1';
                  v.rf_spi_data2data_cnten := FALSE;
                  v.rf_spi_active := '0'; -- end of SPI transfer for SETRX (one complete LE, 22 bits long)
                else
                  v.rf_spi_data := r.rf_spi_txbits(0);
                  v.rf_spi_bit_ndx := r.rf_spi_bit_ndx + 1;
                  v.rf_spi_data2data_cnten := FALSE;
                  v.rf_spi_data2clk_cnten := TRUE;
                end if;
              when others => null;
            end case;
          end if; -- adf_turnaround
        end if; -- data2data_cnten
        if (r.rf_spi_data2clk_cnten) then -- end of data-2-clk phase
          v.rf_spi_data2clk_cnten := FALSE;
          if (r.rf_spi_clklevel = '0') then
            v.rf_spi_clk := '1';
            v.rf_spi_clklevel := '1';
            v.rf_spi_clkwidth_cnten := TRUE;
          else
            v.rf_spi_clklevel := '0';
            v.rf_spi_data := '0'; -- forcing data to 0 between each bit
            v.rf_spi_data2data_cnten := TRUE;
          end if;
        end if;
        if (r.rf_spi_clkwidth_cnten) then -- end of clock-width phase
          v.rf_spi_clkwidth_cnten := FALSE;
          v.rf_spi_data2clk_cnten := TRUE;
          v.rf_spi_clk := '0';
        end if;
      end if; -- counter overflow
    end if; -- spi_active

    -- ==================
    -- Asynchronous reset (for AHB clock domain)
    -- ==================
    if (rstn = '0') then
      v.rf0 := (others => '0'); v.rf1 := (others => '0');
      v.rf0(10) := '1'; -- Latche-Enables of Frequency Synthesizers
      v.rf0(17) := '1'; --   must be reset in high state.
      v.write := FALSE;
      --TODO: Reset all registers
      --v.hready := '1';
      --v.hresp := HRESP_OKAY;
      v.hrdata := (others => '0');
      --v.hsize := (others => '0');
      v.adac_addr := (others => '0');
      v.write := FALSE;
      v.iooffset := 0;
      v.access_range := IO;
      v.adac_intr := '0';
      v.adac_intr_resync(0) := '0';
      v.adac_intr_resync(1) := '0';
      v.adac_intr_prevcycle := '0';
      v.adac_intr_reset := '0';
      v.adac_intr_reset_resync(0) := '0';
      v.adac_intr_reset_resync(1) := '0';
      v.adac_intr_reset_prevcycle := '0';
      v.adac_intr_counter := (others => '0');
      -- ADAC Control Register File
      v.soft_reset := '1';
      v.slot_alloc := "11";
      v.prefix_remove := FALSE;
      v.log2_symbol_size := 8;
      v.cyclic_prefix_length := 63;
      v.frame_length := 20479;
      v.frame_start := 0;
      v.switch_offset := to_unsigned(19, 8);
      -- ADAC Memories Interface
      v.adac_we := (others => '0');
      v.hrdatamux := ADC0;
      -- SPI interface
      for i in 0 to 5 loop
        v.spi_regfile_ahb(i) := (others => '0');
        v.spi_regfile(i) := (others => '0');
      end loop;
      v.spi_sizes_ahb := (others => '0');
      v.spi_sizes := (others => '0');
      v.spi_enables_ahb := (others => '0');
      v.spi_enables := (others => '0');
      v.spi_active := '0';
      v.spi_state := SPI_IDLE;
      v.spi_reg_len := 0;
      v.spi_xfer_len := 0;
      v.spi_clk := '0';
      v.spi_enable_spiclk := FALSE;
      v.spi_clk_cnt := (others => '0');
      -- SPI bus of Idromel RF
      v.rf_spi_le           := (others => '1');
      v.rf_spi_clk          := '0';
      v.rf_spi_data         := '0';
      v.rf_spi_cnt  := (others => '0');
      v.rf_spi_data2clk_cnten  := FALSE;
      v.rf_spi_data2data_cnten := FALSE;
      v.rf_spi_clkwidth_cnten  := FALSE;
      --v.rf_spi_regtype := ; no need to initialize this register
      v.rf_spi_adf_step := "00";
      v.rf_spi_active   := '0';
      --v.spi_adf4108_init  := ; no need to initialize this register
      --v.spi_adf4108_func  := ; no need to initialize this register
      --v.spi_adf4108_ncnt  := ; no need to initialize this register
      --v.spi_adf4108_rcnt  := ; no need to initialize this register
      --v.rf_spi_txbits     := ; no need to initialize this register
      --v.rf_spi_bit_ndx    := ; no need to initialize this register
      --v.rf_spi_clklevel   := ; no need to initialize this register
      --v.rf_spi_bit_width  := ; no need to initialize this regsiter (so software MUST initialize it).
      --Switches of Idromel RF
      v.rf_swt_lo_f   := '0';
      v.rf_swt_rx_tx  := '0';
      v.rf_reset_lfsw := '0';
      v.rf_swt_tx3_1  := '0';
      v.rf_pwrdwn_adf := '0';
      v.rf_swt_tx3_2  := '0';
      v.rf_swt_tx3_3  := '0';
      v.rf_swt_lo1    := '0';
      v.rf_swt_lo2    := '0';
    end if;

    rin <= v;

  end process comb_ahb;

  -- Registers of the AHB clock domain
  reg_ahb : process(clk)
  begin if clk'event and clk = '1' then r <= rin; end if; end process;

  -- =================                  ===========
  -- ADAC clock domain (clock signal is clk_7_68mhz)
  -- =================                  ===========
  comb_clk_7_68mhz : process(rad, rstn, r.log2_symbol_size,       -- These are the
                                        r.cyclic_prefix_length,   -- seven registers
                                        r.frame_length,           -- crossing from
                                        r.frame_start,            -- AHB clock
                                        r.switch_offset,          -- domain to
                                        r.prefix_remove,          -- ADAC clock
                                        r.slot_alloc,             -- domain
                                        r.soft_reset)             -- ...
    variable v : regs_clk_7_68mhz;
    variable v_symbol_size : unsigned(8 downto 0);
  begin
    v := rad;
    v_symbol_size := shift_left(ONE_AS_AN_9BITS_UNSIGNED, rad.log2_symbol_size);

    -- Crossing AHB clock domain to ADAC clock domain
    -- now WITH resynchronization registers (2 layers).
    --   First Layer...
    v.log2_symbol_size_resync(1) := r.log2_symbol_size;
    v.cyclic_prefix_length_resync(1) := r.cyclic_prefix_length;
    v.frame_length_resync(1) := r.frame_length;
    v.frame_start_resync(1) := r.frame_start;
    v.switch_offset_resync(1) := r.switch_offset;
    v.prefix_remove_resync(1) := r.prefix_remove;
    v.slot_alloc_resync(1) := r.slot_alloc;
    v.soft_reset_resync(1) := r.soft_reset;
    --   Second Layer...
    v.log2_symbol_size_resync(0) := rad.log2_symbol_size_resync(1);
    v.cyclic_prefix_length_resync(0) := rad.cyclic_prefix_length_resync(1);
    v.frame_length_resync(0) := rad.frame_length_resync(1);
    v.frame_start_resync(0) := rad.frame_start_resync(1);
    v.switch_offset_resync(0) := rad.switch_offset_resync(1);
    v.prefix_remove_resync(0) := rad.prefix_remove_resync(1);
    v.slot_alloc_resync(0) := rad.slot_alloc_resync(1);
    v.soft_reset_resync(0) := rad.soft_reset_resync(1);
    --   ADAC resynchronizED registers (these ones are the ONLY ones
    --   used in the ADAC clock domain combinational part).
    v.log2_symbol_size := rad.log2_symbol_size_resync(0);
    v.cyclic_prefix_length := rad.cyclic_prefix_length_resync(0);
    v.frame_length := rad.frame_length_resync(0);
    v.frame_start := rad.frame_start_resync(0);
    v.switch_offset := rad.switch_offset_resync(0);
    v.prefix_remove := rad.prefix_remove_resync(0);
    v.slot_alloc := rad.slot_alloc_resync(0);
    v.soft_reset := rad.soft_reset_resync(0);

    -- By default, no intr transmitted to AHB clock domain
    v.adac_intr := '0';
    v.adac_intr_reset := '0';

    if (rad.log2_symbol_size /= to_unsigned(ZERO_AS_AN_INTEGER, 4)) then
      v.dac_sample_counter := rad.dac_sample_counter + 1;
      v.adc_sample_counter := rad.adc_sample_counter + 1;
      v.adc_sample_counter2 := rad.adc_sample_counter2 + 1;
      -- Synchronization reset of counters
      if (rad.adc_sample_counter2 = (to_unsigned(rad.frame_start,24) - resize(rad.switch_offset, 24) - ONE_AS_AN_INTEGER)) then --TODO what happens if switch_offset is 0 ?
        v.dac_sample_counter := (others => '0');
        v.dac_sample_counter_offset := (others => '0');
        v.doing_prefix := FALSE;            
        --v.adac_intr := '1';
        --v.adac_intr_counter := (others => '0');
        v.adac_intr_reset := '1';
      elsif (rad.adc_sample_counter2 = rad.frame_start) then
        v.adc_sample_counter := (others => '0');
        v.counter := (others => '0');
        v.in_prefix := TRUE;
      else
        -- Adjust ADC/DAC counters
        -----------------------------------------------------------------------
        -- DAC addressing
        -----------------------------------------------------------------------
        if (rad.dac_sample_counter = (v_symbol_size - ONE_AS_AN_9BITS_UNSIGNED)) then
          v.dac_sample_counter := (others => '0');
          v.doing_prefix := TRUE;
          --v.adac_intr := '0'; -- No use, this is done by default now
        elsif ((rad.dac_sample_counter = rad.cyclic_prefix_length) and (rad.doing_prefix = TRUE)) then
          v.doing_prefix := FALSE;
          v.dac_sample_counter := (others => '0');
          v.adac_intr := '1';
          --v.adac_intr_counter := rad.adac_intr_counter + 1;  -- this is done in the AHB clock domain now
          v.dac_sample_counter_offset := rad.dac_sample_counter_offset + v_symbol_size;
        --else
        --  v.adac_intr := '0';  -- No use, this is done by default now
        end if; -- dac_sample_counter
        -----------------------------------------------------------------------
        -- ADC addressing
        -----------------------------------------------------------------------
        if (rad.in_prefix = FALSE) then
          v.counter := rad.counter + 1;
        end if;
        if (rad.counter = NUMBER_1023_AS_A_10BITS_UNSIGNED) then
          v.counter := (others =>'0');
        end if;

        if (rad.adc_sample_counter = rad.cyclic_prefix_length) then
          v.in_prefix := FALSE;
        elsif ((rad.adc_sample_counter - rad.cyclic_prefix_length) = v_symbol_size) then
          v.in_prefix := TRUE;
          v.adc_sample_counter := (others => '0');
        end if; -- adc_sample_counter
      end if; -- Adjust ADC/DAC Counters

      if (rad.adc_sample_counter2 = rad.frame_length) then
        v.adc_sample_counter2 := (others => '0');
      end if;
    else
      -- LOG2 symbol size
      v.dac_sample_counter := (others =>'0');
      v.adc_sample_counter := (others => '0');
      v.adc_sample_counter2 := (others => '0');
    end if;

    -------------------------------------------------------------------------------
    -- Switching logic
    -------------------------------------------------------------------------------
    if (rad.dac_sample_counter_offset(8) = '0') then -- RX/TX Mode for Samples 
      v.mode_int := rad.slot_alloc(0);  -- bit 24
    else
      v.mode_int := rad.slot_alloc(1);  -- bit 25
    end if;
    if (rad.dac_sample_counter(7 downto 0) = rad.switch_offset) then -- RX/TX Mode for switch (TX)
      if (rad.dac_sample_counter_offset(8) = '0' ) then
        v.mode := rad.slot_alloc(0);  -- bit 24
      else
        v.mode := rad.slot_alloc(1);  -- bit 25
      end if;
    end if;

    -- Asynchronous reset (for ADAC clock domain)
    if (rstn = '0' or rad.soft_reset = '1') then
      --v.log2_symbol_size := 0; -- KMKTODO
      v.dac_sample_counter := (others => '0');
      v.adc_sample_counter := (others => '0');
      v.adc_sample_counter2 := (others => '0');
      v.dac_sample_counter_offset := (others => '0');
      v.counter := (others => '0');
      v.mode := '1';
      v.adac_intr := '0';
      v.adac_intr_reset := '0';
      --v.adac_intr_counter := (others => '0');
      v.doing_prefix := FALSE;
      v.in_prefix := TRUE;
      -- Resynchronization registers
      v.log2_symbol_size_resync(1) := LOG2_SYMBOL_SIZE_RESET_VALUE;
      v.cyclic_prefix_length_resync(1) := CYCLIC_PREFIX_LENGTH_RESET_VALUE;
      v.frame_length_resync(1) := FRAME_LENGTH_RESET_VALUE;
      v.frame_start_resync(1) := FRAME_START_RESET_VALUE;
      v.switch_offset_resync(1) := SWITCH_OFFSET_RESET_VALUE;
      v.prefix_remove_resync(1) := PREFIX_REMOVE_RESET_VALUE;
      v.slot_alloc_resync(1) := SLOT_ALLOC_RESET_VALUE;
      --v.soft_reset_resync(1) := '1';
      --
      v.log2_symbol_size_resync(0) := LOG2_SYMBOL_SIZE_RESET_VALUE;
      v.cyclic_prefix_length_resync(0) := CYCLIC_PREFIX_LENGTH_RESET_VALUE;
      v.frame_length_resync(0) := FRAME_LENGTH_RESET_VALUE;
      v.frame_start_resync(0) := FRAME_START_RESET_VALUE;
      v.switch_offset_resync(0) := SWITCH_OFFSET_RESET_VALUE;
      v.prefix_remove_resync(0) := PREFIX_REMOVE_RESET_VALUE;
      v.slot_alloc_resync(0) := SLOT_ALLOC_RESET_VALUE;
      --v.soft_reset_resync(0) := '1';
      --
      v.log2_symbol_size := LOG2_SYMBOL_SIZE_RESET_VALUE;
      v.cyclic_prefix_length := CYCLIC_PREFIX_LENGTH_RESET_VALUE;
      v.frame_length := FRAME_LENGTH_RESET_VALUE;
      v.frame_start := FRAME_START_RESET_VALUE;
      v.switch_offset := SWITCH_OFFSET_RESET_VALUE;
      v.prefix_remove := PREFIX_REMOVE_RESET_VALUE;
      v.slot_alloc := SLOT_ALLOC_RESET_VALUE;
      --v.soft_reset := '1';
    end if;

    if (rstn = '0') then
      v.soft_reset_resync(1) := '1';
      v.soft_reset_resync(0) := '1';
      v.soft_reset := '1';
    end if;

    -- Edge-signals of combinational logic
    radin <= v;

  end process comb_clk_7_68mhz;

  -- This is (adc_counter) the address that is actually seen by ADC converters (by both of them).
  adc_counter <= std_logic_vector(resize(rad.counter, log2(adc0size) + 8)) when (rad.prefix_remove) -- Cyclic prefix removal
            else std_logic_vector(rad.adc_sample_counter2(log2(adc0size) + 7 downto 0));            -- No cyclic prefix removal

  -- This is (dac_counter) the address that is actually seen by DAC converters (by both of them).
  dac_counter <= std_logic_vector(resize(rad.dac_sample_counter_offset + rad.dac_sample_counter, log2(dac0size) + 8));

  -- Registers of the clk_7_68mhz clock domain
  reg_clk_7_68mhz : process(clk_7_68mhz)
  begin if clk_7_68mhz'event and clk_7_68mhz = '1' then rad <= radin; end if; end process reg_clk_7_68mhz;

  -- =============
  -- drive outputs
  -- =============
  ahbso.hconfig <= hconfig;
  ahbso.hindex  <= slvndx;
  ahbso.hsplit  <= (others => '0');
  ahbso.hcache  <= '0';
  ahbso.hready  <= '1'; -- r.hready;
  --ahbso.hirq <= (others => '0');
  ahbso.hrdata  <=  adc0_dataout_portA when (r.access_range = MEM) and (r.hrdatamux = ADC0)
               else adc1_dataout_portA when (r.access_range = MEM) and (r.hrdatamux = ADC1)
               else dac0_dataout_portA when (r.access_range = MEM) and (r.hrdatamux = DAC0)
               else dac1_dataout_portA when (r.access_range = MEM) and (r.hrdatamux = DAC1)
               else r.hrdata;
  ahbso.hresp <= HRESP_OKAY; -- r.hresp;
  -- Widens native RF interface
  wmodemo.rf0 <= r.rf0;
  wmodemo.rf1 <= r.rf1;
  wmodemo.swa <= rad.mode;
  wmodemo.swb <= not rad.mode;
  wmodemo.tx_sync_out <= clk_7_68mhz; --when rad.mode_int = '0' else '0';
    --wmodemo.tx_sync_out <= rda2x.tx_sync_out_i when rda2x.mode_int = '0' else '0'; -- clk_7_68mhz; ...is this a bug ???
    --not_rad_mode_int <= not rad.mode_int;
    -- bufgce_tx_sync: BUFGCE port map(I => clk_7_68mhz, O => wmodemo.tx_sync_out, CE => not_rad_mode_int);
  -- Idromel RF interface
  wmodemo.rf_spi_le     <= r.rf_spi_le;
  wmodemo.rf_spi_clk    <= r.rf_spi_clk;
  wmodemo.rf_spi_data   <= r.rf_spi_data;
  wmodemo.rf_swt_lo_f   <= r.rf_swt_lo_f;
  wmodemo.rf_swt_rx_tx  <= r.rf_swt_rx_tx;
  wmodemo.rf_reset_lfsw <= r.rf_reset_lfsw;
  wmodemo.rf_swt_tx3_1  <= r.rf_swt_tx3_1;
  wmodemo.rf_pwrdwn_adf <= r.rf_pwrdwn_adf;
  wmodemo.rf_swt_tx3_2  <= r.rf_swt_tx3_2;
  wmodemo.rf_swt_tx3_3  <= r.rf_swt_tx3_3;
  wmodemo.rf_swt_lo1    <= r.rf_swt_lo1;
  wmodemo.rf_swt_lo2    <= r.rf_swt_lo2;
  dac_mux_proc0 : process(clk_7_68mhz) is
  begin
    if (clk_7_68mhz = '0') then
      sample_I_notQ <= TRUE;
    else
      sample_I_notQ <= FALSE;
    end if;
  end process dac_mux_proc0;
  dac_mux_proc : process(sample_I_notQ, rad.mode_int, dac0_dataout_portB)
  begin
    if (rad.mode_int = '0') then
      if (sample_I_notQ) then
        -- Send I Component on low pulse of clk_7_68mhz
        wmodemo.dac0data(15 downto 8) <= dac0_dataout_portB( 7 downto 0 );
        wmodemo.dac1data(15 downto 8) <= dac0_dataout_portB(23 downto 16);
      else
        -- Send Q Component on high pulse of clk_7_68mhz
        wmodemo.dac0data(15 downto 8) <= dac0_dataout_portB(15 downto 8 );
        wmodemo.dac1data(15 downto 8) <= dac0_dataout_portB(31 downto 24);
      end if;
    else
      wmodemo.dac0data(15 downto 8) <= (others => '0');
      wmodemo.dac1data(15 downto 8) <= (others => '0');
    end if;
  end process dac_mux_proc;
  -- ... and this drives the remaining bits 7 downto 0
  wmodemo.dac0data(7 downto 0) <= (others => '0');
  wmodemo.dac1data(7 downto 0) <= (others => '0');
  -- SPI output
  spi_clocks: for i in 0 to 5 generate
    wmodemo.wdns_spi_clk(i) <= r.spi_clk and r.spi_enables(i);
  end generate;
  wmodemo_spi_data_gen: for i in 0 to 5 generate
    wmodemo.wdns_spi_data(i) <= r.spi_regfile(i)(0);
  end generate;
  -- For generation of the SPI Latch-Enable signals, we can use the condition r.spi_state = SPI_WRITE,
  -- for the register r.spi_state is only made of 1-bit.
      --wmodemo.wdns_spi_en <=      not r.spi_configreg( 7 downto 0) when ((r.spi_reg_len <  16) and (r.spi_state = SPI_WRITE))
      --                       else not r.spi_configreg(15 downto 8) when ((r.spi_reg_len >= 16) and (r.spi_state = SPI_WRITE))
      --                       else (others => '1');
  wmodemo_spi_en_gen: process(r.spi_sizes, r.spi_enables, r.spi_reg_len, r.spi_state)
  begin
    if (r.spi_state = SPI_WRITE) then
      if (r.spi_reg_len < 16) then
        wmodemo.wdns_spi_en <= not r.spi_enables;
      else
        wmodemo.wdns_spi_en <= not r.spi_sizes;
      end if;
    else
      wmodemo.wdns_spi_en <= (others => '1');
    end if;
  end process wmodemo_spi_en_gen;
  -- (By the way, maybe we could just use registered signals both for spi_regfile & spi_configreg, rather
  -- than combinational ones - to avoid setup/hold violations...)
  adc0data(adc0res - 1  downto 0 ) <= wmodemi.adc0data(adc0res - 1  downto 0 );
  adc0data(adc0res + 15 downto 16) <= wmodemi.adc0data(adc0res + 15 downto 16);
  adc0data_sign_ext: for i in adc0res to 15 generate
    adc0data(i) <= wmodemi.adc0data(adc0res - 1);
    adc0data(i+16) <= wmodemi.adc0data(adc0res - 1 + 16);
  end generate;
  adc1data(adc1res - 1  downto 0)  <= wmodemi.adc1data(adc1res - 1  downto 0);
  adc1data(adc1res + 15 downto 16) <= wmodemi.adc1data(adc1res + 15 downto 16);
  adc1data_sign_ext: for i in adc1res to 15 generate
    adc1data(i) <= wmodemi.adc1data(adc1res - 1);
    adc1data(i+16) <= wmodemi.adc1data(adc1res - 1 + 16);
  end generate;

  adac_addr <= r.adac_addr                              when r.write  -- For write, address must be latched one cycle after it is driven on the AHB bus
          else ahbsi.haddr(MAX_ADAC_ADDR + 1 downto 2);               -- while for read, it must be latched in the cycle (on-the-fly),

  -- =============================
  -- ADAC RAM blocks instanciation
  -- =============================
  -- For each one of the 4 following RAMs: port A is the port accessed by bus,
  --                                       port B is the port accessed by AD/DA converter
  -- Generic view of the 4 following instanciations:
  --    port map(clk1,        address1,                                 datain1,                           dataout1,                en1, write1,
  --            (clk2,        address2,                                 datain2,                           dataout2,                en2, write2      );
  dpram_adc0 : syncram_dp
    generic map (memtech, log2(adc0size) + 8, 32)
    port    map (clk,         adac_addr(log2(adc0size) + 7 downto 0),   ahbsi.hwdata,                      adc0_dataout_portA,      '1', r.adac_we(3),  -- port A
                 clk_7_68mhz, adc_counter(log2(adc0size) + 7 downto 0), adc0data,                          adc0_dataout_portB_void, '1', '1'         ); -- port B
  dpram_adc1 : syncram_dp
    generic map (memtech, log2(adc1size) + 8, 32)
    port    map (clk,         adac_addr(log2(adc1size) + 7 downto 0),   ahbsi.hwdata,                      adc1_dataout_portA,      '1', r.adac_we(2),  -- port A
                 clk_7_68mhz, adc_counter(log2(adc1size) + 7 downto 0), adc1data,                          adc1_dataout_portB_void, '1', '1'         ); -- port B
  dpram_dac0 : syncram_dp
    generic map (memtech, log2(dac0size) + 8, 32)
    port    map (clk,         adac_addr(log2(dac0size) + 7 downto 0),   ahbsi.hwdata,                      dac0_dataout_portA,      '1', r.adac_we(1),  -- port A
                 clk_7_68mhz, dac_counter(log2(dac0size) + 7 downto 0), ZERO_AS_A_32BITS_STD_LOGIC_VECTOR, dac0_dataout_portB,      '1', '0'         ); -- port B
  dpram_dac1 : syncram_dp
    generic map (memtech, log2(dac1size) + 8, 32)
    port    map (clk,         adac_addr(log2(dac1size) + 7 downto 0),   ahbsi.hwdata,                      dac1_dataout_portA,      '1', r.adac_we(0),  -- port A
                 clk_7_68mhz, dac_counter(log2(dac1size) + 7 downto 0), ZERO_AS_A_32BITS_STD_LOGIC_VECTOR, dac1_dataout_portB,      '1', '0'         ); -- port B

  -- =================
  -- DCM instanciation
  -- =================
  -- Notes: 1. Configuration of dcm0 depends on the release (v0, 1 or 2) of the Cardbus-MIMO-1 board
  --          (for v0 and v1, the FPGA in-clock frequency is equal to 26 MHz,   so we MUL by 13 and DIV by 11 to get the 30.72 MHz to feed AD/DA converters,
  --            and  on v2,   the FPGA in-clock frequency is equal to 19.2 MHz, so we MUL by 16 and DIV by 10 to get the 30.72 MHz to feed AD/DA converters).
  --        2. Knowledge of the board release is made through the generic 'boardvers', which is passed to cmimo1 module at instanciation time.
  --           It should correspond to the parameter CFG_CDBUSMIMO1_BOARD_VERSION from $PT2/devhard/src/configmore.txt, which becomes VHDL same-named
  --           (integer-)constant in $PT2/devhard/configmore.vhd.
  --        3. Let's proceed this way: if board version is v2 (boardvers = 2) then instanciate dcm0 for v2.
  --                                   Otherwise, assume either version 0 or 1.
  -- Configuration of dcm0 for Version 2:
  dcm0_v2: if (boardvers = 2) generate
    dcm0: DCM generic map (CLKDV_DIVIDE => 2.0,
                           CLKFX_DIVIDE => 10,
                           CLKFX_MULTIPLY => 16,
                           CLKIN_PERIOD => 52.08333, -- 1 / 19.2MHz = 52.08333 ns
                           CLKIN_DIVIDE_BY_2 => FALSE,
                           CLKOUT_PHASE_SHIFT => "NONE",
                           CLK_FEEDBACK => "1X",
                           DESKEW_ADJUST => "SYSTEM_SYNCHRONOUS",
                           DFS_FREQUENCY_MODE => "LOW",
                           DLL_FREQUENCY_MODE => "LOW",
                           DUTY_CYCLE_CORRECTION => TRUE,
                           DSS_MODE => "NONE",
                           PHASE_SHIFT => 0,
                           STARTUP_WAIT => FALSE)
                 port map (CLKIN => clk26mhz,
                           CLKFB => clkfb0_in,
                           CLK0 => clkfb0_out,
                           DSSEN => '0',
                           PSCLK => '0',
                           PSEN => '0',
                           PSINCDEC => '0',
                           RST => nrstn,
                           CLKFX => osc1_01_sig);
  end generate;
  -- Configuration of dcm0 for Versions 0 and 1:
  dcm0_v0v1: if (boardvers /= 2) generate
    dcm0: DCM generic map (CLKDV_DIVIDE => 2.0,
                           CLKFX_DIVIDE => 11,
                           CLKFX_MULTIPLY => 13,
                           CLKIN_PERIOD => 38.46153, -- 1 / 26MHz = 38.46153 ns
                           CLKIN_DIVIDE_BY_2 => FALSE,
                           CLKOUT_PHASE_SHIFT => "NONE",
                           CLK_FEEDBACK => "1X",
                           DESKEW_ADJUST => "SYSTEM_SYNCHRONOUS",
                           DFS_FREQUENCY_MODE => "LOW",
                           DLL_FREQUENCY_MODE => "LOW",
                           DUTY_CYCLE_CORRECTION => TRUE,
                           DSS_MODE => "NONE",
                           PHASE_SHIFT => 0,
                           STARTUP_WAIT => FALSE)
                 port map (CLKIN => clk26mhz,
                           CLKFB => clkfb0_in,
                           CLK0 => clkfb0_out,
                           DSSEN => '0',
                           PSCLK => '0',
                           PSEN => '0',
                           PSINCDEC => '0',
                           RST => nrstn,
                           CLKFX => osc1_01_sig);
  end generate;
  -- Clock buffer for feedback of dcm0
  bufg_fb0: BUFG port map(I => clkfb0_out, O => clkfb0_in);
  -- Clock buffer for osc1_01 output clock signal (feeds the external AD/DA converters)
  bufg_osc1_01: BUFG port map(I => osc1_01_sig, O => osc1_01);
  
  dcm1: DCM generic map (CLKDV_DIVIDE => 2.0,
                         CLKFX_DIVIDE => 1,
                         CLKFX_MULTIPLY => 2,
                         CLKIN_PERIOD => 65.104167, -- 1 / 15.36MHz = 65.104167 ns
                         CLKIN_DIVIDE_BY_2 => FALSE,
                         CLKOUT_PHASE_SHIFT => "NONE",
                         CLK_FEEDBACK => "1X",
                         DESKEW_ADJUST => "SYSTEM_SYNCHRONOUS",
                         DFS_FREQUENCY_MODE => "LOW",
                         DLL_FREQUENCY_MODE => "LOW",
                         DUTY_CYCLE_CORRECTION => TRUE,
                         DSS_MODE => "NONE",
                         PHASE_SHIFT => 0,
                         STARTUP_WAIT => FALSE)
               port map (CLKIN => clk_15_36mhz,
                         CLKFB => clkfb1_in,
                         CLK0 => clkfb1_out,
                         DSSEN => '0',
                         PSCLK => '0',
                         PSEN => '0',
                         PSINCDEC => '0',
                         RST => nrstn_delayed,
                         CLKDV => clk_7_68mhz);
  -- Clock buffer for feedback of dcm1
  bufg_fb1: BUFG port map(I => clkfb1_out, O => clkfb1_in);

  --  dcm2: DCM generic map (CLKDV_DIVIDE => 2.0,
  --                         CLKFX_DIVIDE => 1,
  --                         CLKFX_MULTIPLY => 2,
  --                         CLKIN_PERIOD => 130.208, -- 1 / 7.68MHz = 130.208 ns
  --                         CLKIN_DIVIDE_BY_2 => FALSE,
  --                         CLKOUT_PHASE_SHIFT => "NONE",
  --                         CLK_FEEDBACK => "1X",
  --                         DESKEW_ADJUST => "SYSTEM_SYNCHRONOUS",
  --                         DFS_FREQUENCY_MODE => "LOW",
  --                         DLL_FREQUENCY_MODE => "LOW",
  --                         DUTY_CYCLE_CORRECTION => TRUE,
  --                         DSS_MODE => "NONE",
  --                         PHASE_SHIFT => 0.0,
  --                         STARTUP_WAIT => FALSE)
  --               port map (CLKIN => clk_7_68mhz,
  --                         CLKFB => clkfb2_in,
  --                         CLK0 => clkfb2_out,
  --                         DSSEN => '0',
  --                         PSCLK => '0',
  --                         PSEN => '0',
  --                         PSINCDEC => '0',
  --                         RST => nrstn_delayed,
  --                         CLK90 => clk_7_68mhz_90deg);
  --  -- Clock buffer for feedback of dcm2
  --  bufg_fb2: BUFG port map(I => clkfb2_out, O => clkfb2_in);

  -- Reset for the DCMs
  nrstn <= not rstn; -- DCMs' reset is active HIGH, so we invert the global RTL reset, which is active LOW.
  nrstn_delayed_gen : process(clk26mhz, nrstn)
  variable v_nrstn_counter : unsigned(LOG2_NCYCLES_DELAY_NRSTN downto 0);
  begin
    if (nrstn = '1') then
      nrstn_counter <= (others => '0');
      nrstn_delayed <= '1';
    elsif (clk26mhz'event and clk26mhz = '1') then
      v_nrstn_counter := nrstn_counter + 1;
      if (nrstn_counter(LOG2_NCYCLES_DELAY_NRSTN) = '0') then
        nrstn_counter <= v_nrstn_counter;
      end if;
      if (v_nrstn_counter(LOG2_NCYCLES_DELAY_NRSTN) = '1') then
        nrstn_delayed <= '0';
      end if;
    end if;
  end process nrstn_delayed_gen;
 
-- pragma translate_off
  bootmsg : report_version 
  generic map (
    "cmimo1" & tost(slvndx) & 
    ": Cardbus-MIMO-1 control, revision " & tost(REVISION) & 
    ", pirq " & tost(pirq));
-- pragma translate_on

end architecture rtl;