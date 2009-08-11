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
-- Package:     charrom_package
-- File:        charrom_package.vhd
-- Author:      Marcus Hellqvist
-- Description: Charrom types and component
-----------------------------------------------------------------------------
library IEEE;
use IEEE.std_logic_1164.all;
library grlib;
use grlib.stdlib.all;

package charrom_package is

type rom_type is record
 addr           : std_logic_vector(11 downto 0);
 data           : std_logic_vector(7 downto 0);
end record;

component charrom
  port(
    clk         : in std_ulogic;
    addr        : in std_logic_vector(11 downto 0);
    data        : out std_logic_vector(7 downto 0)
    );
end component;

end package;
