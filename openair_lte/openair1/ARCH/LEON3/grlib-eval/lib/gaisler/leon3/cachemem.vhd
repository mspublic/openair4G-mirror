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
-- Entity: 	cachemem
-- File:	cachemem.vhd
-- Author:	Jiri Gaisler - Gaisler Research
-- Description:	Contains ram cells for both instruction and data caches
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library gaisler;
use gaisler.libiu.all;
use gaisler.libcache.all;
use gaisler.mmuconfig.all;
library grlib;
use grlib.stdlib.all;
library techmap;
use techmap.gencomp.all;
-- to directly instanciate RAMB16 macros for local I & D scratch pad rams
use techmap.xcram.all;

entity cachemem is
  generic (
    tech      : integer range 0 to NTECH := 0;
    icen      : integer range 0 to 1 := 0;
    irepl     : integer range 0 to 2 := 0;
    isets     : integer range 1 to 4 := 1;
    ilinesize : integer range 4 to 8 := 4;
    isetsize  : integer range 1 to 256 := 1;
    isetlock  : integer range 0 to 1 := 0;
    dcen      : integer range 0 to 1 := 0;
    drepl     : integer range 0 to 2 := 0;
    dsets     : integer range 1 to 4 := 1;
    dlinesize : integer range 4 to 8 := 4;
    dsetsize  : integer range 1 to 256 := 1;
    dsetlock  : integer range 0 to 1 := 0;
    dsnoop    : integer range 0 to 6 := 0;
    ilram      : integer range 0 to 1 := 0;
    ilramsize  : integer range 1 to 512 := 1;        
    dlram      : integer range 0 to 1 := 0;
    dlramsize  : integer range 1 to 512 := 1;
    mmuen     : integer range 0 to 1 := 0
  );
  port (
        clk   : in  std_ulogic;
	crami : in  cram_in_type;
	cramo : out cram_out_type;
        sclk  : in  std_ulogic
  );
end;

architecture rtl of cachemem is
  constant DSNOOPMMU    : boolean := (dsnoop > 3);
  constant ILINE_BITS   : integer := log2(ilinesize);
  constant IOFFSET_BITS : integer := 8 +log2(isetsize) - ILINE_BITS;
  constant DLINE_BITS   : integer := log2(dlinesize);
  constant DOFFSET_BITS : integer := 8 +log2(dsetsize) - DLINE_BITS;
  constant ITAG_BITS    : integer := TAG_HIGH - IOFFSET_BITS - ILINE_BITS - 2 + ilinesize + 1;
  constant DTAG_BITS    : integer := TAG_HIGH - DOFFSET_BITS - DLINE_BITS - 2 + dlinesize + 1;
  constant IPTAG_BITS   : integer := TAG_HIGH - IOFFSET_BITS - ILINE_BITS - 2 + 1;
  constant DPTAG_BITS   : integer := TAG_HIGH - DOFFSET_BITS - DLINE_BITS - 2 + 1;
  constant ILRR_BIT     : integer := creplalg_tbl(irepl);
  constant DLRR_BIT     : integer := creplalg_tbl(drepl);
  constant ITAG_LOW     : integer := IOFFSET_BITS + ILINE_BITS + 2;
  constant DTAG_LOW     : integer := DOFFSET_BITS + DLINE_BITS + 2;
  constant ICLOCK_BIT   : integer := isetlock;
  constant DCLOCK_BIT   : integer := dsetlock;
  constant ILRAM_BITS   : integer := log2(ilramsize) + 10;
  constant DLRAM_BITS   : integer := log2(dlramsize) + 10;


  constant ITDEPTH : natural := 2**IOFFSET_BITS;
  constant DTDEPTH : natural := 2**DOFFSET_BITS;
  constant MMUCTX_BITS : natural := 8*mmuen;

  -- i/d tag layout
  -- +-----+----------+--------+-----+-------+
  -- | LRR | LOCK_BIT | MMUCTX | TAG | VALID |
  -- +-----+----------+--------+-----+-------+

  constant ITWIDTH : natural := ITAG_BITS + ILRR_BIT + isetlock + MMUCTX_BITS;
  constant DTWIDTH : natural := DTAG_BITS + DLRR_BIT + dsetlock + MMUCTX_BITS;
  constant IDWIDTH : natural := 32;
  constant DDWIDTH : natural := 32;

  subtype dtdatain_vector is std_logic_vector(DTWIDTH downto 0);
  type dtdatain_type is array (0 to MAXSETS-1) of dtdatain_vector;
  subtype itdatain_vector is std_logic_vector(ITWIDTH downto 0);
  type itdatain_type is array (0 to MAXSETS-1) of itdatain_vector;
  
  subtype itdataout_vector is std_logic_vector(ITWIDTH-1 downto 0);
  type itdataout_type is array (0 to MAXSETS-1) of itdataout_vector;
  subtype iddataout_vector is std_logic_vector(IDWIDTH -1 downto 0);
  type iddataout_type is array (0 to MAXSETS-1) of iddataout_vector;
  subtype dtdataout_vector is std_logic_vector(DTWIDTH-1 downto 0);
  type dtdataout_type is array (0 to MAXSETS-1) of dtdataout_vector;
  subtype dddataout_vector is std_logic_vector(DDWIDTH -1 downto 0);
  type dddataout_type is array (0 to MAXSETS-1) of dddataout_vector;
  

  signal itaddr    : std_logic_vector(IOFFSET_BITS + ILINE_BITS -1 downto ILINE_BITS);
  signal idaddr    : std_logic_vector(IOFFSET_BITS + ILINE_BITS -1 downto 0);
  signal ildaddr   : std_logic_vector(ILRAM_BITS-3 downto 0);

  signal itdatain  : itdatain_type; 
  signal itdataout : itdataout_type;
  signal iddatain  : std_logic_vector(IDWIDTH -1 downto 0);
  signal iddataout : iddataout_type;
  signal ildataout : std_logic_vector(31 downto 0);

  signal itenable  : std_ulogic;
  signal idenable  : std_ulogic;
  signal itwrite   : std_logic_vector(0 to MAXSETS-1);
  signal idwrite   : std_logic_vector(0 to MAXSETS-1);

  signal dtaddr    : std_logic_vector(DOFFSET_BITS + DLINE_BITS -1 downto DLINE_BITS);
  signal dtaddr2   : std_logic_vector(DOFFSET_BITS + DLINE_BITS -1 downto DLINE_BITS);
  signal ddaddr    : std_logic_vector(DOFFSET_BITS + DLINE_BITS -1 downto 0);
  signal ldaddr    : std_logic_vector(DLRAM_BITS-1 downto 2);
  
  signal dtdatain  : dtdatain_type; 
  signal dtdatain2 : dtdatain_type;
  signal dtdatain3 : dtdatain_type;
  signal dtdatainu : dtdatain_type;
  signal dtdataout : dtdataout_type;
  signal dtdataout2: dtdataout_type;
  signal dtdataout3: dtdataout_type;
  signal dtdataoutu: dtdataout_type;
  signal dddatain  : cdatatype;
  signal dddataout : dddataout_type;
  signal lddatain, ldataout  : std_logic_vector(31 downto 0);

  signal dtenable  : std_ulogic;
  signal dtenable2 : std_ulogic;
  signal dtenable3 : std_ulogic;
  signal ddenable  : std_ulogic;
  signal dtwrite   : std_logic_vector(0 to MAXSETS-1);
  signal dtwrite2  : std_logic_vector(0 to MAXSETS-1);
  signal dtwrite3  : std_logic_vector(0 to MAXSETS-1);
  signal ddwrite   : std_logic_vector(0 to MAXSETS-1);

  signal vcc, gnd  : std_ulogic;

begin

  vcc <= '1'; gnd <= '0'; 
  itaddr <= crami.icramin.address(IOFFSET_BITS + ILINE_BITS -1 downto ILINE_BITS);
  idaddr <= crami.icramin.address(IOFFSET_BITS + ILINE_BITS -1 downto 0);
  ildaddr <= crami.icramin.address(ILRAM_BITS-3 downto 0);
  
  itinsel : process(crami, dtdataout2, dtdataout3)

  variable viddatain  : std_logic_vector(IDWIDTH -1 downto 0);
  variable vdddatain  : cdatatype;
  variable vitdatain : itdatain_type;
  variable vdtdatain : dtdatain_type;
  variable vdtdatain2 : dtdatain_type;
  variable vdtdatain3 : dtdatain_type;
  variable vdtdatainu : dtdatain_type;
  begin
    viddatain := (others => '0');
    vdddatain := (others => (others => '0'));

    viddatain(31 downto 0) := crami.icramin.data;

    for i in 0 to DSETS-1 loop
      vdtdatain(i) := (others => '0');
      if mmuen = 1 then
        vdtdatain(i)((DTWIDTH - (DLRR_BIT+dsetlock+1)) downto (DTWIDTH - (DLRR_BIT+dsetlock+M_CTX_SZ))) := crami.dcramin.ctx(i);
      end if;
      vdtdatain(i)(DTWIDTH-(DCLOCK_BIT + dsetlock)) := crami.dcramin.tag(i)(CTAG_LOCKPOS);
      vdtdatain(i)(DTWIDTH-DLRR_BIT) := crami.dcramin.tag(i)(CTAG_LRRPOS);          
      vdtdatain(i)(DTAG_BITS-1 downto 0) := crami.dcramin.tag(i)(TAG_HIGH downto DTAG_LOW) & crami.dcramin.tag(i)(dlinesize-1 downto 0);
      if (DSETS > 1) and (crami.dcramin.flush = '1') then
	vdtdatain(i)(dlinesize+1 downto dlinesize) :=  conv_std_logic_vector(i,2);
      end if;
    end loop;

    vdtdatain2 := (others => (others => '0'));
    for i in 0 to DSETS-1 loop

--       vdtdatain2(i)(DTWIDTH-(DLRR_BIT + DCLOCK_BIT)) := dtdataout2(i)(DTWIDTH-(1+DCLOCK_BIT));
--       vdtdatain2(i)(DTWIDTH-DCLOCK_BIT) := dtdataout2(i)(DTWIDTH-1);
-- --      vdtdatain2(i)(DTAG_BITS-1 downto dlinesize) := crami.dcramin.stag;
      --vdtdatain2(i)(DTWIDTH-1 downto 0) := dtdataout2(i)(DTWIDTH-1 downto 0);
      --vdtdatain2(i)(dlinesize-1 downto 0) := zero64(dlinesize-1 downto 0);
      if (DSETS > 1) then 
        vdtdatain2(i)(dlinesize+1 downto dlinesize) := conv_std_logic_vector(i,2);
      end if;
    end loop;
    vdddatain := crami.dcramin.data;

    vdtdatainu := (others => (others => '0'));
    vdtdatain3 := (others => (others => '0'));
    for i in 0 to DSETS-1 loop
      vdtdatain3(i) := (others => '0');
      vdtdatain3(i)(DTAG_BITS-1 downto DTAG_BITS-DPTAG_BITS) := crami.dcramin.ptag(i)(TAG_HIGH downto DTAG_LOW);
    end loop;

    for i in 0 to ISETS-1 loop
      vitdatain(i) := (others => '0');
      if mmuen = 1 then
        vitdatain(i)((ITWIDTH - (ILRR_BIT+isetlock+1)) downto (ITWIDTH - (ILRR_BIT+isetlock+M_CTX_SZ))) := crami.icramin.ctx;
      end if;
      vitdatain(i)(ITWIDTH-(ICLOCK_BIT + isetlock)) := crami.icramin.tag(i)(CTAG_LOCKPOS);
      vitdatain(i)(ITWIDTH-ILRR_BIT) := crami.icramin.tag(i)(CTAG_LRRPOS);
      vitdatain(i)(ITAG_BITS-1 downto 0) := crami.icramin.tag(i)(TAG_HIGH downto ITAG_LOW) & crami.icramin.tag(i)(ilinesize-1 downto 0);
      if (ISETS > 1) and (crami.icramin.flush = '1') then
	vitdatain(i)(ilinesize+1 downto ilinesize) :=  conv_std_logic_vector(i,2);
      end if;
    end loop;

    itdatain <= vitdatain; iddatain <= viddatain;
    dtdatain <= vdtdatain; dtdatain2 <= vdtdatain2; dtdatain3 <= vdtdatain3; dtdatainu <= vdtdatainu; dddatain <= vdddatain;

  end process;

  
  itwrite   <= crami.icramin.twrite;
  idwrite   <= crami.icramin.dwrite;
  itenable  <= crami.icramin.tenable;
  idenable  <= crami.icramin.denable;

  dtaddr <= crami.dcramin.address(DOFFSET_BITS + DLINE_BITS -1 downto DLINE_BITS);
  dtaddr2 <= crami.dcramin.saddress(DOFFSET_BITS-1 downto 0);  
  ddaddr <= crami.dcramin.address(DOFFSET_BITS + DLINE_BITS -1 downto 0);
  ldaddr <= crami.dcramin.ldramin.address(DLRAM_BITS-1 downto 2);
  dtwrite   <= crami.dcramin.twrite;
  dtwrite2  <= crami.dcramin.swrite;
  dtwrite3  <= crami.dcramin.tpwrite;
  ddwrite   <= crami.dcramin.dwrite;
  dtenable  <= crami.dcramin.tenable;
  dtenable2 <= crami.dcramin.senable;
  dtenable3 <= crami.dcramin.senable;
  ddenable  <= crami.dcramin.denable;


  ime : if icen = 1 generate
    im0 : for i in 0 to ISETS-1 generate
      itags0 : syncram generic map (tech, IOFFSET_BITS, ITWIDTH)
      port map ( clk, itaddr, itdatain(i)(ITWIDTH-1 downto 0), itdataout(i)(ITWIDTH-1 downto 0), itenable, itwrite(i));
      idata0 : syncram generic map (tech, IOFFSET_BITS+ILINE_BITS, IDWIDTH)
      port map (clk, idaddr, iddatain, iddataout(i), idenable, idwrite(i));
    end generate;
  end generate;

  imd : if icen = 0 generate
    ind0 : for i in 0 to ISETS-1 generate
      itdataout(i) <= (others => '0');
      iddataout(i) <= (others => '0');
    end generate;
  end generate;

