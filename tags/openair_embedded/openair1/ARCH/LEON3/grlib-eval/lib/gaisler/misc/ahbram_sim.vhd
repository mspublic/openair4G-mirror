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
-- Entity: 	ahbram
-- File:	ahbram.vhd
-- Author:	Jiri Gaisler - Gaisler Reserch
-- Description:	AHB ram. 0-waitstate read, 0/1-waitstate write.
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library grlib;
use grlib.amba.all;
use grlib.stdlib.all;
use grlib.devices.all;

library techmap;
use techmap.gencomp.all;
-- to directly instanciate RAMB16 macros for local I & D scratch pad rams
library unisim,simprim;
use unisim.vcomponents.all,simprim.vcomponents.all;
library ocmeminit;use ocmeminit.bprom_init_pkg.all;

entity ahbram is
  generic (
    hindex  : integer := 0;
    haddr   : integer := 0;
    hmask   : integer := 16#fff#;
    tech    : integer := DEFMEMTECH; 
    kbytes  : integer := 1); 
  port (
    rst     : in  std_ulogic;
    clk     : in  std_ulogic;
    ahbsi   : in  ahb_slv_in_type;
    ahbso   : out ahb_slv_out_type
  );
end;

architecture rtl of ahbram is

constant abits : integer := log2(kbytes) + 8;

constant hconfig : ahb_config_type := (
  0 => ahb_device_reg ( VENDOR_GAISLER, GAISLER_AHBRAM, 0, abits+2, 0),
  4 => ahb_membar(haddr, '1', '1', hmask),
  others => zero32);


type reg_type is record
  hwrite : std_ulogic;
  hready : std_ulogic;
  hsel   : std_ulogic;
  addr   : std_logic_vector(abits+1 downto 0);
  size   : std_logic_vector(1 downto 0);
end record;

signal r, c : reg_type;
signal ramsel : std_ulogic;
signal write : std_logic_vector(3 downto 0);
signal ramaddr  : std_logic_vector(abits-1 downto 0);
signal ramaddrb : std_logic_vector(9 downto 0);
signal ramdata  : std_logic_vector(31 downto 0);
signal dip : std_logic_vector(1 downto 0);
type dop_type is array(0 to 3) of std_logic_vector(1 downto 0);
signal dop : dop_type;
signal we32, we10 : std_logic;
begin

  comb : process (ahbsi, r, rst, ramdata)
  variable bs : std_logic_vector(3 downto 0);
  variable v : reg_type;
  variable haddr  : std_logic_vector(abits-1 downto 0);
  begin
    v := r; v.hready := '1'; bs := (others => '0');
    if (r.hwrite or not r.hready) = '1' then haddr := r.addr(abits+1 downto 2);
    else
      haddr := ahbsi.haddr(abits+1 downto 2); bs := (others => '0'); 
    end if;

    if ahbsi.hready = '1' then 
      v.hsel := ahbsi.hsel(hindex) and ahbsi.htrans(1);
      v.hwrite := ahbsi.hwrite and v.hsel;
      v.addr := ahbsi.haddr(abits+1 downto 0); 
      v.size := ahbsi.hsize(1 downto 0);
    end if;

    if r.hwrite = '1' then
      case r.size(1 downto 0) is
      when "00" => bs (conv_integer(r.addr(1 downto 0))) := '1';
      when "01" => bs := r.addr(1) & r.addr(1) & not (r.addr(1) & r.addr(1));
      when others => bs := (others => '1');
      end case;
      v.hready := not (v.hsel and not ahbsi.hwrite);
      v.hwrite := v.hwrite and v.hready;
    end if;

    if rst = '0' then v.hwrite := '0'; v.hready := '1'; end if;
    write <= bs; ramsel <= v.hsel or r.hwrite; ahbso.hready <= r.hready; 
    ramaddr <= haddr; c <= v; ahbso.hrdata <= ramdata;

  end process;

  ahbso.hresp   <= "00"; 
  ahbso.hsplit  <= (others => '0'); 
  ahbso.hirq    <= (others => '0');
  ahbso.hcache  <= '1';
  ahbso.hconfig <= hconfig;
  ahbso.hindex  <= hindex;

