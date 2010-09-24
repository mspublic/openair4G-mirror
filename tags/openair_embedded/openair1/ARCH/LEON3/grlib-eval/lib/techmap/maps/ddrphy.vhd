------------------------------------------------------------------------------
--  This file is a part of the GRLIB VHDL IP LIBRARY
--  Copyright (C) 2003, Gaisler Research
--
--  This program is free software; you can redistribute it and/or modify
--  it under the terms of the GNU General Public License as published by
--  the Free Software Foundation; either version 2 of the License, or
--  (at your option) any later version.
--
--  This program is distributed in the hope that it will be useful,
--  but WITHOUT ANY WARRANTY; without even the implied warranty of
--  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--  GNU General Public License for more details.
--
--  You should have received a copy of the GNU General Public License
--  along with this program; if not, write to the Free Software
--  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
-----------------------------------------------------------------------------
-- Entity: 	ddrphy
-- File:	ddrphy.vhd
-- Author:	Jiri Gaisler, Gaisler Research
-- Description:	DDR PHY with tech mapping
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;

library grlib;
use grlib.stdlib.all;
library techmap;
use techmap.gencomp.all;
use techmap.allddr.all;

------------------------------------------------------------------
-- DDR PHY with tech mapping  ------------------------------------
------------------------------------------------------------------

entity ddrphy is
  generic (tech : integer := virtex2; MHz : integer := 100; 
	rstdelay : integer := 200; dbits : integer := 16; 
	clk_mul : integer := 2 ; clk_div : integer := 2;
	rskew : integer :=0);
  port (
    rst       : in  std_ulogic;
    clk       : in  std_logic;          	-- input clock
    clkout    : out std_ulogic;			-- system clock
    clkread   : out std_ulogic;			-- read clock
    lock      : out std_ulogic;			-- DCM locked

    ddr_clk 	: out std_logic_vector(2 downto 0);
    ddr_clkb	: out std_logic_vector(2 downto 0);
    ddr_clk_fb_out  : out std_logic;
    ddr_clk_fb  : in std_logic;
    ddr_cke  	: out std_logic_vector(1 downto 0);
    ddr_csb  	: out std_logic_vector(1 downto 0);
    ddr_web  	: out std_ulogic;                       -- ddr write enable
    ddr_rasb  	: out std_ulogic;                       -- ddr ras
    ddr_casb  	: out std_ulogic;                       -- ddr cas
    ddr_dm   	: out std_logic_vector (dbits/8-1 downto 0);    -- ddr dm
    ddr_dqs  	: inout std_logic_vector (dbits/8-1 downto 0);    -- ddr dqs
    ddr_ad      : out std_logic_vector (13 downto 0);   -- ddr address
    ddr_ba      : out std_logic_vector (1 downto 0);    -- ddr bank address
    ddr_dq    	: inout  std_logic_vector (dbits-1 downto 0); -- ddr data
 
    addr  	: in  std_logic_vector (13 downto 0); -- data mask
    ba    	: in  std_logic_vector ( 1 downto 0); -- data mask
    dqin  	: out std_logic_vector (dbits*2-1 downto 0); -- ddr input data
    dqout 	: in  std_logic_vector (dbits*2-1 downto 0); -- ddr input data
    dm    	: in  std_logic_vector (dbits/4-1 downto 0); -- data mask
    oen       	: in  std_ulogic;
    dqs       	: in  std_ulogic;
    dqsoen     	: in  std_ulogic;
    rasn      	: in  std_ulogic;
    casn      	: in  std_ulogic;
    wen       	: in  std_ulogic;
    csn       	: in  std_logic_vector(1 downto 0);
    cke       	: in  std_logic_vector(1 downto 0));
end;

architecture rtl of ddrphy is

begin

  --   alt : if tech = stratxii generate
  -- 
  --     ddr_phy0 : stratixii_ddr_phy 
  --      generic map (MHz => MHz, rstdelay => rstdelay
  -- -- reduce 200 us start-up delay during simulation
  -- -- pragma translate_off
  -- 	/ 200
  -- -- pragma translate_on
  -- 	, clk_mul => clk_mul, clk_div => clk_div, dbits => dbits
  -- 	)
  --      port map (
  -- 	rst, clk, clkout, lock,
  -- 	ddr_clk, ddr_clkb, ddr_clk_fb_out, ddr_clk_fb,
  -- 	ddr_cke, ddr_csb, ddr_web, ddr_rasb, ddr_casb, 
  -- 	ddr_dm, ddr_dqs, ddr_ad, ddr_ba, ddr_dq,
  -- 	addr, ba, dqin, dqout, dm, oen, dqs, dqsoen,
  -- 	rasn, casn, wen, csn, cke);
  -- 
  --   end generate;

  xc2v : if tech = virtex2 generate

    ddr_phy0 : virtex2_ddr_phy 
     generic map (MHz => MHz, rstdelay => rstdelay
-- reduce 200 us start-up delay during simulation
-- pragma translate_off
	/ 200
-- pragma translate_on
	, clk_mul => clk_mul, clk_div => clk_div, dbits => dbits, rskew => rskew
	)
     port map (
	rst, clk, clkout, lock,
	ddr_clk, ddr_clkb, ddr_clk_fb_out, ddr_clk_fb,
	ddr_cke, ddr_csb, ddr_web, ddr_rasb, ddr_casb, 
	ddr_dm, ddr_dqs, ddr_ad, ddr_ba, ddr_dq,
	addr, ba, dqin, dqout, dm, oen, dqs, dqsoen,
	rasn, casn, wen, csn, cke);

  end generate;

  xc4v : if (tech = virtex4) or (tech = virtex5) generate

    ddr_phy0 : virtex4_ddr_phy 
     generic map (MHz => MHz, rstdelay => rstdelay
-- reduce 200 us start-up delay during simulation
-- pragma translate_off
	/ 200
-- pragma translate_on
	, clk_mul => clk_mul, clk_div => clk_div, dbits => dbits, rskew => rskew
	)
     port map (
	rst, clk, clkout, lock,
	ddr_clk, ddr_clkb, ddr_clk_fb_out, ddr_clk_fb,
	ddr_cke, ddr_csb, ddr_web, ddr_rasb, ddr_casb, 
	ddr_dm, ddr_dqs, ddr_ad, ddr_ba, ddr_dq,
	addr, ba, dqin, dqout, dm, oen, dqs, dqsoen,
	rasn, casn, wen, csn, cke);

  end generate;

  --   xc3se : if tech = spartan3e generate
  -- 
  --     ddr_phy0 : spartan3e_ddr_phy 
  --      generic map (MHz => MHz, rstdelay => rstdelay
  -- -- reduce 200 us start-up delay during simulation
  -- -- pragma translate_off
  -- 	/ 200
  -- -- pragma translate_on
  -- 	, clk_mul => clk_mul, clk_div => clk_div, dbits => dbits, rskew => rskew
  -- 	)
  --      port map (
  -- 	rst, clk, clkout, clkread, lock,
  -- 	ddr_clk, ddr_clkb, ddr_clk_fb_out, ddr_clk_fb,
  -- 	ddr_cke, ddr_csb, ddr_web, ddr_rasb, ddr_casb, 
  -- 	ddr_dm, ddr_dqs, ddr_ad, ddr_ba, ddr_dq,
  -- 	addr, ba, dqin, dqout, dm, oen, dqs, dqsoen,
  -- 	rasn, casn, wen, csn, cke);
  -- 
  --   end generate;

end;
