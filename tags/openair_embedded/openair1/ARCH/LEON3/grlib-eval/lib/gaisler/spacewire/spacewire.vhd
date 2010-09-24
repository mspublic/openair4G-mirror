library ieee;
use ieee.std_logic_1164.all;
library grlib;
use grlib.amba.all;
library techmap;
use techmap.gencomp.all;

package spacewire is
  type grspw_in_type is record
    d           : std_ulogic;
    s           : std_ulogic;
    tickin      : std_ulogic;
    clkdiv10    : std_logic_vector(7 downto 0);
    rmapen      : std_ulogic;
    dcrstval    : std_logic_vector(9 downto 0);
    timerrstval : std_logic_vector(11 downto 0);
  end record;

  type grspw_out_type is record
    d           : std_ulogic;
    s           : std_ulogic;
    tickout     : std_ulogic;
  end record;

  component grspw is
    generic(
      tech         : integer range 0 to NTECH     := DEFMEMTECH;
      hindex       : integer range 0 to NAHBMST-1 := 0;
      pindex       : integer range 0 to NAPBSLV-1 := 0;
      paddr        : integer range 0 to 16#FFF#   := 0;
      pmask        : integer range 0 to 16#FFF#   := 16#FFF#;
      pirq         : integer range 0 to NAHBIRQ-1 := 0;
      sysfreq      : integer := 10000;
      usegen       : integer range 0 to 1  := 1;
      nsync        : integer range 1 to 2  := 1; 
      rmap         : integer range 0 to 1  := 0;
      rmapcrc      : integer range 0 to 1  := 0;
      fifosize1    : integer range 4 to 32 := 32;
      fifosize2    : integer range 16 to 64 := 64;
      rxclkbuftype : integer range 0 to 2 := 0;
      rxunaligned  : integer range 0 to 1 := 0;
      rmapbufs     : integer range 2 to 8 := 4;
      ft           : integer range 0 to 2 := 0
    );
    port(
      rst        : in  std_ulogic;
      clk        : in  std_ulogic;
      txclk      : in  std_ulogic;
      ahbmi      : in  ahb_mst_in_type;
      ahbmo      : out ahb_mst_out_type;
      apbi       : in  apb_slv_in_type;
      apbo       : out apb_slv_out_type;
      swni       : in  grspw_in_type;
      swno       : out grspw_out_type);
  end component;

  type grspw_in_type_vector is array (natural range <>) of grspw_in_type;
  type grspw_out_type_vector is array (natural range <>) of grspw_out_type;

end package;
