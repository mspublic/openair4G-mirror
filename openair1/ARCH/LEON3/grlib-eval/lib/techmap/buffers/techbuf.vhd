----------------------------------------------------------------------------
--  This file is a part of the GRLIB VHDL IP LIBRARY
--  Copyright (C) 2004 GAISLER RESEARCH
--
--  This program is free software; you can redistribute it and/or modify
--  it under the terms of the GNU General Public License as published by
--  the Free Software Foundation; either version 2 of the License, or
--  (at your option) any later version.
--
--  See the file COPYING for the full details of the license.
--
-----------------------------------------------------------------------------
-- Entity: 	genclkbuf
-- File:	genclkbuf.vhd
-- Author:	Jiri Gaisler, Marko Isomaki - Gaisler Research
-- Description:	Hard buffers with tech wrapper
------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use work.gencomp.all;

entity techbuf is
  generic(
    buftype  :  integer range 0 to 2 := 0; 
    tech     :  integer range 0 to NTECH := inferred);
  port(
    i        :  in  std_ulogic;
    o        :  out std_ulogic
  );
end entity;

architecture rtl of techbuf is
component clkbuf_actel is
  generic(
    buftype :  integer range 0 to 2 := 0);
  port(
    i       :  in  std_ulogic;
    o       :  out std_ulogic
  );
end component;

component clkbuf_xilinx is
  generic(
    buftype :  integer range 0 to 2 := 0);
  port(
    i       :  in  std_ulogic;
    o       :  out std_ulogic
  );
end component;

begin
  gen : if (tech /= axcel) and (tech /= virtex) and (tech /= virtex2) and
     (tech /= spartan3) and (tech /= virtex4)
  generate
    o <= i;
  end generate;
  axc : if (tech = axcel) generate
    axc : clkbuf_actel generic map (buftype => buftype) port map(i => i, o => o);
  end generate;
  xil : if (tech = virtex) or (tech = virtex2) or (tech = spartan3) or (tech = virtex4) generate
    xil : clkbuf_xilinx generic map (buftype => buftype) port map(i => i, o => o);
  end generate;
end architecture;
