library ieee;
use ieee.std_logic_1164.all;

package spwcomp is

  component grspwc is
    generic(
      sysfreq      : integer := 10000;
      usegen       : integer range 0 to 1  := 1;
      nsync        : integer range 1 to 2  := 1; 
      rmap         : integer range 0 to 1  := 0;
      rmapcrc      : integer range 0 to 1  := 0;
      fifosize1    : integer range 4 to 32 := 32;
      fifosize2    : integer range 16 to 64 := 64;
      rxunaligned  : integer range 0 to 1 := 0;
      rmapbufs     : integer range 2 to 8 := 4
    );
    port(
      rst          : in  std_ulogic;
      clk          : in  std_ulogic;
      txclk        : in  std_ulogic;
      --ahb mst in
      hgrant       : in  std_ulogic;
      hready       : in  std_ulogic;   
      hresp        : in  std_logic_vector(1 downto 0);
      hrdata       : in  std_logic_vector(31 downto 0); 
      --ahb mst out
      hbusreq      : out  std_ulogic;        
      hlock        : out  std_ulogic;
      htrans       : out  std_logic_vector(1 downto 0);
      haddr        : out  std_logic_vector(31 downto 0);
      hwrite       : out  std_ulogic;
      hsize        : out  std_logic_vector(2 downto 0);
      hburst       : out  std_logic_vector(2 downto 0);
      hprot        : out  std_logic_vector(3 downto 0);
      hwdata       : out  std_logic_vector(31 downto 0);
      --apb slv in 
      psel	   : in   std_ulogic;
      penable	   : in   std_ulogic;
      paddr	   : in   std_logic_vector(31 downto 0);
      pwrite	   : in   std_ulogic;
      pwdata	   : in   std_logic_vector(31 downto 0);
      --apb slv out
      prdata	   : out  std_logic_vector(31 downto 0);
      --spw in
      di           : in   std_ulogic;
      si           : in   std_ulogic;
      --spw out
      do           : out  std_ulogic;
      so           : out  std_ulogic;
      --time iface
      tickin       : in   std_ulogic;
      tickout      : out  std_ulogic;
      --irq
      irq          : out  std_logic;
      --misc            
      clkdiv10     : in   std_logic_vector(7 downto 0);
      dcrstval     : in   std_logic_vector(9 downto 0);
      timerrstval  : in   std_logic_vector(11 downto 0);
      --rmapen
      rmapen       : in   std_ulogic;
      --clk bufs
      rxclki       : in   std_ulogic;
      rxclko       : out  std_ulogic;
      --rx ahb fifo
      rxrenable    : out  std_ulogic;
      rxraddress   : out  std_logic_vector(4 downto 0);
      rxwrite      : out  std_ulogic;
      rxwdata      : out  std_logic_vector(31 downto 0);
      rxwaddress   : out  std_logic_vector(4 downto 0);
      rxrdata      : in   std_logic_vector(31 downto 0);    
      --tx ahb fifo
      txrenable    : out  std_ulogic;
      txraddress   : out  std_logic_vector(4 downto 0);
      txwrite      : out  std_ulogic;
      txwdata      : out  std_logic_vector(31 downto 0);
      txwaddress   : out  std_logic_vector(4 downto 0);
      txrdata      : in   std_logic_vector(31 downto 0);    
      --nchar fifo
      ncrenable    : out  std_ulogic;
      ncraddress   : out  std_logic_vector(5 downto 0);
      ncwrite      : out  std_ulogic;
      ncwdata      : out  std_logic_vector(8 downto 0);
      ncwaddress   : out  std_logic_vector(5 downto 0);
      ncrdata      : in   std_logic_vector(8 downto 0);
      --rmap buf
      rmrenable    : out  std_ulogic;
      rmraddress   : out  std_logic_vector(7 downto 0);
      rmwrite      : out  std_ulogic;
      rmwdata      : out  std_logic_vector(7 downto 0);
      rmwaddress   : out  std_logic_vector(7 downto 0);
      rmrdata      : in   std_logic_vector(7 downto 0)
    );
  end component;

end package;