--  ild0 : if ilram = 1 generate
--    ildata0 : syncram
--     generic map (tech, ILRAM_BITS-2, 32)
--      port map (clk, ildaddr, iddatain, ildataout, 
--	  crami.icramin.ldramin.enable, crami.icramin.ldramin.write);
--  end generate;

  ild0 : if ilram = 1 generate
    -- ildramgen : for i in 0 to 7 generate
      ildram0 : RAMB16_S2
-- pragma translate_off
       generic map (
        INIT_00 => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_00,
        INIT_01 => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_01,
        INIT_02 => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_02,
        INIT_03 => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_03,
        INIT_04 => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_04,
        INIT_05 => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_05,
        INIT_06 => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_06,
        INIT_07 => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_07,
        INIT_08 => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_08,
        INIT_09 => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_09,
        INIT_0A => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_0A,
        INIT_0B => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_0B,
        INIT_0C => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_0C,
        INIT_0D => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_0D,
        INIT_0E => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_0E,
        INIT_0F => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_0F,
        INIT_10 => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_10,
        INIT_11 => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_11,
        INIT_12 => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_12,
        INIT_13 => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_13,
        INIT_14 => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_14,
        INIT_15 => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_15,
        INIT_16 => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_16,
        INIT_17 => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_17,
        INIT_18 => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_18,
        INIT_19 => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_19,
        INIT_1A => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_1A,
        INIT_1B => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_1B,
        INIT_1C => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_1C,
        INIT_1D => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_1D,
        INIT_1E => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_1E,
        INIT_1F => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_1F,
        INIT_20 => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_20,
        INIT_21 => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_21,
        INIT_22 => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_22,
        INIT_23 => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_23,
        INIT_24 => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_24,
        INIT_25 => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_25,
        INIT_26 => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_26,
        INIT_27 => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_27,
        INIT_28 => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_28,
        INIT_29 => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_29,
        INIT_2A => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_2A,
        INIT_2B => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_2B,
        INIT_2C => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_2C,
        INIT_2D => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_2D,
        INIT_2E => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_2E,
        INIT_2F => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_2F,
        INIT_30 => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_30,
        INIT_31 => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_31,
        INIT_32 => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_32,
        INIT_33 => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_33,
        INIT_34 => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_34,
        INIT_35 => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_35,
        INIT_36 => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_36,
        INIT_37 => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_37,
        INIT_38 => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_38,
        INIT_39 => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_39,
        INIT_3A => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_3A,
        INIT_3B => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_3B,
        INIT_3C => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_3C,
        INIT_3D => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_3D,
        INIT_3E => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_3E,
        INIT_3F => cpu_0_u0_cmem0_ild0_ildramgen_0_ildram_INIT_3F )
-- pragma translate_on
                 port map (ildataout(1 downto 0), ildaddr, clk,
                          iddatain(1 downto 0), crami.icramin.ldramin.enable, '0', crami.icramin.ldramin.write);
      ildram1 : RAMB16_S2
-- pragma translate_off
generic map (
        INIT_00 => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_00,
        INIT_01 => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_01,
        INIT_02 => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_02,
        INIT_03 => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_03,
        INIT_04 => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_04,
        INIT_05 => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_05,
        INIT_06 => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_06,
        INIT_07 => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_07,
        INIT_08 => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_08,
        INIT_09 => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_09,
        INIT_0A => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_0A,
        INIT_0B => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_0B,
        INIT_0C => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_0C,
        INIT_0D => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_0D,
        INIT_0E => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_0E,
        INIT_0F => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_0F,
        INIT_10 => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_10,
        INIT_11 => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_11,
        INIT_12 => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_12,
        INIT_13 => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_13,
        INIT_14 => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_14,
        INIT_15 => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_15,
        INIT_16 => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_16,
        INIT_17 => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_17,
        INIT_18 => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_18,
        INIT_19 => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_19,
        INIT_1A => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_1A,
        INIT_1B => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_1B,
        INIT_1C => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_1C,
        INIT_1D => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_1D,
        INIT_1E => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_1E,
        INIT_1F => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_1F,
        INIT_20 => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_20,
        INIT_21 => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_21,
        INIT_22 => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_22,
        INIT_23 => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_23,
        INIT_24 => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_24,
        INIT_25 => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_25,
        INIT_26 => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_26,
        INIT_27 => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_27,
        INIT_28 => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_28,
        INIT_29 => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_29,
        INIT_2A => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_2A,
        INIT_2B => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_2B,
        INIT_2C => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_2C,
        INIT_2D => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_2D,
        INIT_2E => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_2E,
        INIT_2F => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_2F,
        INIT_30 => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_30,
        INIT_31 => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_31,
        INIT_32 => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_32,
        INIT_33 => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_33,
        INIT_34 => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_34,
        INIT_35 => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_35,
        INIT_36 => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_36,
        INIT_37 => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_37,
        INIT_38 => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_38,
        INIT_39 => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_39,
        INIT_3A => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_3A,
        INIT_3B => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_3B,
        INIT_3C => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_3C,
        INIT_3D => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_3D,
        INIT_3E => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_3E,
        INIT_3F => cpu_0_u0_cmem0_ild0_ildramgen_1_ildram_INIT_3F )
-- pragma translate_on
                 port map (ildataout(3 downto 2), ildaddr, clk,
                          iddatain(3 downto 2), crami.icramin.ldramin.enable, '0', crami.icramin.ldramin.write);
      ildram2 : RAMB16_S2
-- pragma translate_off
generic map (
        INIT_00 => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_00,
        INIT_01 => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_01,
        INIT_02 => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_02,
        INIT_03 => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_03,
        INIT_04 => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_04,
        INIT_05 => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_05,
        INIT_06 => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_06,
        INIT_07 => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_07,
        INIT_08 => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_08,
        INIT_09 => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_09,
        INIT_0A => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_0A,
        INIT_0B => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_0B,
        INIT_0C => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_0C,
        INIT_0D => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_0D,
        INIT_0E => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_0E,
        INIT_0F => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_0F,
        INIT_10 => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_10,
        INIT_11 => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_11,
        INIT_12 => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_12,
        INIT_13 => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_13,
        INIT_14 => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_14,
        INIT_15 => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_15,
        INIT_16 => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_16,
        INIT_17 => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_17,
        INIT_18 => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_18,
        INIT_19 => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_19,
        INIT_1A => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_1A,
        INIT_1B => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_1B,
        INIT_1C => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_1C,
        INIT_1D => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_1D,
        INIT_1E => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_1E,
        INIT_1F => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_1F,
        INIT_20 => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_20,
        INIT_21 => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_21,
        INIT_22 => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_22,
        INIT_23 => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_23,
        INIT_24 => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_24,
        INIT_25 => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_25,
        INIT_26 => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_26,
        INIT_27 => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_27,
        INIT_28 => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_28,
        INIT_29 => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_29,
        INIT_2A => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_2A,
        INIT_2B => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_2B,
        INIT_2C => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_2C,
        INIT_2D => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_2D,
        INIT_2E => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_2E,
        INIT_2F => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_2F,
        INIT_30 => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_30,
        INIT_31 => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_31,
        INIT_32 => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_32,
        INIT_33 => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_33,
        INIT_34 => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_34,
        INIT_35 => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_35,
        INIT_36 => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_36,
        INIT_37 => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_37,
        INIT_38 => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_38,
        INIT_39 => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_39,
        INIT_3A => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_3A,
        INIT_3B => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_3B,
        INIT_3C => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_3C,
        INIT_3D => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_3D,
        INIT_3E => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_3E,
        INIT_3F => cpu_0_u0_cmem0_ild0_ildramgen_2_ildram_INIT_3F )
-- pragma translate_on
                 port map (ildataout(5 downto 4), ildaddr, clk,
                          iddatain(5 downto 4), crami.icramin.ldramin.enable, '0', crami.icramin.ldramin.write);
      ildram3 : RAMB16_S2
-- pragma translate_off
generic map (
        INIT_00 => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_00,
        INIT_01 => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_01,
        INIT_02 => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_02,
        INIT_03 => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_03,
        INIT_04 => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_04,
        INIT_05 => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_05,
        INIT_06 => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_06,
        INIT_07 => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_07,
        INIT_08 => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_08,
        INIT_09 => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_09,
        INIT_0A => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_0A,
        INIT_0B => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_0B,
        INIT_0C => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_0C,
        INIT_0D => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_0D,
        INIT_0E => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_0E,
        INIT_0F => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_0F,
        INIT_10 => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_10,
        INIT_11 => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_11,
        INIT_12 => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_12,
        INIT_13 => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_13,
        INIT_14 => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_14,
        INIT_15 => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_15,
        INIT_16 => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_16,
        INIT_17 => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_17,
        INIT_18 => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_18,
        INIT_19 => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_19,
        INIT_1A => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_1A,
        INIT_1B => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_1B,
        INIT_1C => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_1C,
        INIT_1D => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_1D,
        INIT_1E => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_1E,
        INIT_1F => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_1F,
        INIT_20 => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_20,
        INIT_21 => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_21,
        INIT_22 => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_22,
        INIT_23 => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_23,
        INIT_24 => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_24,
        INIT_25 => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_25,
        INIT_26 => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_26,
        INIT_27 => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_27,
        INIT_28 => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_28,
        INIT_29 => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_29,
        INIT_2A => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_2A,
        INIT_2B => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_2B,
        INIT_2C => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_2C,
        INIT_2D => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_2D,
        INIT_2E => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_2E,
        INIT_2F => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_2F,
        INIT_30 => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_30,
        INIT_31 => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_31,
        INIT_32 => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_32,
        INIT_33 => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_33,
        INIT_34 => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_34,
        INIT_35 => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_35,
        INIT_36 => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_36,
        INIT_37 => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_37,
        INIT_38 => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_38,
        INIT_39 => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_39,
        INIT_3A => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_3A,
        INIT_3B => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_3B,
        INIT_3C => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_3C,
        INIT_3D => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_3D,
        INIT_3E => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_3E,
        INIT_3F => cpu_0_u0_cmem0_ild0_ildramgen_3_ildram_INIT_3F )
-- pragma translate_on
                 port map (ildataout(7 downto 6), ildaddr, clk,
                          iddatain(7 downto 6), crami.icramin.ldramin.enable, '0', crami.icramin.ldramin.write);
      ildram4 : RAMB16_S2
-- pragma translate_off
generic map (
        INIT_00 => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_00,
        INIT_01 => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_01,
        INIT_02 => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_02,
        INIT_03 => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_03,
        INIT_04 => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_04,
        INIT_05 => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_05,
        INIT_06 => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_06,
        INIT_07 => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_07,
        INIT_08 => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_08,
        INIT_09 => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_09,
        INIT_0A => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_0A,
        INIT_0B => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_0B,
        INIT_0C => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_0C,
        INIT_0D => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_0D,
        INIT_0E => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_0E,
        INIT_0F => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_0F,
        INIT_10 => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_10,
        INIT_11 => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_11,
        INIT_12 => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_12,
        INIT_13 => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_13,
        INIT_14 => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_14,
        INIT_15 => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_15,
        INIT_16 => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_16,
        INIT_17 => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_17,
        INIT_18 => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_18,
        INIT_19 => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_19,
        INIT_1A => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_1A,
        INIT_1B => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_1B,
        INIT_1C => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_1C,
        INIT_1D => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_1D,
        INIT_1E => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_1E,
        INIT_1F => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_1F,
        INIT_20 => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_20,
        INIT_21 => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_21,
        INIT_22 => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_22,
        INIT_23 => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_23,
        INIT_24 => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_24,
        INIT_25 => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_25,
        INIT_26 => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_26,
        INIT_27 => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_27,
        INIT_28 => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_28,
        INIT_29 => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_29,
        INIT_2A => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_2A,
        INIT_2B => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_2B,
        INIT_2C => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_2C,
        INIT_2D => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_2D,
        INIT_2E => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_2E,
        INIT_2F => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_2F,
        INIT_30 => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_30,
        INIT_31 => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_31,
        INIT_32 => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_32,
        INIT_33 => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_33,
        INIT_34 => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_34,
        INIT_35 => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_35,
        INIT_36 => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_36,
        INIT_37 => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_37,
        INIT_38 => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_38,
        INIT_39 => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_39,
        INIT_3A => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_3A,
        INIT_3B => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_3B,
        INIT_3C => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_3C,
        INIT_3D => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_3D,
        INIT_3E => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_3E,
        INIT_3F => cpu_0_u0_cmem0_ild0_ildramgen_4_ildram_INIT_3F )