-- This is the Gaisler's original instanciation. It always instanciate at least
-- 4 banks, due to the need of byte-wide access...
      --  ra : for i in 0 to 3 generate
      --    aram :  syncram generic map (tech, abits, 8) port map (
      --	clk, ramaddr, ahbsi.hwdata(i*8+7 downto i*8),
      --	ramdata(i*8+7 downto i*8), ramsel, write(3-i));
      --  end generate;

-- ... This is for an 8 Kbytes memory in the form of 4 banks of 2 Kbytes each
-- (each one byte-wide accessed). This was a waste of memory, since we only used
-- 1 Kbytes in total.
-- Anyway, we did that because of the necessity to access the array on a byte basis...
      --  ra : for i in 0 to 3 generate
      --    aram : RAMB16_S9 port map (ramdata(i*8+7 downto i*8), dop(i), ramaddrb, clk,
      --                     ahbsi.hwdata(i*8+7 downto i*8), dip, ramsel, '0', write(3-i));
      --  end generate;
      --  ramaddrb(10 downto abits) <= (others => '0');  -- 1O downto 8...
      --  ramaddrb(abits-1 downto 0) <= ramaddr;         --  7 downto 0...
      --  dip <= (others => '0');

-- ... Since we only use the AHBRAM for the boot prom feature, we DON'T need to access
-- the array one byte after the other, so we could just map 1 bank of 2 Kbytes.
-- But then again, to map Dhrystone .text section in the prom (at least, for test)
-- we must increase the size of the array to 4 Kbytes (abits=10). Therefore, we
-- use 2 banks of 2 Kbytes each, each one being half-word accessed. The write input port
-- is an issue. So we use ORs, and forbid byte sized write accesses (dangerous...):
  aram0: RAMB16_S18
