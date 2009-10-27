
library ieee;
use ieee.std_logic_1164.all;
library grlib;
use grlib.amba.all;
library techmap;
use techmap.gencomp.all;

package cmimo1lib is

  type wmodem_out_type is record
    -- Widens native RF
    rf0           : std_logic_vector(31 downto 0);
    rf1           : std_logic_vector(31 downto 0);
    swa           : std_ulogic;
    swb           : std_ulogic;
    tx_sync_out   : std_ulogic;
    wdns_spi_clk       : std_logic_vector(5 downto 0);
    wdns_spi_data      : std_logic_vector(5 downto 0);
    wdns_spi_en        : std_logic_vector(5 downto 0);
    -- Idromel new RF
    rf_spi_le     : std_logic_vector(4 downto 1);
    rf_spi_clk    : std_logic;
    rf_spi_data   : std_logic;
    rf_swt_lo_f   : std_logic;
    rf_swt_rx_tx  : std_logic;
    rf_reset_lfsw : std_logic;
    rf_swt_tx3_1  : std_logic;
    rf_pwrdwn_adf : std_logic;
    rf_swt_tx3_2  : std_logic;
    rf_swt_tx3_3  : std_logic;
    rf_swt_lo1    : std_logic;
    rf_swt_lo2    : std_logic;
    -- Output to DAC converters
    dac0data : std_logic_vector(15 downto 0);
    dac1data : std_logic_vector(15 downto 0);
  end record;

  type wmodem_in_type is record
    adc0data: std_logic_vector(31 downto 0);
    adc1data: std_logic_vector(31 downto 0);
  end record;

  component cmimo1 is
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
  end component cmimo1;

end package cmimo1lib;