-- pragma translate_on
                 port map (ildataout(9 downto 8), ildaddr, clk,
                          iddatain(9 downto 8), crami.icramin.ldramin.enable, '0', crami.icramin.ldramin.write);
      ildram5 : RAMB16_S2
-- pragma translate_off
generic map (
        INIT_00 => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_00,
        INIT_01 => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_01,
        INIT_02 => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_02,
        INIT_03 => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_03,
        INIT_04 => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_04,
        INIT_05 => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_05,
        INIT_06 => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_06,
        INIT_07 => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_07,
        INIT_08 => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_08,
        INIT_09 => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_09,
        INIT_0A => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_0A,
        INIT_0B => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_0B,
        INIT_0C => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_0C,
        INIT_0D => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_0D,
        INIT_0E => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_0E,
        INIT_0F => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_0F,
        INIT_10 => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_10,
        INIT_11 => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_11,
        INIT_12 => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_12,
        INIT_13 => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_13,
        INIT_14 => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_14,
        INIT_15 => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_15,
        INIT_16 => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_16,
        INIT_17 => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_17,
        INIT_18 => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_18,
        INIT_19 => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_19,
        INIT_1A => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_1A,
        INIT_1B => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_1B,
        INIT_1C => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_1C,
        INIT_1D => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_1D,
        INIT_1E => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_1E,
        INIT_1F => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_1F,
        INIT_20 => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_20,
        INIT_21 => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_21,
        INIT_22 => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_22,
        INIT_23 => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_23,
        INIT_24 => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_24,
        INIT_25 => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_25,
        INIT_26 => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_26,
        INIT_27 => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_27,
        INIT_28 => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_28,
        INIT_29 => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_29,
        INIT_2A => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_2A,
        INIT_2B => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_2B,
        INIT_2C => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_2C,
        INIT_2D => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_2D,
        INIT_2E => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_2E,
        INIT_2F => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_2F,
        INIT_30 => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_30,
        INIT_31 => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_31,
        INIT_32 => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_32,
        INIT_33 => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_33,
        INIT_34 => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_34,
        INIT_35 => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_35,
        INIT_36 => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_36,
        INIT_37 => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_37,
        INIT_38 => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_38,
        INIT_39 => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_39,
        INIT_3A => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_3A,
        INIT_3B => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_3B,
        INIT_3C => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_3C,
        INIT_3D => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_3D,
        INIT_3E => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_3E,
        INIT_3F => cpu_0_u0_cmem0_ild0_ildramgen_5_ildram_INIT_3F )
-- pragma translate_on
                 port map (ildataout(11 downto 10), ildaddr, clk,
                          iddatain(11 downto 10), crami.icramin.ldramin.enable, '0', crami.icramin.ldramin.write);
      ildram6 : RAMB16_S2
-- pragma translate_off
generic map (
        INIT_00 => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_00,
        INIT_01 => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_01,
        INIT_02 => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_02,
        INIT_03 => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_03,
        INIT_04 => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_04,
        INIT_05 => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_05,
        INIT_06 => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_06,
        INIT_07 => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_07,
        INIT_08 => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_08,
        INIT_09 => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_09,
        INIT_0A => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_0A,
        INIT_0B => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_0B,
        INIT_0C => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_0C,
        INIT_0D => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_0D,
        INIT_0E => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_0E,
        INIT_0F => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_0F,
        INIT_10 => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_10,
        INIT_11 => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_11,
        INIT_12 => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_12,
        INIT_13 => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_13,
        INIT_14 => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_14,
        INIT_15 => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_15,
        INIT_16 => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_16,
        INIT_17 => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_17,
        INIT_18 => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_18,
        INIT_19 => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_19,
        INIT_1A => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_1A,
        INIT_1B => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_1B,
        INIT_1C => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_1C,
        INIT_1D => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_1D,
        INIT_1E => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_1E,
        INIT_1F => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_1F,
        INIT_20 => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_20,
        INIT_21 => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_21,
        INIT_22 => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_22,
        INIT_23 => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_23,
        INIT_24 => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_24,
        INIT_25 => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_25,
        INIT_26 => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_26,
        INIT_27 => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_27,
        INIT_28 => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_28,
        INIT_29 => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_29,
        INIT_2A => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_2A,
        INIT_2B => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_2B,
        INIT_2C => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_2C,
        INIT_2D => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_2D,
        INIT_2E => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_2E,
        INIT_2F => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_2F,
        INIT_30 => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_30,
        INIT_31 => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_31,
        INIT_32 => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_32,
        INIT_33 => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_33,
        INIT_34 => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_34,
        INIT_35 => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_35,
        INIT_36 => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_36,
        INIT_37 => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_37,
        INIT_38 => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_38,
        INIT_39 => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_39,
        INIT_3A => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_3A,
        INIT_3B => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_3B,
        INIT_3C => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_3C,
        INIT_3D => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_3D,
        INIT_3E => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_3E,
        INIT_3F => cpu_0_u0_cmem0_ild0_ildramgen_6_ildram_INIT_3F )
-- pragma translate_on
                 port map (ildataout(13 downto 12), ildaddr, clk,
                          iddatain(13 downto 12), crami.icramin.ldramin.enable, '0', crami.icramin.ldramin.write);
      ildram7 : RAMB16_S2
-- pragma translate_off
generic map (
        INIT_00 => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_00,
        INIT_01 => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_01,
        INIT_02 => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_02,
        INIT_03 => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_03,
        INIT_04 => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_04,
        INIT_05 => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_05,
        INIT_06 => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_06,
        INIT_07 => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_07,
        INIT_08 => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_08,
        INIT_09 => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_09,
        INIT_0A => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_0A,
        INIT_0B => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_0B,
        INIT_0C => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_0C,
        INIT_0D => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_0D,
        INIT_0E => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_0E,
        INIT_0F => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_0F,
        INIT_10 => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_10,
        INIT_11 => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_11,
        INIT_12 => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_12,
        INIT_13 => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_13,
        INIT_14 => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_14,
        INIT_15 => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_15,
        INIT_16 => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_16,
        INIT_17 => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_17,
        INIT_18 => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_18,
        INIT_19 => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_19,
        INIT_1A => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_1A,
        INIT_1B => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_1B,
        INIT_1C => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_1C,
        INIT_1D => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_1D,
        INIT_1E => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_1E,
        INIT_1F => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_1F,
        INIT_20 => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_20,
        INIT_21 => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_21,
        INIT_22 => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_22,
        INIT_23 => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_23,
        INIT_24 => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_24,
        INIT_25 => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_25,
        INIT_26 => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_26,
        INIT_27 => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_27,
        INIT_28 => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_28,
        INIT_29 => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_29,
        INIT_2A => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_2A,
        INIT_2B => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_2B,
        INIT_2C => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_2C,
        INIT_2D => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_2D,
        INIT_2E => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_2E,
        INIT_2F => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_2F,
        INIT_30 => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_30,
        INIT_31 => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_31,
        INIT_32 => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_32,
        INIT_33 => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_33,
        INIT_34 => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_34,
        INIT_35 => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_35,
        INIT_36 => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_36,
        INIT_37 => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_37,
        INIT_38 => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_38,
        INIT_39 => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_39,
        INIT_3A => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_3A,
        INIT_3B => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_3B,
        INIT_3C => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_3C,
        INIT_3D => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_3D,
        INIT_3E => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_3E,
        INIT_3F => cpu_0_u0_cmem0_ild0_ildramgen_7_ildram_INIT_3F )
-- pragma translate_on
                 port map (ildataout(15 downto 14), ildaddr, clk,
                          iddatain(15 downto 14), crami.icramin.ldramin.enable, '0', crami.icramin.ldramin.write);


      ildram8 : RAMB16_S2
-- pragma translate_off
       generic map (
        INIT_00 => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_00,
        INIT_01 => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_01,
        INIT_02 => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_02,
        INIT_03 => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_03,
        INIT_04 => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_04,
        INIT_05 => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_05,
        INIT_06 => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_06,
        INIT_07 => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_07,
        INIT_08 => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_08,
        INIT_09 => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_09,
        INIT_0A => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_0A,
        INIT_0B => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_0B,
        INIT_0C => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_0C,
        INIT_0D => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_0D,
        INIT_0E => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_0E,
        INIT_0F => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_0F,
        INIT_10 => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_10,
        INIT_11 => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_11,
        INIT_12 => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_12,
        INIT_13 => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_13,
        INIT_14 => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_14,
        INIT_15 => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_15,
        INIT_16 => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_16,
        INIT_17 => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_17,
        INIT_18 => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_18,
        INIT_19 => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_19,
        INIT_1A => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_1A,
        INIT_1B => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_1B,
        INIT_1C => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_1C,
        INIT_1D => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_1D,
        INIT_1E => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_1E,
        INIT_1F => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_1F,
        INIT_20 => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_20,
        INIT_21 => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_21,
        INIT_22 => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_22,
        INIT_23 => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_23,
        INIT_24 => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_24,
        INIT_25 => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_25,
        INIT_26 => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_26,
        INIT_27 => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_27,
        INIT_28 => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_28,
        INIT_29 => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_29,
        INIT_2A => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_2A,
        INIT_2B => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_2B,
        INIT_2C => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_2C,
        INIT_2D => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_2D,
        INIT_2E => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_2E,
        INIT_2F => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_2F,
        INIT_30 => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_30,
        INIT_31 => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_31,
        INIT_32 => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_32,
        INIT_33 => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_33,
        INIT_34 => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_34,
        INIT_35 => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_35,
        INIT_36 => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_36,
        INIT_37 => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_37,
        INIT_38 => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_38,
        INIT_39 => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_39,
        INIT_3A => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_3A,
        INIT_3B => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_3B,
        INIT_3C => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_3C,
        INIT_3D => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_3D,
        INIT_3E => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_3E,
        INIT_3F => cpu_0_u0_cmem0_ild0_ildramgen_8_ildram_INIT_3F )
-- pragma translate_on
                 port map (ildataout(17 downto 16), ildaddr, clk,
                          iddatain(17 downto 16), crami.icramin.ldramin.enable, '0', crami.icramin.ldramin.write);
      ildram9 : RAMB16_S2
-- pragma translate_off
generic map (
        INIT_00 => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_00,
        INIT_01 => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_01,
        INIT_02 => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_02,
        INIT_03 => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_03,
        INIT_04 => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_04,
        INIT_05 => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_05,
        INIT_06 => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_06,
        INIT_07 => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_07,
        INIT_08 => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_08,
        INIT_09 => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_09,
        INIT_0A => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_0A,
        INIT_0B => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_0B,
        INIT_0C => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_0C,
        INIT_0D => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_0D,
        INIT_0E => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_0E,
        INIT_0F => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_0F,
        INIT_10 => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_10,
        INIT_11 => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_11,
        INIT_12 => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_12,
        INIT_13 => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_13,
        INIT_14 => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_14,
        INIT_15 => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_15,
        INIT_16 => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_16,
        INIT_17 => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_17,
        INIT_18 => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_18,
        INIT_19 => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_19,
        INIT_1A => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_1A,
        INIT_1B => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_1B,
        INIT_1C => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_1C,
        INIT_1D => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_1D,
        INIT_1E => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_1E,
        INIT_1F => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_1F,
        INIT_20 => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_20,
        INIT_21 => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_21,
        INIT_22 => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_22,
        INIT_23 => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_23,
        INIT_24 => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_24,
        INIT_25 => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_25,
        INIT_26 => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_26,
        INIT_27 => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_27,
        INIT_28 => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_28,
        INIT_29 => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_29,
        INIT_2A => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_2A,
        INIT_2B => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_2B,
        INIT_2C => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_2C,
        INIT_2D => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_2D,
        INIT_2E => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_2E,
        INIT_2F => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_2F,
        INIT_30 => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_30,
        INIT_31 => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_31,
        INIT_32 => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_32,
        INIT_33 => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_33,
        INIT_34 => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_34,
        INIT_35 => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_35,
        INIT_36 => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_36,
        INIT_37 => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_37,
        INIT_38 => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_38,
        INIT_39 => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_39,
        INIT_3A => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_3A,
        INIT_3B => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_3B,
        INIT_3C => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_3C,
        INIT_3D => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_3D,
        INIT_3E => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_3E,
        INIT_3F => cpu_0_u0_cmem0_ild0_ildramgen_9_ildram_INIT_3F )