-- pragma translate_off
      generic map (
        INIT_00 => ocram_ahbram0_aram0_INIT_00,
        INIT_01 => ocram_ahbram0_aram0_INIT_01,
        INIT_02 => ocram_ahbram0_aram0_INIT_02,
        INIT_03 => ocram_ahbram0_aram0_INIT_03,
        INIT_04 => ocram_ahbram0_aram0_INIT_04,
        INIT_05 => ocram_ahbram0_aram0_INIT_05,
        INIT_06 => ocram_ahbram0_aram0_INIT_06,
        INIT_07 => ocram_ahbram0_aram0_INIT_07,
        INIT_08 => ocram_ahbram0_aram0_INIT_08,
        INIT_09 => ocram_ahbram0_aram0_INIT_09,
        INIT_0A => ocram_ahbram0_aram0_INIT_0A,
        INIT_0B => ocram_ahbram0_aram0_INIT_0B,
        INIT_0C => ocram_ahbram0_aram0_INIT_0C,
        INIT_0D => ocram_ahbram0_aram0_INIT_0D,
        INIT_0E => ocram_ahbram0_aram0_INIT_0E,
        INIT_0F => ocram_ahbram0_aram0_INIT_0F,
        INIT_10 => ocram_ahbram0_aram0_INIT_10,
        INIT_11 => ocram_ahbram0_aram0_INIT_11,
        INIT_12 => ocram_ahbram0_aram0_INIT_12,
        INIT_13 => ocram_ahbram0_aram0_INIT_13,
        INIT_14 => ocram_ahbram0_aram0_INIT_14,
        INIT_15 => ocram_ahbram0_aram0_INIT_15,
        INIT_16 => ocram_ahbram0_aram0_INIT_16,
        INIT_17 => ocram_ahbram0_aram0_INIT_17,
        INIT_18 => ocram_ahbram0_aram0_INIT_18,
        INIT_19 => ocram_ahbram0_aram0_INIT_19,
        INIT_1A => ocram_ahbram0_aram0_INIT_1A,
        INIT_1B => ocram_ahbram0_aram0_INIT_1B,
        INIT_1C => ocram_ahbram0_aram0_INIT_1C,
        INIT_1D => ocram_ahbram0_aram0_INIT_1D,
        INIT_1E => ocram_ahbram0_aram0_INIT_1E,
        INIT_1F => ocram_ahbram0_aram0_INIT_1F,
        INIT_20 => ocram_ahbram0_aram0_INIT_20,
        INIT_21 => ocram_ahbram0_aram0_INIT_21,
        INIT_22 => ocram_ahbram0_aram0_INIT_22,
        INIT_23 => ocram_ahbram0_aram0_INIT_23,
        INIT_24 => ocram_ahbram0_aram0_INIT_24,
        INIT_25 => ocram_ahbram0_aram0_INIT_25,
        INIT_26 => ocram_ahbram0_aram0_INIT_26,
        INIT_27 => ocram_ahbram0_aram0_INIT_27,
        INIT_28 => ocram_ahbram0_aram0_INIT_28,
        INIT_29 => ocram_ahbram0_aram0_INIT_29,
        INIT_2A => ocram_ahbram0_aram0_INIT_2A,
        INIT_2B => ocram_ahbram0_aram0_INIT_2B,
        INIT_2C => ocram_ahbram0_aram0_INIT_2C,
        INIT_2D => ocram_ahbram0_aram0_INIT_2D,
        INIT_2E => ocram_ahbram0_aram0_INIT_2E,
        INIT_2F => ocram_ahbram0_aram0_INIT_2F,
        INIT_30 => ocram_ahbram0_aram0_INIT_30,
        INIT_31 => ocram_ahbram0_aram0_INIT_31,
        INIT_32 => ocram_ahbram0_aram0_INIT_32,
        INIT_33 => ocram_ahbram0_aram0_INIT_33,
        INIT_34 => ocram_ahbram0_aram0_INIT_34,
        INIT_35 => ocram_ahbram0_aram0_INIT_35,
        INIT_36 => ocram_ahbram0_aram0_INIT_36,
        INIT_37 => ocram_ahbram0_aram0_INIT_37,
        INIT_38 => ocram_ahbram0_aram0_INIT_38,
        INIT_39 => ocram_ahbram0_aram0_INIT_39,
        INIT_3A => ocram_ahbram0_aram0_INIT_3A,
        INIT_3B => ocram_ahbram0_aram0_INIT_3B,
        INIT_3C => ocram_ahbram0_aram0_INIT_3C,
        INIT_3D => ocram_ahbram0_aram0_INIT_3D,
        INIT_3E => ocram_ahbram0_aram0_INIT_3E,
        INIT_3F => ocram_ahbram0_aram0_INIT_3F )
-- pragma translate_on
                    port map (ramdata(15 downto 0), dop(0), ramaddrb, clk,
                    ahbsi.hwdata(15 downto 0), dip, ramsel, '0', we32);
  aram1: RAMB16_S18
