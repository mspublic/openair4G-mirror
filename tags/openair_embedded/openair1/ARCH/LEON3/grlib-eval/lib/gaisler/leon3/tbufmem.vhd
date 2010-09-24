-----------------------------------------------------------------------------
-- Entity: 	tbufmem
-- File:	tbufmem.vhd
-- Author:	Jiri Gaisler - Gaisler Research
-- Description:	128-bit trace buffer memory (CPU/AHB)
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use work.libiu.all;
library techmap;
use techmap.gencomp.all;
library grlib;
use grlib.stdlib.all;

entity tbufmem is
  generic (
    tech   : integer := 0;
    tbuf   : integer := 0 -- trace buf size in kB (0 - no trace buffer)
    );
  port (
    clk : in std_ulogic;
    di  : in tracebuf_in_type;
    do  : out tracebuf_out_type);
end;

architecture rtl of tbufmem is

constant ADDRBITS : integer := 10 + log2(tbuf) - 4;
signal enable : std_logic_vector(1 downto 0);
  
begin

  enable <= di.enable & di.enable;
  mem0 : for i in 0 to 1 generate
    ram0 : syncram64 generic map (tech => tech, abits => addrbits)
      port map ( clk, di.addr(addrbits-1 downto 0), di.data(((i*64)+63) downto (i*64)),
                 do.data(((i*64)+63) downto (i*64)), enable ,di.write(i*2+1 downto i*2));
  end generate;
end;
  