-- pragma translate_on
                 port map (ildataout(19 downto 18), ildaddr, clk,
                          iddatain(19 downto 18), crami.icramin.ldramin.enable, '0', crami.icramin.ldramin.write);
      ildram10 : RAMB16_S2
-- pragma translate_off
generic map (
        INIT_00 => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_00,
        INIT_01 => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_01,
        INIT_02 => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_02,
        INIT_03 => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_03,
        INIT_04 => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_04,
        INIT_05 => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_05,
        INIT_06 => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_06,
        INIT_07 => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_07,
        INIT_08 => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_08,
        INIT_09 => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_09,
        INIT_0A => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_0A,
        INIT_0B => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_0B,
        INIT_0C => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_0C,
        INIT_0D => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_0D,
        INIT_0E => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_0E,
        INIT_0F => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_0F,
        INIT_10 => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_10,
        INIT_11 => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_11,
        INIT_12 => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_12,
        INIT_13 => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_13,
        INIT_14 => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_14,
        INIT_15 => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_15,
        INIT_16 => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_16,
        INIT_17 => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_17,
        INIT_18 => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_18,
        INIT_19 => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_19,
        INIT_1A => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_1A,
        INIT_1B => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_1B,
        INIT_1C => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_1C,
        INIT_1D => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_1D,
        INIT_1E => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_1E,
        INIT_1F => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_1F,
        INIT_20 => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_20,
        INIT_21 => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_21,
        INIT_22 => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_22,
        INIT_23 => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_23,
        INIT_24 => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_24,
        INIT_25 => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_25,
        INIT_26 => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_26,
        INIT_27 => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_27,
        INIT_28 => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_28,
        INIT_29 => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_29,
        INIT_2A => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_2A,
        INIT_2B => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_2B,
        INIT_2C => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_2C,
        INIT_2D => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_2D,
        INIT_2E => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_2E,
        INIT_2F => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_2F,
        INIT_30 => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_30,
        INIT_31 => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_31,
        INIT_32 => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_32,
        INIT_33 => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_33,
        INIT_34 => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_34,
        INIT_35 => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_35,
        INIT_36 => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_36,
        INIT_37 => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_37,
        INIT_38 => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_38,
        INIT_39 => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_39,
        INIT_3A => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_3A,
        INIT_3B => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_3B,
        INIT_3C => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_3C,
        INIT_3D => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_3D,
        INIT_3E => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_3E,
        INIT_3F => cpu_0_u0_cmem0_ild0_ildramgen_10_ildram_INIT_3F )
-- pragma translate_on
                 port map (ildataout(21 downto 20), ildaddr, clk,
                          iddatain(21 downto 20), crami.icramin.ldramin.enable, '0', crami.icramin.ldramin.write);
      ildram11 : RAMB16_S2
-- pragma translate_off
generic map (
        INIT_00 => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_00,
        INIT_01 => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_01,
        INIT_02 => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_02,
        INIT_03 => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_03,
        INIT_04 => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_04,
        INIT_05 => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_05,
        INIT_06 => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_06,
        INIT_07 => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_07,
        INIT_08 => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_08,
        INIT_09 => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_09,
        INIT_0A => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_0A,
        INIT_0B => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_0B,
        INIT_0C => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_0C,
        INIT_0D => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_0D,
        INIT_0E => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_0E,
        INIT_0F => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_0F,
        INIT_10 => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_10,
        INIT_11 => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_11,
        INIT_12 => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_12,
        INIT_13 => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_13,
        INIT_14 => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_14,
        INIT_15 => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_15,
        INIT_16 => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_16,
        INIT_17 => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_17,
        INIT_18 => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_18,
        INIT_19 => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_19,
        INIT_1A => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_1A,
        INIT_1B => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_1B,
        INIT_1C => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_1C,
        INIT_1D => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_1D,
        INIT_1E => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_1E,
        INIT_1F => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_1F,
        INIT_20 => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_20,
        INIT_21 => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_21,
        INIT_22 => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_22,
        INIT_23 => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_23,
        INIT_24 => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_24,
        INIT_25 => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_25,
        INIT_26 => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_26,
        INIT_27 => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_27,
        INIT_28 => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_28,
        INIT_29 => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_29,
        INIT_2A => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_2A,
        INIT_2B => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_2B,
        INIT_2C => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_2C,
        INIT_2D => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_2D,
        INIT_2E => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_2E,
        INIT_2F => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_2F,
        INIT_30 => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_30,
        INIT_31 => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_31,
        INIT_32 => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_32,
        INIT_33 => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_33,
        INIT_34 => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_34,
        INIT_35 => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_35,
        INIT_36 => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_36,
        INIT_37 => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_37,
        INIT_38 => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_38,
        INIT_39 => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_39,
        INIT_3A => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_3A,
        INIT_3B => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_3B,
        INIT_3C => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_3C,
        INIT_3D => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_3D,
        INIT_3E => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_3E,
        INIT_3F => cpu_0_u0_cmem0_ild0_ildramgen_11_ildram_INIT_3F )
-- pragma translate_on
                 port map (ildataout(23 downto 22), ildaddr, clk,
                          iddatain(23 downto 22), crami.icramin.ldramin.enable, '0', crami.icramin.ldramin.write);
      ildram12 : RAMB16_S2
-- pragma translate_off
generic map (
        INIT_00 => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_00,
        INIT_01 => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_01,
        INIT_02 => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_02,
        INIT_03 => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_03,
        INIT_04 => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_04,
        INIT_05 => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_05,
        INIT_06 => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_06,
        INIT_07 => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_07,
        INIT_08 => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_08,
        INIT_09 => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_09,
        INIT_0A => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_0A,
        INIT_0B => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_0B,
        INIT_0C => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_0C,
        INIT_0D => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_0D,
        INIT_0E => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_0E,
        INIT_0F => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_0F,
        INIT_10 => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_10,
        INIT_11 => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_11,
        INIT_12 => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_12,
        INIT_13 => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_13,
        INIT_14 => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_14,
        INIT_15 => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_15,
        INIT_16 => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_16,
        INIT_17 => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_17,
        INIT_18 => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_18,
        INIT_19 => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_19,
        INIT_1A => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_1A,
        INIT_1B => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_1B,
        INIT_1C => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_1C,
        INIT_1D => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_1D,
        INIT_1E => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_1E,
        INIT_1F => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_1F,
        INIT_20 => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_20,
        INIT_21 => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_21,
        INIT_22 => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_22,
        INIT_23 => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_23,
        INIT_24 => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_24,
        INIT_25 => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_25,
        INIT_26 => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_26,
        INIT_27 => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_27,
        INIT_28 => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_28,
        INIT_29 => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_29,
        INIT_2A => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_2A,
        INIT_2B => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_2B,
        INIT_2C => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_2C,
        INIT_2D => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_2D,
        INIT_2E => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_2E,
        INIT_2F => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_2F,
        INIT_30 => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_30,
        INIT_31 => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_31,
        INIT_32 => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_32,
        INIT_33 => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_33,
        INIT_34 => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_34,
        INIT_35 => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_35,
        INIT_36 => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_36,
        INIT_37 => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_37,
        INIT_38 => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_38,
        INIT_39 => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_39,
        INIT_3A => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_3A,
        INIT_3B => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_3B,
        INIT_3C => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_3C,
        INIT_3D => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_3D,
        INIT_3E => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_3E,
        INIT_3F => cpu_0_u0_cmem0_ild0_ildramgen_12_ildram_INIT_3F )
-- pragma translate_on
                 port map (ildataout(25 downto 24), ildaddr, clk,
                          iddatain(25 downto 24), crami.icramin.ldramin.enable, '0', crami.icramin.ldramin.write);
      ildram13 : RAMB16_S2
-- pragma translate_off
generic map (
        INIT_00 => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_00,
        INIT_01 => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_01,
        INIT_02 => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_02,
        INIT_03 => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_03,
        INIT_04 => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_04,
        INIT_05 => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_05,
        INIT_06 => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_06,
        INIT_07 => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_07,
        INIT_08 => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_08,
        INIT_09 => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_09,
        INIT_0A => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_0A,
        INIT_0B => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_0B,
        INIT_0C => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_0C,
        INIT_0D => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_0D,
        INIT_0E => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_0E,
        INIT_0F => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_0F,
        INIT_10 => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_10,
        INIT_11 => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_11,
        INIT_12 => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_12,
        INIT_13 => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_13,
        INIT_14 => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_14,
        INIT_15 => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_15,
        INIT_16 => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_16,
        INIT_17 => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_17,
        INIT_18 => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_18,
        INIT_19 => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_19,
        INIT_1A => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_1A,
        INIT_1B => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_1B,
        INIT_1C => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_1C,
        INIT_1D => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_1D,
        INIT_1E => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_1E,
        INIT_1F => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_1F,
        INIT_20 => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_20,
        INIT_21 => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_21,
        INIT_22 => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_22,
        INIT_23 => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_23,
        INIT_24 => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_24,
        INIT_25 => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_25,
        INIT_26 => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_26,
        INIT_27 => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_27,
        INIT_28 => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_28,
        INIT_29 => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_29,
        INIT_2A => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_2A,
        INIT_2B => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_2B,
        INIT_2C => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_2C,
        INIT_2D => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_2D,
        INIT_2E => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_2E,
        INIT_2F => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_2F,
        INIT_30 => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_30,
        INIT_31 => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_31,
        INIT_32 => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_32,
        INIT_33 => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_33,
        INIT_34 => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_34,
        INIT_35 => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_35,
        INIT_36 => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_36,
        INIT_37 => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_37,
        INIT_38 => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_38,
        INIT_39 => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_39,
        INIT_3A => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_3A,
        INIT_3B => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_3B,
        INIT_3C => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_3C,
        INIT_3D => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_3D,
        INIT_3E => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_3E,
        INIT_3F => cpu_0_u0_cmem0_ild0_ildramgen_13_ildram_INIT_3F )
-- pragma translate_on
                 port map (ildataout(27 downto 26), ildaddr, clk,
                          iddatain(27 downto 26), crami.icramin.ldramin.enable, '0', crami.icramin.ldramin.write);
      ildram14 : RAMB16_S2
-- pragma translate_off
generic map (
        INIT_00 => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_00,
        INIT_01 => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_01,
        INIT_02 => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_02,
        INIT_03 => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_03,
        INIT_04 => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_04,
        INIT_05 => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_05,
        INIT_06 => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_06,
        INIT_07 => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_07,
        INIT_08 => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_08,
        INIT_09 => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_09,
        INIT_0A => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_0A,
        INIT_0B => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_0B,
        INIT_0C => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_0C,
        INIT_0D => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_0D,
        INIT_0E => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_0E,
        INIT_0F => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_0F,
        INIT_10 => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_10,
        INIT_11 => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_11,
        INIT_12 => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_12,
        INIT_13 => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_13,
        INIT_14 => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_14,
        INIT_15 => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_15,
        INIT_16 => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_16,
        INIT_17 => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_17,
        INIT_18 => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_18,
        INIT_19 => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_19,
        INIT_1A => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_1A,
        INIT_1B => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_1B,
        INIT_1C => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_1C,
        INIT_1D => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_1D,
        INIT_1E => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_1E,
        INIT_1F => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_1F,
        INIT_20 => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_20,
        INIT_21 => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_21,
        INIT_22 => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_22,
        INIT_23 => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_23,
        INIT_24 => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_24,
        INIT_25 => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_25,
        INIT_26 => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_26,
        INIT_27 => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_27,
        INIT_28 => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_28,
        INIT_29 => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_29,
        INIT_2A => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_2A,
        INIT_2B => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_2B,
        INIT_2C => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_2C,
        INIT_2D => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_2D,
        INIT_2E => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_2E,
        INIT_2F => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_2F,
        INIT_30 => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_30,
        INIT_31 => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_31,
        INIT_32 => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_32,
        INIT_33 => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_33,
        INIT_34 => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_34,
        INIT_35 => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_35,
        INIT_36 => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_36,
        INIT_37 => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_37,
        INIT_38 => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_38,
        INIT_39 => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_39,
        INIT_3A => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_3A,
        INIT_3B => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_3B,
        INIT_3C => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_3C,
        INIT_3D => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_3D,
        INIT_3E => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_3E,
        INIT_3F => cpu_0_u0_cmem0_ild0_ildramgen_14_ildram_INIT_3F )
-- pragma translate_on
                 port map (ildataout(29 downto 28), ildaddr, clk,
                          iddatain(29 downto 28), crami.icramin.ldramin.enable, '0', crami.icramin.ldramin.write);
      ildram15 : RAMB16_S2