-- pragma translate_off
      generic map (
        INIT_00 => ocram_ahbram0_aram1_INIT_00,
        INIT_01 => ocram_ahbram0_aram1_INIT_01,
        INIT_02 => ocram_ahbram0_aram1_INIT_02,
        INIT_03 => ocram_ahbram0_aram1_INIT_03,
        INIT_04 => ocram_ahbram0_aram1_INIT_04,
        INIT_05 => ocram_ahbram0_aram1_INIT_05,
        INIT_06 => ocram_ahbram0_aram1_INIT_06,
        INIT_07 => ocram_ahbram0_aram1_INIT_07,
        INIT_08 => ocram_ahbram0_aram1_INIT_08,
        INIT_09 => ocram_ahbram0_aram1_INIT_09,
        INIT_0A => ocram_ahbram0_aram1_INIT_0A,
        INIT_0B => ocram_ahbram0_aram1_INIT_0B,
        INIT_0C => ocram_ahbram0_aram1_INIT_0C,
        INIT_0D => ocram_ahbram0_aram1_INIT_0D,
        INIT_0E => ocram_ahbram0_aram1_INIT_0E,
        INIT_0F => ocram_ahbram0_aram1_INIT_0F,
        INIT_10 => ocram_ahbram0_aram1_INIT_10,
        INIT_11 => ocram_ahbram0_aram1_INIT_11,
        INIT_12 => ocram_ahbram0_aram1_INIT_12,
        INIT_13 => ocram_ahbram0_aram1_INIT_13,
        INIT_14 => ocram_ahbram0_aram1_INIT_14,
        INIT_15 => ocram_ahbram0_aram1_INIT_15,
        INIT_16 => ocram_ahbram0_aram1_INIT_16,
        INIT_17 => ocram_ahbram0_aram1_INIT_17,
        INIT_18 => ocram_ahbram0_aram1_INIT_18,
        INIT_19 => ocram_ahbram0_aram1_INIT_19,
        INIT_1A => ocram_ahbram0_aram1_INIT_1A,
        INIT_1B => ocram_ahbram0_aram1_INIT_1B,
        INIT_1C => ocram_ahbram0_aram1_INIT_1C,
        INIT_1D => ocram_ahbram0_aram1_INIT_1D,
        INIT_1E => ocram_ahbram0_aram1_INIT_1E,
        INIT_1F => ocram_ahbram0_aram1_INIT_1F,
        INIT_20 => ocram_ahbram0_aram1_INIT_20,
        INIT_21 => ocram_ahbram0_aram1_INIT_21,
        INIT_22 => ocram_ahbram0_aram1_INIT_22,
        INIT_23 => ocram_ahbram0_aram1_INIT_23,
        INIT_24 => ocram_ahbram0_aram1_INIT_24,
        INIT_25 => ocram_ahbram0_aram1_INIT_25,
        INIT_26 => ocram_ahbram0_aram1_INIT_26,
        INIT_27 => ocram_ahbram0_aram1_INIT_27,
        INIT_28 => ocram_ahbram0_aram1_INIT_28,
        INIT_29 => ocram_ahbram0_aram1_INIT_29,
        INIT_2A => ocram_ahbram0_aram1_INIT_2A,
        INIT_2B => ocram_ahbram0_aram1_INIT_2B,
        INIT_2C => ocram_ahbram0_aram1_INIT_2C,
        INIT_2D => ocram_ahbram0_aram1_INIT_2D,
        INIT_2E => ocram_ahbram0_aram1_INIT_2E,
        INIT_2F => ocram_ahbram0_aram1_INIT_2F,
        INIT_30 => ocram_ahbram0_aram1_INIT_30,
        INIT_31 => ocram_ahbram0_aram1_INIT_31,
        INIT_32 => ocram_ahbram0_aram1_INIT_32,
        INIT_33 => ocram_ahbram0_aram1_INIT_33,
        INIT_34 => ocram_ahbram0_aram1_INIT_34,
        INIT_35 => ocram_ahbram0_aram1_INIT_35,
        INIT_36 => ocram_ahbram0_aram1_INIT_36,
        INIT_37 => ocram_ahbram0_aram1_INIT_37,
        INIT_38 => ocram_ahbram0_aram1_INIT_38,
        INIT_39 => ocram_ahbram0_aram1_INIT_39,
        INIT_3A => ocram_ahbram0_aram1_INIT_3A,
        INIT_3B => ocram_ahbram0_aram1_INIT_3B,
        INIT_3C => ocram_ahbram0_aram1_INIT_3C,
        INIT_3D => ocram_ahbram0_aram1_INIT_3D,
        INIT_3E => ocram_ahbram0_aram1_INIT_3E,
        INIT_3F => ocram_ahbram0_aram1_INIT_3F )
-- pragma translate_on
                    port map (ramdata(31 downto 16), dop(1), ramaddrb, clk,
                    ahbsi.hwdata(31 downto 16), dip, ramsel, '0', we10);
  ramaddrb(9 downto 0) <= ramaddr(abits-1 downto 0);        -- 9 downto 0...
  dip <= (others => '0');
  we32 <= write(3) or write(2);
  we10 <= write(1) or write(0);

  reg : process (clk)
  begin
    if rising_edge(clk ) then r <= c; end if;
  end process;

-- pragma translate_off
    bootmsg : report_version 
    generic map ("ahbram" & tost(hindex) &
    ": AHB SRAM Module rev 1, " & tost(kbytes) & " kbytes");
-- pragma translate_on
end;
