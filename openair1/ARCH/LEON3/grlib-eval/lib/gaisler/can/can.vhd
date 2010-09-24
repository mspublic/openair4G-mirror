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
-- Package: 	can
-- File:	can.vhd
-- Author:	Jiri Gaisler - Gaisler Research
-- Description:	CAN component declartions
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;

library grlib;
use grlib.amba.all;
library techmap;
use techmap.gencomp.all;

package can is

  component can_mod
  generic ( memtech   : integer := DEFMEMTECH);
  port (                          
      reset  : in  std_logic;        
      clk     : in  std_logic;        
      cs      : in  std_logic;        
      we      : in  std_logic;        
      addr    : in  std_logic_vector(7 downto 0);   
      data_in : in  std_logic_vector(7 downto 0);   
      data_out: out std_logic_vector(7 downto 0);   
      irq     : out std_logic;      
      rxi     : in  std_logic;      
      txo     : out std_logic);                           
  end component;

  component can_oc                   
  generic (
    slvndx    : integer := 0;
    ioaddr    : integer := 16#000#;
    iomask    : integer := 16#FF0#;
    irq       : integer := 0;
    memtech   : integer := DEFMEMTECH);
  port (                          
      resetn  : in  std_logic;        
      clk     : in  std_logic;        
      ahbsi   : in  ahb_slv_in_type; 
      ahbso   : out ahb_slv_out_type;
      can_rxi : in  std_logic;      
      can_txo : out std_logic
   );                           
  end component;         

  component can_mc
  generic (
    slvndx    : integer := 0;
    ioaddr    : integer := 16#000#;
    iomask    : integer := 16#FF0#;
    irq       : integer := 0;
    memtech   : integer := DEFMEMTECH;
    ncores    : integer range 1 to 8 := 1;
    sepirq    : integer range 0 to 1 := 0);
  port (                          
    resetn  : in  std_logic;        
    clk     : in  std_logic;        
    ahbsi   : in  ahb_slv_in_type; 
    ahbso   : out ahb_slv_out_type;
    can_rxi : in  std_logic_vector(0 to 7);      
    can_txo : out std_logic_vector(0 to 7)
  );                           
  end component;

end;