-- pragma translate_off
generic map (
        INIT_00 => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_00,
        INIT_01 => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_01,
        INIT_02 => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_02,
        INIT_03 => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_03,
        INIT_04 => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_04,
        INIT_05 => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_05,
        INIT_06 => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_06,
        INIT_07 => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_07,
        INIT_08 => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_08,
        INIT_09 => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_09,
        INIT_0A => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_0A,
        INIT_0B => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_0B,
        INIT_0C => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_0C,
        INIT_0D => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_0D,
        INIT_0E => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_0E,
        INIT_0F => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_0F,
        INIT_10 => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_10,
        INIT_11 => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_11,
        INIT_12 => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_12,
        INIT_13 => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_13,
        INIT_14 => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_14,
        INIT_15 => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_15,
        INIT_16 => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_16,
        INIT_17 => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_17,
        INIT_18 => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_18,
        INIT_19 => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_19,
        INIT_1A => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_1A,
        INIT_1B => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_1B,
        INIT_1C => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_1C,
        INIT_1D => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_1D,
        INIT_1E => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_1E,
        INIT_1F => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_1F,
        INIT_20 => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_20,
        INIT_21 => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_21,
        INIT_22 => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_22,
        INIT_23 => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_23,
        INIT_24 => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_24,
        INIT_25 => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_25,
        INIT_26 => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_26,
        INIT_27 => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_27,
        INIT_28 => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_28,
        INIT_29 => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_29,
        INIT_2A => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_2A,
        INIT_2B => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_2B,
        INIT_2C => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_2C,
        INIT_2D => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_2D,
        INIT_2E => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_2E,
        INIT_2F => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_2F,
        INIT_30 => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_30,
        INIT_31 => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_31,
        INIT_32 => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_32,
        INIT_33 => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_33,
        INIT_34 => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_34,
        INIT_35 => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_35,
        INIT_36 => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_36,
        INIT_37 => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_37,
        INIT_38 => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_38,
        INIT_39 => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_39,
        INIT_3A => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_3A,
        INIT_3B => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_3B,
        INIT_3C => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_3C,
        INIT_3D => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_3D,
        INIT_3E => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_3E,
        INIT_3F => cpu_0_u0_cmem0_ild0_ildramgen_15_ildram_INIT_3F )
-- pragma translate_on
                 port map (ildataout(31 downto 30), ildaddr, clk,
                          iddatain(31 downto 30), crami.icramin.ldramin.enable, '0', crami.icramin.ldramin.write);
    -- end generate;
  end generate;
  
  dme : if dcen = 1 generate
    dtags0 : if DSNOOP = 0 generate
      dt0 : for i in 0 to DSETS-1 generate
        dtags0 : syncram
        generic map (tech, DOFFSET_BITS, DTWIDTH)
        port map (clk, dtaddr, dtdatain(i)(DTWIDTH-1 downto 0), 
	    dtdataout(i)(DTWIDTH-1 downto 0), dtenable, dtwrite(i));
      end generate;
    end generate;

    dtags1 : if DSNOOP /= 0 generate
      dt1 : if ((MMUEN = 0) or not DSNOOPMMU) generate
        dt0 : for i in 0 to DSETS-1 generate
          dtags0 : syncram_dp
          generic map (tech, DOFFSET_BITS, DTWIDTH) port map (
	    clk, dtaddr, dtdatain(i)(DTWIDTH-1 downto 0), 
		dtdataout(i)(DTWIDTH-1 downto 0), dtenable, dtwrite(i),
            sclk, dtaddr2, dtdatain2(i)(DTWIDTH-1 downto 0), 
		dtdataout2(i)(DTWIDTH-1 downto 0), dtenable2, dtwrite2(i));
        end generate;
      end generate;
      mdt1 : if not ((MMUEN = 0) or not DSNOOPMMU) generate
        dt0 : for i in 0 to DSETS-1 generate
          dtags0 : syncram_dp
          generic map (tech, DOFFSET_BITS, DTWIDTH) port map (
	    clk, dtaddr, dtdatain(i)(DTWIDTH-1 downto 0), 
		dtdataout(i)(DTWIDTH-1 downto 0), dtenable, dtwrite(i),
            sclk, dtaddr2, dtdatain2(i)(DTWIDTH-1 downto 0), 
		dtdataout2(i)(DTWIDTH-1 downto 0), dtenable2, dtwrite2(i));
          dtags1 : syncram_dp
          generic map (tech, DOFFSET_BITS, DPTAG_BITS) port map (
            clk, dtaddr, dtdatain3(i)(DTAG_BITS-1 downto DTAG_BITS-DPTAG_BITS), 
               dtdataoutu(i)(DTAG_BITS-1 downto DTAG_BITS-DPTAG_BITS), dtenable3, dtwrite3(i),
            sclk, dtaddr2, dtdatainu(i)(DTAG_BITS-1 downto DTAG_BITS-DPTAG_BITS), 
               dtdataout3(i)(DTAG_BITS-1 downto DTAG_BITS-DPTAG_BITS), dtenable3, dtwrite2(i));
        end generate;
      end generate;
    end generate;
    nodtags1 : if DSNOOP = 0 generate
      dt0 : for i in 0 to DSETS-1 generate
        dtdataout2(i)(DTWIDTH-1 downto 0) <= zero64(DTWIDTH-1 downto 0);
        dtdataout3(i)(DTWIDTH-1 downto 0) <= zero64(DTWIDTH-1 downto 0);
      end generate;
    end generate;

    dd0 : for i in 0 to DSETS-1 generate
      ddata0 : syncram
       generic map (tech, DOFFSET_BITS+DLINE_BITS, DDWIDTH)
        port map (clk, ddaddr, dddatain(i), dddataout(i), ddenable, ddwrite(i));
    end generate;
  end generate;

  dmd : if dcen = 0 generate
    dnd0 : for i in 0 to DSETS-1 generate
      dtdataout(i) <= (others => '0');
      dtdataout2(i) <= (others => '0');
      dddataout(i) <= (others => '0');
    end generate;
  end generate;

  ldxs0 : if not ((dlram = 1) and (DSETS > 1)) generate
    lddatain <= dddatain(0);    
  end generate;
  
  ldxs1 : if (dlram = 1) and (DSETS > 1) generate
    lddatain <= dddatain(1);    
  end generate;

--   ld0 : if dlram = 1 generate
--     ldramgen : for i in 0 to 7 generate
--       ldram : RAMB16_S4 port map (ldataout((4*i)+3 downto (4*i)), ldaddr, clk,
--                           lddatain((4*i)+3 downto (4*i)), crami.dcramin.ldramin.enable, '0', crami.dcramin.ldramin.write);
--     end generate;
--   end generate;

  
  ld0 : if dlram = 1 generate
    -- ldramgen : for i in 0 to 7 generate
      ldram0 : RAMB16_S2
-- pragma translate_off
generic map (
        INIT_00 => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_00,
        INIT_01 => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_01,
        INIT_02 => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_02,
        INIT_03 => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_03,
        INIT_04 => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_04,
        INIT_05 => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_05,
        INIT_06 => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_06,
        INIT_07 => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_07,
        INIT_08 => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_08,
        INIT_09 => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_09,
        INIT_0A => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_0A,
        INIT_0B => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_0B,
        INIT_0C => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_0C,
        INIT_0D => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_0D,
        INIT_0E => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_0E,
        INIT_0F => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_0F,
        INIT_10 => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_10,
        INIT_11 => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_11,
        INIT_12 => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_12,
        INIT_13 => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_13,
        INIT_14 => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_14,
        INIT_15 => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_15,
        INIT_16 => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_16,
        INIT_17 => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_17,
        INIT_18 => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_18,
        INIT_19 => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_19,
        INIT_1A => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_1A,
        INIT_1B => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_1B,
        INIT_1C => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_1C,
        INIT_1D => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_1D,
        INIT_1E => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_1E,
        INIT_1F => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_1F,
        INIT_20 => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_20,
        INIT_21 => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_21,
        INIT_22 => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_22,
        INIT_23 => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_23,
        INIT_24 => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_24,
        INIT_25 => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_25,
        INIT_26 => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_26,
        INIT_27 => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_27,
        INIT_28 => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_28,
        INIT_29 => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_29,
        INIT_2A => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_2A,
        INIT_2B => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_2B,
        INIT_2C => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_2C,
        INIT_2D => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_2D,
        INIT_2E => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_2E,
        INIT_2F => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_2F,
        INIT_30 => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_30,
        INIT_31 => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_31,
        INIT_32 => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_32,
        INIT_33 => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_33,
        INIT_34 => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_34,
        INIT_35 => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_35,
        INIT_36 => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_36,
        INIT_37 => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_37,
        INIT_38 => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_38,
        INIT_39 => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_39,
        INIT_3A => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_3A,
        INIT_3B => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_3B,
        INIT_3C => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_3C,
        INIT_3D => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_3D,
        INIT_3E => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_3E,
        INIT_3F => cpu_0_u0_cmem0_ld0_ldramgen_0_ldram_INIT_3F )
-- pragma translate_on
                 port map (ldataout(1 downto 0), ldaddr, clk,
                          lddatain(1 downto 0), crami.dcramin.ldramin.enable, '0', crami.dcramin.ldramin.write);
      ldram1 : RAMB16_S2
-- pragma translate_off
generic map (
        INIT_00 => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_00,
        INIT_01 => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_01,
        INIT_02 => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_02,
        INIT_03 => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_03,
        INIT_04 => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_04,
        INIT_05 => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_05,
        INIT_06 => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_06,
        INIT_07 => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_07,
        INIT_08 => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_08,
        INIT_09 => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_09,
        INIT_0A => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_0A,
        INIT_0B => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_0B,
        INIT_0C => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_0C,
        INIT_0D => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_0D,
        INIT_0E => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_0E,
        INIT_0F => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_0F,
        INIT_10 => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_10,
        INIT_11 => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_11,
        INIT_12 => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_12,
        INIT_13 => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_13,
        INIT_14 => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_14,
        INIT_15 => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_15,
        INIT_16 => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_16,
        INIT_17 => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_17,
        INIT_18 => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_18,
        INIT_19 => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_19,
        INIT_1A => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_1A,
        INIT_1B => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_1B,
        INIT_1C => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_1C,
        INIT_1D => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_1D,
        INIT_1E => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_1E,
        INIT_1F => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_1F,
        INIT_20 => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_20,
        INIT_21 => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_21,
        INIT_22 => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_22,
        INIT_23 => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_23,
        INIT_24 => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_24,
        INIT_25 => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_25,
        INIT_26 => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_26,
        INIT_27 => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_27,
        INIT_28 => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_28,
        INIT_29 => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_29,
        INIT_2A => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_2A,
        INIT_2B => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_2B,
        INIT_2C => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_2C,
        INIT_2D => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_2D,
        INIT_2E => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_2E,
        INIT_2F => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_2F,
        INIT_30 => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_30,
        INIT_31 => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_31,
        INIT_32 => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_32,
        INIT_33 => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_33,
        INIT_34 => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_34,
        INIT_35 => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_35,
        INIT_36 => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_36,
        INIT_37 => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_37,
        INIT_38 => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_38,
        INIT_39 => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_39,
        INIT_3A => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_3A,
        INIT_3B => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_3B,
        INIT_3C => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_3C,
        INIT_3D => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_3D,
        INIT_3E => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_3E,
        INIT_3F => cpu_0_u0_cmem0_ld0_ldramgen_1_ldram_INIT_3F )
-- pragma translate_on
                 port map (ldataout(3 downto 2), ldaddr, clk,
                          lddatain(3 downto 2), crami.dcramin.ldramin.enable, '0', crami.dcramin.ldramin.write);
      ldram2 : RAMB16_S2
-- pragma translate_off
generic map (
        INIT_00 => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_00,
        INIT_01 => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_01,
        INIT_02 => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_02,
        INIT_03 => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_03,
        INIT_04 => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_04,
        INIT_05 => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_05,
        INIT_06 => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_06,
        INIT_07 => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_07,
        INIT_08 => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_08,
        INIT_09 => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_09,
        INIT_0A => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_0A,
        INIT_0B => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_0B,
        INIT_0C => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_0C,
        INIT_0D => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_0D,
        INIT_0E => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_0E,
        INIT_0F => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_0F,
        INIT_10 => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_10,
        INIT_11 => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_11,
        INIT_12 => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_12,
        INIT_13 => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_13,
        INIT_14 => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_14,
        INIT_15 => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_15,
        INIT_16 => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_16,
        INIT_17 => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_17,
        INIT_18 => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_18,
        INIT_19 => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_19,
        INIT_1A => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_1A,
        INIT_1B => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_1B,
        INIT_1C => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_1C,
        INIT_1D => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_1D,
        INIT_1E => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_1E,
        INIT_1F => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_1F,
        INIT_20 => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_20,
        INIT_21 => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_21,
        INIT_22 => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_22,
        INIT_23 => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_23,
        INIT_24 => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_24,
        INIT_25 => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_25,
        INIT_26 => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_26,
        INIT_27 => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_27,
        INIT_28 => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_28,
        INIT_29 => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_29,
        INIT_2A => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_2A,
        INIT_2B => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_2B,
        INIT_2C => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_2C,
        INIT_2D => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_2D,
        INIT_2E => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_2E,
        INIT_2F => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_2F,
        INIT_30 => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_30,
        INIT_31 => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_31,
        INIT_32 => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_32,
        INIT_33 => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_33,
        INIT_34 => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_34,
        INIT_35 => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_35,
        INIT_36 => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_36,
        INIT_37 => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_37,
        INIT_38 => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_38,
        INIT_39 => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_39,
        INIT_3A => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_3A,
        INIT_3B => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_3B,
        INIT_3C => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_3C,
        INIT_3D => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_3D,
        INIT_3E => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_3E,
        INIT_3F => cpu_0_u0_cmem0_ld0_ldramgen_2_ldram_INIT_3F )
