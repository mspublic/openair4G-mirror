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
----------------------------------------------------------------------------   
-- Entity:      defmst
-- File:        defmst.vhd
-- Author:      Edvin Catovic, Gaisler Research
-- Description: Default AHB master
------------------------------------------------------------------------------ 


library IEEE;
use IEEE.std_logic_1164.all;
library grlib;
use grlib.stdlib.all;
use grlib.amba.all;

entity ahbdefmst is
  generic ( hindex : integer range 0 to NAHBMST-1 := 0);
  port ( ahbmo  : out ahb_mst_out_type);
end;


architecture rtl of ahbdefmst is
begin
  
  ahbmo.hbusreq <= '0';
  ahbmo.hlock   <= '0';
  ahbmo.htrans  <= HTRANS_IDLE;
  ahbmo.haddr   <= (others => '0');
  ahbmo.hwrite  <= '0';
  ahbmo.hsize   <= (others => '0');
  ahbmo.hburst  <= (others => '0');
  ahbmo.hprot   <= (others => '0');
  ahbmo.hwdata  <= (others => '0');
  ahbmo.hirq    <= (others => '0');
  ahbmo.hconfig <= (others => (others => '0'));
  ahbmo.hindex  <= hindex;

end;