-- pragma translate_on
                 port map (ldataout(5 downto 4), ldaddr, clk,
                          lddatain(5 downto 4), crami.dcramin.ldramin.enable, '0', crami.dcramin.ldramin.write);
      ldram3 : RAMB16_S2
-- pragma translate_off
generic map (
        INIT_00 => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_00,
        INIT_01 => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_01,
        INIT_02 => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_02,
        INIT_03 => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_03,
        INIT_04 => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_04,
        INIT_05 => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_05,
        INIT_06 => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_06,
        INIT_07 => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_07,
        INIT_08 => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_08,
        INIT_09 => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_09,
        INIT_0A => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_0A,
        INIT_0B => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_0B,
        INIT_0C => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_0C,
        INIT_0D => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_0D,
        INIT_0E => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_0E,
        INIT_0F => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_0F,
        INIT_10 => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_10,
        INIT_11 => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_11,
        INIT_12 => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_12,
        INIT_13 => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_13,
        INIT_14 => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_14,
        INIT_15 => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_15,
        INIT_16 => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_16,
        INIT_17 => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_17,
        INIT_18 => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_18,
        INIT_19 => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_19,
        INIT_1A => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_1A,
        INIT_1B => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_1B,
        INIT_1C => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_1C,
        INIT_1D => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_1D,
        INIT_1E => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_1E,
        INIT_1F => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_1F,
        INIT_20 => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_20,
        INIT_21 => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_21,
        INIT_22 => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_22,
        INIT_23 => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_23,
        INIT_24 => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_24,
        INIT_25 => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_25,
        INIT_26 => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_26,
        INIT_27 => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_27,
        INIT_28 => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_28,
        INIT_29 => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_29,
        INIT_2A => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_2A,
        INIT_2B => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_2B,
        INIT_2C => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_2C,
        INIT_2D => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_2D,
        INIT_2E => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_2E,
        INIT_2F => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_2F,
        INIT_30 => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_30,
        INIT_31 => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_31,
        INIT_32 => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_32,
        INIT_33 => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_33,
        INIT_34 => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_34,
        INIT_35 => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_35,
        INIT_36 => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_36,
        INIT_37 => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_37,
        INIT_38 => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_38,
        INIT_39 => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_39,
        INIT_3A => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_3A,
        INIT_3B => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_3B,
        INIT_3C => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_3C,
        INIT_3D => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_3D,
        INIT_3E => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_3E,
        INIT_3F => cpu_0_u0_cmem0_ld0_ldramgen_3_ldram_INIT_3F )
-- pragma translate_on
                 port map (ldataout(7 downto 6), ldaddr, clk,
                          lddatain(7 downto 6), crami.dcramin.ldramin.enable, '0', crami.dcramin.ldramin.write);
      ldram4 : RAMB16_S2
-- pragma translate_off
generic map (
        INIT_00 => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_00,
        INIT_01 => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_01,
        INIT_02 => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_02,
        INIT_03 => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_03,
        INIT_04 => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_04,
        INIT_05 => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_05,
        INIT_06 => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_06,
        INIT_07 => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_07,
        INIT_08 => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_08,
        INIT_09 => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_09,
        INIT_0A => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_0A,
        INIT_0B => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_0B,
        INIT_0C => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_0C,
        INIT_0D => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_0D,
        INIT_0E => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_0E,
        INIT_0F => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_0F,
        INIT_10 => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_10,
        INIT_11 => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_11,
        INIT_12 => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_12,
        INIT_13 => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_13,
        INIT_14 => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_14,
        INIT_15 => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_15,
        INIT_16 => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_16,
        INIT_17 => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_17,
        INIT_18 => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_18,
        INIT_19 => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_19,
        INIT_1A => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_1A,
        INIT_1B => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_1B,
        INIT_1C => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_1C,
        INIT_1D => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_1D,
        INIT_1E => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_1E,
        INIT_1F => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_1F,
        INIT_20 => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_20,
        INIT_21 => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_21,
        INIT_22 => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_22,
        INIT_23 => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_23,
        INIT_24 => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_24,
        INIT_25 => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_25,
        INIT_26 => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_26,
        INIT_27 => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_27,
        INIT_28 => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_28,
        INIT_29 => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_29,
        INIT_2A => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_2A,
        INIT_2B => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_2B,
        INIT_2C => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_2C,
        INIT_2D => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_2D,
        INIT_2E => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_2E,
        INIT_2F => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_2F,
        INIT_30 => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_30,
        INIT_31 => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_31,
        INIT_32 => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_32,
        INIT_33 => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_33,
        INIT_34 => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_34,
        INIT_35 => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_35,
        INIT_36 => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_36,
        INIT_37 => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_37,
        INIT_38 => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_38,
        INIT_39 => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_39,
        INIT_3A => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_3A,
        INIT_3B => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_3B,
        INIT_3C => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_3C,
        INIT_3D => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_3D,
        INIT_3E => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_3E,
        INIT_3F => cpu_0_u0_cmem0_ld0_ldramgen_4_ldram_INIT_3F )
-- pragma translate_on
                 port map (ldataout(9 downto 8), ldaddr, clk,
                          lddatain(9 downto 8), crami.dcramin.ldramin.enable, '0', crami.dcramin.ldramin.write);
      ldram5 : RAMB16_S2
-- pragma translate_off
generic map (
        INIT_00 => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_00,
        INIT_01 => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_01,
        INIT_02 => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_02,
        INIT_03 => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_03,
        INIT_04 => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_04,
        INIT_05 => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_05,
        INIT_06 => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_06,
        INIT_07 => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_07,
        INIT_08 => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_08,
        INIT_09 => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_09,
        INIT_0A => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_0A,
        INIT_0B => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_0B,
        INIT_0C => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_0C,
        INIT_0D => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_0D,
        INIT_0E => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_0E,
        INIT_0F => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_0F,
        INIT_10 => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_10,
        INIT_11 => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_11,
        INIT_12 => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_12,
        INIT_13 => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_13,
        INIT_14 => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_14,
        INIT_15 => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_15,
        INIT_16 => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_16,
        INIT_17 => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_17,
        INIT_18 => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_18,
        INIT_19 => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_19,
        INIT_1A => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_1A,
        INIT_1B => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_1B,
        INIT_1C => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_1C,
        INIT_1D => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_1D,
        INIT_1E => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_1E,
        INIT_1F => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_1F,
        INIT_20 => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_20,
        INIT_21 => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_21,
        INIT_22 => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_22,
        INIT_23 => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_23,
        INIT_24 => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_24,
        INIT_25 => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_25,
        INIT_26 => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_26,
        INIT_27 => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_27,
        INIT_28 => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_28,
        INIT_29 => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_29,
        INIT_2A => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_2A,
        INIT_2B => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_2B,
        INIT_2C => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_2C,
        INIT_2D => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_2D,
        INIT_2E => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_2E,
        INIT_2F => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_2F,
        INIT_30 => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_30,
        INIT_31 => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_31,
        INIT_32 => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_32,
        INIT_33 => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_33,
        INIT_34 => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_34,
        INIT_35 => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_35,
        INIT_36 => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_36,
        INIT_37 => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_37,
        INIT_38 => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_38,
        INIT_39 => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_39,
        INIT_3A => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_3A,
        INIT_3B => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_3B,
        INIT_3C => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_3C,
        INIT_3D => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_3D,
        INIT_3E => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_3E,
        INIT_3F => cpu_0_u0_cmem0_ld0_ldramgen_5_ldram_INIT_3F )
-- pragma translate_on
                 port map (ldataout(11 downto 10), ldaddr, clk,
                          lddatain(11 downto 10), crami.dcramin.ldramin.enable, '0', crami.dcramin.ldramin.write);
      ldram6 : RAMB16_S2
-- pragma translate_off
generic map (
        INIT_00 => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_00,
        INIT_01 => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_01,
        INIT_02 => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_02,
        INIT_03 => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_03,
        INIT_04 => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_04,
        INIT_05 => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_05,
        INIT_06 => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_06,
        INIT_07 => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_07,
        INIT_08 => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_08,
        INIT_09 => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_09,
        INIT_0A => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_0A,
        INIT_0B => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_0B,
        INIT_0C => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_0C,
        INIT_0D => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_0D,
        INIT_0E => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_0E,
        INIT_0F => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_0F,
        INIT_10 => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_10,
        INIT_11 => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_11,
        INIT_12 => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_12,
        INIT_13 => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_13,
        INIT_14 => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_14,
        INIT_15 => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_15,
        INIT_16 => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_16,
        INIT_17 => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_17,
        INIT_18 => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_18,
        INIT_19 => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_19,
        INIT_1A => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_1A,
        INIT_1B => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_1B,
        INIT_1C => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_1C,
        INIT_1D => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_1D,
        INIT_1E => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_1E,
        INIT_1F => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_1F,
        INIT_20 => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_20,
        INIT_21 => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_21,
        INIT_22 => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_22,
        INIT_23 => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_23,
        INIT_24 => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_24,
        INIT_25 => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_25,
        INIT_26 => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_26,
        INIT_27 => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_27,
        INIT_28 => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_28,
        INIT_29 => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_29,
        INIT_2A => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_2A,
        INIT_2B => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_2B,
        INIT_2C => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_2C,
        INIT_2D => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_2D,
        INIT_2E => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_2E,
        INIT_2F => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_2F,
        INIT_30 => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_30,
        INIT_31 => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_31,
        INIT_32 => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_32,
        INIT_33 => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_33,
        INIT_34 => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_34,
        INIT_35 => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_35,
        INIT_36 => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_36,
        INIT_37 => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_37,
        INIT_38 => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_38,
        INIT_39 => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_39,
        INIT_3A => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_3A,
        INIT_3B => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_3B,
        INIT_3C => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_3C,
        INIT_3D => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_3D,
        INIT_3E => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_3E,
        INIT_3F => cpu_0_u0_cmem0_ld0_ldramgen_6_ldram_INIT_3F )
-- pragma translate_on
                 port map (ldataout(13 downto 12), ldaddr, clk,
                          lddatain(13 downto 12), crami.dcramin.ldramin.enable, '0', crami.dcramin.ldramin.write);
      ldram7 : RAMB16_S2
-- pragma translate_off
generic map (
        INIT_00 => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_00,
        INIT_01 => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_01,
        INIT_02 => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_02,
        INIT_03 => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_03,
        INIT_04 => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_04,
        INIT_05 => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_05,
        INIT_06 => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_06,
        INIT_07 => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_07,
        INIT_08 => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_08,
        INIT_09 => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_09,
        INIT_0A => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_0A,
        INIT_0B => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_0B,
        INIT_0C => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_0C,
        INIT_0D => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_0D,
        INIT_0E => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_0E,
        INIT_0F => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_0F,
        INIT_10 => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_10,
        INIT_11 => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_11,
        INIT_12 => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_12,
        INIT_13 => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_13,
        INIT_14 => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_14,
        INIT_15 => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_15,
        INIT_16 => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_16,
        INIT_17 => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_17,
        INIT_18 => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_18,
        INIT_19 => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_19,
        INIT_1A => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_1A,
        INIT_1B => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_1B,
        INIT_1C => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_1C,
        INIT_1D => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_1D,
        INIT_1E => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_1E,
        INIT_1F => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_1F,
        INIT_20 => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_20,
        INIT_21 => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_21,
        INIT_22 => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_22,
        INIT_23 => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_23,
        INIT_24 => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_24,
        INIT_25 => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_25,
        INIT_26 => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_26,
        INIT_27 => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_27,
        INIT_28 => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_28,
        INIT_29 => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_29,
        INIT_2A => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_2A,
        INIT_2B => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_2B,
        INIT_2C => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_2C,
        INIT_2D => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_2D,
        INIT_2E => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_2E,
        INIT_2F => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_2F,
        INIT_30 => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_30,
        INIT_31 => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_31,
        INIT_32 => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_32,
        INIT_33 => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_33,
        INIT_34 => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_34,
        INIT_35 => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_35,
        INIT_36 => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_36,
        INIT_37 => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_37,
        INIT_38 => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_38,
        INIT_39 => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_39,
        INIT_3A => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_3A,
        INIT_3B => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_3B,
        INIT_3C => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_3C,
        INIT_3D => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_3D,
        INIT_3E => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_3E,
        INIT_3F => cpu_0_u0_cmem0_ld0_ldramgen_7_ldram_INIT_3F )
-- pragma translate_on
                 port map (ldataout(15 downto 14), ldaddr, clk,
                          lddatain(15 downto 14), crami.dcramin.ldramin.enable, '0', crami.dcramin.ldramin.write);


      ldram8 : RAMB16_S2
-- pragma translate_off
generic map (
        INIT_00 => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_00,
        INIT_01 => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_01,
        INIT_02 => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_02,
        INIT_03 => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_03,
        INIT_04 => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_04,
        INIT_05 => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_05,
        INIT_06 => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_06,
        INIT_07 => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_07,
        INIT_08 => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_08,
        INIT_09 => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_09,
        INIT_0A => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_0A,
        INIT_0B => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_0B,
        INIT_0C => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_0C,
        INIT_0D => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_0D,
        INIT_0E => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_0E,
        INIT_0F => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_0F,
        INIT_10 => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_10,
        INIT_11 => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_11,
        INIT_12 => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_12,
        INIT_13 => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_13,
        INIT_14 => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_14,
        INIT_15 => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_15,
        INIT_16 => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_16,
        INIT_17 => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_17,
        INIT_18 => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_18,
        INIT_19 => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_19,
        INIT_1A => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_1A,
        INIT_1B => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_1B,
        INIT_1C => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_1C,
        INIT_1D => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_1D,
        INIT_1E => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_1E,
        INIT_1F => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_1F,
        INIT_20 => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_20,
        INIT_21 => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_21,
        INIT_22 => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_22,
        INIT_23 => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_23,
        INIT_24 => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_24,
        INIT_25 => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_25,
        INIT_26 => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_26,
        INIT_27 => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_27,
        INIT_28 => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_28,
        INIT_29 => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_29,
        INIT_2A => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_2A,
        INIT_2B => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_2B,
        INIT_2C => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_2C,
        INIT_2D => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_2D,
        INIT_2E => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_2E,
        INIT_2F => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_2F,
        INIT_30 => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_30,
        INIT_31 => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_31,
        INIT_32 => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_32,
        INIT_33 => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_33,
        INIT_34 => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_34,
        INIT_35 => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_35,
        INIT_36 => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_36,
        INIT_37 => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_37,
        INIT_38 => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_38,
        INIT_39 => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_39,
        INIT_3A => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_3A,
        INIT_3B => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_3B,
        INIT_3C => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_3C,
        INIT_3D => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_3D,
        INIT_3E => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_3E,
        INIT_3F => cpu_0_u0_cmem0_ld0_ldramgen_8_ldram_INIT_3F )
-- pragma translate_on
                 port map (ldataout(17 downto 16), ldaddr, clk,
                          lddatain(17 downto 16), crami.dcramin.ldramin.enable, '0', crami.dcramin.ldramin.write);
      ldram9 : RAMB16_S2
-- pragma translate_off
generic map (
        INIT_00 => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_00,
        INIT_01 => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_01,
        INIT_02 => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_02,
        INIT_03 => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_03,
        INIT_04 => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_04,
        INIT_05 => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_05,
        INIT_06 => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_06,
        INIT_07 => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_07,
        INIT_08 => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_08,
        INIT_09 => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_09,
        INIT_0A => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_0A,
        INIT_0B => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_0B,
        INIT_0C => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_0C,
        INIT_0D => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_0D,
        INIT_0E => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_0E,
        INIT_0F => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_0F,
        INIT_10 => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_10,
        INIT_11 => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_11,
        INIT_12 => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_12,
        INIT_13 => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_13,
        INIT_14 => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_14,
        INIT_15 => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_15,
        INIT_16 => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_16,
        INIT_17 => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_17,
        INIT_18 => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_18,
        INIT_19 => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_19,
        INIT_1A => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_1A,
        INIT_1B => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_1B,
        INIT_1C => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_1C,
        INIT_1D => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_1D,
        INIT_1E => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_1E,
        INIT_1F => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_1F,
        INIT_20 => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_20,
        INIT_21 => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_21,
        INIT_22 => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_22,
        INIT_23 => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_23,
        INIT_24 => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_24,
        INIT_25 => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_25,
        INIT_26 => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_26,
        INIT_27 => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_27,
        INIT_28 => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_28,
        INIT_29 => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_29,
        INIT_2A => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_2A,
        INIT_2B => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_2B,
        INIT_2C => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_2C,
        INIT_2D => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_2D,
        INIT_2E => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_2E,
        INIT_2F => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_2F,
        INIT_30 => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_30,
        INIT_31 => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_31,
        INIT_32 => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_32,
        INIT_33 => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_33,
        INIT_34 => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_34,
        INIT_35 => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_35,
        INIT_36 => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_36,
        INIT_37 => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_37,
        INIT_38 => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_38,
        INIT_39 => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_39,
        INIT_3A => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_3A,
        INIT_3B => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_3B,
        INIT_3C => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_3C,
        INIT_3D => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_3D,
        INIT_3E => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_3E,
        INIT_3F => cpu_0_u0_cmem0_ld0_ldramgen_9_ldram_INIT_3F )
-- pragma translate_on
                 port map (ldataout(19 downto 18), ldaddr, clk,
                          lddatain(19 downto 18), crami.dcramin.ldramin.enable, '0', crami.dcramin.ldramin.write);
      ldram10 : RAMB16_S2
-- pragma translate_off
generic map (
        INIT_00 => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_00,
        INIT_01 => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_01,
        INIT_02 => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_02,
        INIT_03 => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_03,
        INIT_04 => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_04,
        INIT_05 => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_05,
        INIT_06 => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_06,
        INIT_07 => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_07,
        INIT_08 => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_08,
        INIT_09 => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_09,
        INIT_0A => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_0A,
        INIT_0B => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_0B,
        INIT_0C => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_0C,
        INIT_0D => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_0D,
        INIT_0E => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_0E,
        INIT_0F => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_0F,
        INIT_10 => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_10,
        INIT_11 => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_11,
        INIT_12 => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_12,
        INIT_13 => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_13,
        INIT_14 => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_14,
        INIT_15 => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_15,
        INIT_16 => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_16,
        INIT_17 => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_17,
        INIT_18 => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_18,
        INIT_19 => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_19,
        INIT_1A => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_1A,
        INIT_1B => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_1B,
        INIT_1C => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_1C,
        INIT_1D => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_1D,
        INIT_1E => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_1E,
        INIT_1F => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_1F,
        INIT_20 => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_20,
        INIT_21 => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_21,
        INIT_22 => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_22,
        INIT_23 => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_23,
        INIT_24 => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_24,
        INIT_25 => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_25,
        INIT_26 => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_26,
        INIT_27 => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_27,
        INIT_28 => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_28,
        INIT_29 => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_29,
        INIT_2A => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_2A,
        INIT_2B => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_2B,
        INIT_2C => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_2C,
        INIT_2D => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_2D,
        INIT_2E => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_2E,
        INIT_2F => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_2F,
        INIT_30 => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_30,
        INIT_31 => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_31,
        INIT_32 => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_32,
        INIT_33 => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_33,
        INIT_34 => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_34,
        INIT_35 => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_35,
        INIT_36 => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_36,
        INIT_37 => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_37,
        INIT_38 => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_38,
        INIT_39 => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_39,
        INIT_3A => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_3A,
        INIT_3B => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_3B,
        INIT_3C => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_3C,
        INIT_3D => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_3D,
        INIT_3E => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_3E,
        INIT_3F => cpu_0_u0_cmem0_ld0_ldramgen_10_ldram_INIT_3F )
-- pragma translate_on
                 port map (ldataout(21 downto 20), ldaddr, clk,
                          lddatain(21 downto 20), crami.dcramin.ldramin.enable, '0', crami.dcramin.ldramin.write);
      ldram11 : RAMB16_S2
-- pragma translate_off
generic map (
        INIT_00 => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_00,
        INIT_01 => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_01,
        INIT_02 => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_02,
        INIT_03 => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_03,
        INIT_04 => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_04,
        INIT_05 => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_05,
        INIT_06 => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_06,
        INIT_07 => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_07,
        INIT_08 => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_08,
        INIT_09 => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_09,
        INIT_0A => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_0A,
        INIT_0B => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_0B,
        INIT_0C => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_0C,
        INIT_0D => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_0D,
        INIT_0E => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_0E,
        INIT_0F => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_0F,
        INIT_10 => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_10,
        INIT_11 => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_11,
        INIT_12 => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_12,
        INIT_13 => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_13,
        INIT_14 => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_14,
        INIT_15 => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_15,
        INIT_16 => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_16,
        INIT_17 => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_17,
        INIT_18 => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_18,
        INIT_19 => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_19,
        INIT_1A => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_1A,
        INIT_1B => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_1B,
        INIT_1C => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_1C,
        INIT_1D => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_1D,
        INIT_1E => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_1E,
        INIT_1F => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_1F,
        INIT_20 => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_20,
        INIT_21 => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_21,
        INIT_22 => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_22,
        INIT_23 => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_23,
        INIT_24 => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_24,
        INIT_25 => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_25,
        INIT_26 => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_26,
        INIT_27 => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_27,
        INIT_28 => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_28,
        INIT_29 => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_29,
        INIT_2A => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_2A,
        INIT_2B => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_2B,
        INIT_2C => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_2C,
        INIT_2D => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_2D,
        INIT_2E => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_2E,
        INIT_2F => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_2F,
        INIT_30 => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_30,
        INIT_31 => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_31,
        INIT_32 => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_32,
        INIT_33 => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_33,
        INIT_34 => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_34,
        INIT_35 => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_35,
        INIT_36 => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_36,
        INIT_37 => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_37,
        INIT_38 => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_38,
        INIT_39 => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_39,
        INIT_3A => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_3A,
        INIT_3B => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_3B,
        INIT_3C => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_3C,
        INIT_3D => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_3D,
        INIT_3E => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_3E,
        INIT_3F => cpu_0_u0_cmem0_ld0_ldramgen_11_ldram_INIT_3F )
-- pragma translate_on
                 port map (ldataout(23 downto 22), ldaddr, clk,
                          lddatain(23 downto 22), crami.dcramin.ldramin.enable, '0', crami.dcramin.ldramin.write);
      ldram12 : RAMB16_S2
-- pragma translate_off
generic map (
        INIT_00 => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_00,
        INIT_01 => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_01,
        INIT_02 => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_02,
        INIT_03 => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_03,
        INIT_04 => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_04,
        INIT_05 => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_05,
        INIT_06 => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_06,
        INIT_07 => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_07,
        INIT_08 => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_08,
        INIT_09 => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_09,
        INIT_0A => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_0A,
        INIT_0B => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_0B,
        INIT_0C => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_0C,
        INIT_0D => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_0D,
        INIT_0E => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_0E,
        INIT_0F => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_0F,
        INIT_10 => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_10,
        INIT_11 => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_11,
        INIT_12 => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_12,
        INIT_13 => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_13,
        INIT_14 => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_14,
        INIT_15 => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_15,
        INIT_16 => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_16,
        INIT_17 => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_17,
        INIT_18 => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_18,
        INIT_19 => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_19,
        INIT_1A => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_1A,
        INIT_1B => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_1B,
        INIT_1C => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_1C,
        INIT_1D => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_1D,
        INIT_1E => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_1E,
        INIT_1F => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_1F,
        INIT_20 => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_20,
        INIT_21 => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_21,
        INIT_22 => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_22,
        INIT_23 => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_23,
        INIT_24 => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_24,
        INIT_25 => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_25,
        INIT_26 => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_26,
        INIT_27 => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_27,
        INIT_28 => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_28,
        INIT_29 => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_29,
        INIT_2A => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_2A,
        INIT_2B => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_2B,
        INIT_2C => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_2C,
        INIT_2D => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_2D,
        INIT_2E => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_2E,
        INIT_2F => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_2F,
        INIT_30 => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_30,
        INIT_31 => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_31,
        INIT_32 => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_32,
        INIT_33 => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_33,
        INIT_34 => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_34,
        INIT_35 => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_35,
        INIT_36 => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_36,
        INIT_37 => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_37,
        INIT_38 => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_38,
        INIT_39 => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_39,
        INIT_3A => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_3A,
        INIT_3B => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_3B,
        INIT_3C => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_3C,
        INIT_3D => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_3D,
        INIT_3E => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_3E,
        INIT_3F => cpu_0_u0_cmem0_ld0_ldramgen_12_ldram_INIT_3F )
-- pragma translate_on
                 port map (ldataout(25 downto 24), ldaddr, clk,
                          lddatain(25 downto 24), crami.dcramin.ldramin.enable, '0', crami.dcramin.ldramin.write);
      ldram13 : RAMB16_S2
-- pragma translate_off
generic map (
        INIT_00 => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_00,
        INIT_01 => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_01,
        INIT_02 => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_02,
        INIT_03 => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_03,
        INIT_04 => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_04,
        INIT_05 => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_05,
        INIT_06 => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_06,
        INIT_07 => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_07,
        INIT_08 => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_08,
        INIT_09 => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_09,
        INIT_0A => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_0A,
        INIT_0B => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_0B,
        INIT_0C => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_0C,
        INIT_0D => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_0D,
        INIT_0E => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_0E,
        INIT_0F => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_0F,
        INIT_10 => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_10,
        INIT_11 => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_11,
        INIT_12 => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_12,
        INIT_13 => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_13,
        INIT_14 => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_14,
        INIT_15 => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_15,
        INIT_16 => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_16,
        INIT_17 => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_17,
        INIT_18 => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_18,
        INIT_19 => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_19,
        INIT_1A => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_1A,
        INIT_1B => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_1B,
        INIT_1C => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_1C,
        INIT_1D => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_1D,
        INIT_1E => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_1E,
        INIT_1F => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_1F,
        INIT_20 => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_20,
        INIT_21 => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_21,
        INIT_22 => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_22,
        INIT_23 => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_23,
        INIT_24 => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_24,
        INIT_25 => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_25,
        INIT_26 => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_26,
        INIT_27 => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_27,
        INIT_28 => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_28,
        INIT_29 => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_29,
        INIT_2A => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_2A,
        INIT_2B => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_2B,
        INIT_2C => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_2C,
        INIT_2D => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_2D,
        INIT_2E => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_2E,
        INIT_2F => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_2F,
        INIT_30 => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_30,
        INIT_31 => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_31,
        INIT_32 => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_32,
        INIT_33 => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_33,
        INIT_34 => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_34,
        INIT_35 => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_35,
        INIT_36 => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_36,
        INIT_37 => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_37,
        INIT_38 => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_38,
        INIT_39 => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_39,
        INIT_3A => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_3A,
        INIT_3B => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_3B,
        INIT_3C => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_3C,
        INIT_3D => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_3D,
        INIT_3E => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_3E,
        INIT_3F => cpu_0_u0_cmem0_ld0_ldramgen_13_ldram_INIT_3F )
-- pragma translate_on
                 port map (ldataout(27 downto 26), ldaddr, clk,
                          lddatain(27 downto 26), crami.dcramin.ldramin.enable, '0', crami.dcramin.ldramin.write);
      ldram14 : RAMB16_S2
-- pragma translate_off
generic map (
        INIT_00 => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_00,
        INIT_01 => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_01,
        INIT_02 => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_02,
        INIT_03 => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_03,
        INIT_04 => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_04,
        INIT_05 => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_05,
        INIT_06 => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_06,
        INIT_07 => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_07,
        INIT_08 => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_08,
        INIT_09 => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_09,
        INIT_0A => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_0A,
        INIT_0B => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_0B,
        INIT_0C => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_0C,
        INIT_0D => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_0D,
        INIT_0E => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_0E,
        INIT_0F => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_0F,
        INIT_10 => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_10,
        INIT_11 => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_11,
        INIT_12 => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_12,
        INIT_13 => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_13,
        INIT_14 => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_14,
        INIT_15 => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_15,
        INIT_16 => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_16,
        INIT_17 => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_17,
        INIT_18 => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_18,
        INIT_19 => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_19,
        INIT_1A => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_1A,
        INIT_1B => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_1B,
        INIT_1C => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_1C,
        INIT_1D => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_1D,
        INIT_1E => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_1E,
        INIT_1F => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_1F,
        INIT_20 => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_20,
        INIT_21 => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_21,
        INIT_22 => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_22,
        INIT_23 => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_23,
        INIT_24 => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_24,
        INIT_25 => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_25,
        INIT_26 => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_26,
        INIT_27 => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_27,
        INIT_28 => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_28,
        INIT_29 => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_29,
        INIT_2A => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_2A,
        INIT_2B => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_2B,
        INIT_2C => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_2C,
        INIT_2D => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_2D,
        INIT_2E => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_2E,
        INIT_2F => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_2F,
        INIT_30 => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_30,
        INIT_31 => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_31,
        INIT_32 => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_32,
        INIT_33 => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_33,
        INIT_34 => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_34,
        INIT_35 => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_35,
        INIT_36 => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_36,
        INIT_37 => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_37,
        INIT_38 => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_38,
        INIT_39 => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_39,
        INIT_3A => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_3A,
        INIT_3B => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_3B,
        INIT_3C => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_3C,
        INIT_3D => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_3D,
        INIT_3E => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_3E,
        INIT_3F => cpu_0_u0_cmem0_ld0_ldramgen_14_ldram_INIT_3F )
-- pragma translate_on
                 port map (ldataout(29 downto 28), ldaddr, clk,
                          lddatain(29 downto 28), crami.dcramin.ldramin.enable, '0', crami.dcramin.ldramin.write);
      ldram15 : RAMB16_S2
-- pragma translate_off
generic map (
        INIT_00 => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_00,
        INIT_01 => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_01,
        INIT_02 => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_02,
        INIT_03 => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_03,
        INIT_04 => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_04,
        INIT_05 => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_05,
        INIT_06 => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_06,
        INIT_07 => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_07,
        INIT_08 => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_08,
        INIT_09 => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_09,
        INIT_0A => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_0A,
        INIT_0B => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_0B,
        INIT_0C => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_0C,
        INIT_0D => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_0D,
        INIT_0E => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_0E,
        INIT_0F => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_0F,
        INIT_10 => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_10,
        INIT_11 => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_11,
        INIT_12 => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_12,
        INIT_13 => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_13,
        INIT_14 => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_14,
        INIT_15 => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_15,
        INIT_16 => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_16,
        INIT_17 => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_17,
        INIT_18 => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_18,
        INIT_19 => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_19,
        INIT_1A => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_1A,
        INIT_1B => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_1B,
        INIT_1C => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_1C,
        INIT_1D => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_1D,
        INIT_1E => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_1E,
        INIT_1F => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_1F,
        INIT_20 => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_20,
        INIT_21 => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_21,
        INIT_22 => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_22,
        INIT_23 => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_23,
        INIT_24 => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_24,
        INIT_25 => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_25,
        INIT_26 => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_26,
        INIT_27 => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_27,
        INIT_28 => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_28,
        INIT_29 => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_29,
        INIT_2A => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_2A,
        INIT_2B => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_2B,
        INIT_2C => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_2C,
        INIT_2D => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_2D,
        INIT_2E => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_2E,
        INIT_2F => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_2F,
        INIT_30 => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_30,
        INIT_31 => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_31,
        INIT_32 => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_32,
        INIT_33 => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_33,
        INIT_34 => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_34,
        INIT_35 => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_35,
        INIT_36 => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_36,
        INIT_37 => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_37,
        INIT_38 => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_38,
        INIT_39 => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_39,
        INIT_3A => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_3A,
        INIT_3B => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_3B,
        INIT_3C => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_3C,
        INIT_3D => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_3D,
        INIT_3E => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_3E,
        INIT_3F => cpu_0_u0_cmem0_ld0_ldramgen_15_ldram_INIT_3F )
-- pragma translate_on
                 port map (ldataout(31 downto 30), ldaddr, clk,
                          lddatain(31 downto 30), crami.dcramin.ldramin.enable, '0', crami.dcramin.ldramin.write);
    -- end generate;
  end generate;

  itx : for i in 0 to ISETS-1 generate
    cramo.icramo.tag(i)(TAG_HIGH downto ITAG_LOW) <= itdataout(i)(ITAG_BITS-1 downto (ITAG_BITS-1) - (TAG_HIGH - ITAG_LOW));
    --(ITWIDTH-1-(ILRR_BIT+ICLOCK_BIT) downto ITWIDTH-(TAG_HIGH-ITAG_LOW)-(ILRR_BIT+ICLOCK_BIT)-1);    
    cramo.icramo.tag(i)(ilinesize-1 downto 0) <= itdataout(i)(ilinesize-1 downto 0);
    cramo.icramo.tag(i)(CTAG_LRRPOS) <= itdataout(i)(ITWIDTH - (1+ICLOCK_BIT));
    cramo.icramo.tag(i)(CTAG_LOCKPOS) <= itdataout(i)(ITWIDTH-1);     
    ictx : if mmuen = 1 generate
      cramo.icramo.ctx(i) <= itdataout(i)((ITWIDTH - (ILRR_BIT+ICLOCK_BIT+1)) downto (ITWIDTH - (ILRR_BIT+ICLOCK_BIT+M_CTX_SZ)));
    end generate;
    cramo.icramo.data(i) <= ildataout when (ilram = 1) and ((ISETS = 1) or (i = 1)) and (crami.icramin.ldramin.read = '1') else iddataout(i)(31 downto 0);
    itv : if ilinesize = 4 generate
      cramo.icramo.tag(i)(7 downto 4) <= (others => '0');
    end generate;
    ite : for j in 10 to ITAG_LOW-1 generate
      cramo.icramo.tag(i)(j) <= '0';
    end generate;
  end generate;

  itx2 : for i in ISETS to MAXSETS-1 generate
    cramo.icramo.tag(i) <= (others => '0');
    cramo.icramo.data(i) <= (others => '0');
  end generate;


  itd : for i in 0 to DSETS-1 generate
    cramo.dcramo.tag(i)(TAG_HIGH downto DTAG_LOW) <= dtdataout(i)(DTAG_BITS-1 downto (DTAG_BITS-1) - (TAG_HIGH - DTAG_LOW));
    --(DTWIDTH-1-(DLRR_BIT+DCLOCK_BIT) downto DTWIDTH-(TAG_HIGH-DTAG_LOW)-(DLRR_BIT+DCLOCK_BIT)-1);
    --cramo.dcramo.tag(i)(TAG_HIGH downto DTAG_LOW) <= dtdataout(i)(DTWIDTH-1-(DLRR_BIT+DCLOCK_BIT) downto DTWIDTH-(TAG_HIGH-DTAG_LOW)-(DLRR_BIT+DCLOCK_BIT)-1);
    cramo.dcramo.tag(i)(dlinesize-1 downto 0) <= dtdataout(i)(dlinesize-1 downto 0);
    cramo.dcramo.tag(i)(CTAG_LRRPOS) <= dtdataout(i)(DTWIDTH - (1+DCLOCK_BIT));
    cramo.dcramo.tag(i)(CTAG_LOCKPOS) <= dtdataout(i)(DTWIDTH-1);     
    ictx : if mmuen /= 0 generate
      cramo.dcramo.ctx(i) <= dtdataout(i)((DTWIDTH - (DLRR_BIT+DCLOCK_BIT+1)) downto (DTWIDTH - (DLRR_BIT+DCLOCK_BIT+M_CTX_SZ)));
    end generate;
    
    stagv : if not ((MMUEN = 0) or not DSNOOPMMU) generate
      cramo.dcramo.stag(i)(TAG_HIGH downto DTAG_LOW) <= dtdataout3(i)(DTAG_BITS-1 downto (DTAG_BITS-1) - (TAG_HIGH - DTAG_LOW));
    end generate;
    stagp : if ((MMUEN = 0) or not DSNOOPMMU) generate
      cramo.dcramo.stag(i)(TAG_HIGH downto DTAG_LOW) <= dtdataout2(i)(DTAG_BITS-1 downto (DTAG_BITS-1) - (TAG_HIGH - DTAG_LOW));
    end generate;
    
--    cramo.dcramo.stag(i)(TAG_HIGH downto DTAG_LOW) <= dtdataout2(i)(DTWIDTH-1 downto DTWIDTH-(TAG_HIGH-DTAG_LOW)-1);
    cramo.dcramo.stag(i)(dlinesize-1 downto 0) <= dtdataout2(i)(dlinesize-1 downto 0);
    cramo.dcramo.stag(i)(CTAG_LRRPOS) <= dtdataout2(i)(DTWIDTH - (1+DCLOCK_BIT));
    cramo.dcramo.stag(i)(CTAG_LOCKPOS) <= dtdataout2(i)(DTWIDTH-1);     
    cramo.dcramo.data(i) <= ldataout when (dlram = 1) and ((DSETS = 1) or (i = 1)) and (crami.dcramin.ldramin.read = '1')
    else dddataout(i)(31 downto 0);
    dtv : if dlinesize = 4 generate
      cramo.dcramo.tag(i)(7 downto 4) <= (others => '0');
      cramo.dcramo.stag(i)(7 downto 4) <= (others => '0');
    end generate;
    dte : for j in 10 to DTAG_LOW-1 generate
      cramo.dcramo.tag(i)(j) <= '0';
      cramo.dcramo.stag(i)(j) <= '0';
    end generate;
  end generate;

  itd2 : for i in DSETS to MAXSETS-1 generate
    cramo.dcramo.tag(i) <= (others => '0');
    cramo.dcramo.stag(i) <= (others => '0');
    cramo.dcramo.data(i) <= (others => '0');
  end generate;

end ;

