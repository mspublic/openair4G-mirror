----------------------------------------------------------------------------
--  This file is a part of the GRLIB VHDL IP LIBRARY
--  Copyright (C) 2005 GAISLER RESEARCH
--
--  This program is free software; you can redistribute it and/or modify
--  it under the terms of the GNU General Public License as published by
--  the Free Software Foundation; either version 2 of the License, or
--  (at your option) any later version.
--
--  See the file COPYING for the full details of the license.
--
-----------------------------------------------------------------------------
-- Entity: 	grfpwx
-- File:	grfpwx.vhd
-- Author:	Edvin Catovic - Gaisler Research
-- Description:	Meiko/MFPC wrapper and FP register file
------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
library fpu;
use fpu.libfpu.all;
library techmap;
use techmap.gencomp.all;

entity mfpwx is
  generic (tech     : integer := 0;
           pclow    : integer range 0 to 2 := 2;
           dsu      : integer range 0 to 1 := 0;           
           disas    : integer range 0 to 1 := 0);
  port (
    rst    : in  std_ulogic;			-- Reset
    clk    : in  std_ulogic;
    holdn  : in  std_ulogic;			-- pipeline hold
    cpi    : in  fpc_in_type;
    cpo    : out fpc_out_type
    );
end;

architecture rtl of mfpwx is

  signal rfi1, rfi2  : fp_rf_in_type;
  signal rfo1, rfo2  : fp_rf_out_type;

begin

   mfpw0 : mfpw generic map (pclow, dsu, disas)
   port map (
    rst          ,
    clk          ,
    holdn        , 
    cpi.flush    ,
    cpi.exack    ,
    cpi.a_rs1    ,
    cpi.d.pc     ,
    cpi.d.inst   ,
    cpi.d.cnt    ,
    cpi.d.trap   ,
    cpi.d.annul  ,
    cpi.d.pv     ,
    cpi.a.pc     ,
    cpi.a.inst   ,
    cpi.a.cnt    ,
    cpi.a.trap   ,
    cpi.a.annul  ,
    cpi.a.pv     ,
    cpi.e.pc     ,
    cpi.e.inst   ,
    cpi.e.cnt    ,
    cpi.e.trap   ,
    cpi.e.annul  ,
    cpi.e.pv     ,
    cpi.m.pc     ,
    cpi.m.inst   ,
    cpi.m.cnt    ,
    cpi.m.trap   ,
    cpi.m.annul  ,
    cpi.m.pv     ,
    cpi.x.pc     ,
    cpi.x.inst   ,
    cpi.x.cnt    ,
    cpi.x.trap   ,
    cpi.x.annul  ,
    cpi.x.pv     ,
    cpi.lddata   ,
    cpi.dbg.enable  ,
    cpi.dbg.write   ,
    cpi.dbg.fsr     ,
    cpi.dbg.addr    ,
    cpi.dbg.data    ,

    cpo.data        ,
    cpo.exc  	    ,
    cpo.cc          ,
    cpo.ccv  	    ,
    cpo.ldlock      ,
    cpo.holdn       ,
    cpo.dbg.data    ,

    rfi1.rd1addr    ,
    rfi1.rd2addr     ,
    rfi1.wraddr      ,
    rfi1.wrdata      ,
    rfi1.ren1        ,
    rfi1.ren2        ,
    rfi1.wren        ,

    rfi2.rd1addr     ,
    rfi2.rd2addr     ,
    rfi2.wraddr       ,
    rfi2.wrdata       ,
    rfi2.ren1         ,
    rfi2.ren2         ,
    rfi2.wren         ,        

    rfo1.data1        ,
    rfo1.data2        ,
    rfo2.data1        ,
    rfo2.data2        
    );

   

   rf1 : regfile_3p generic map (tech, 4, 32, 1, 16)
     port map (clk, rfi1.wraddr, rfi1.wrdata, rfi1.wren, clk, rfi1.rd1addr, rfi1.ren1, rfo1.data1,
               rfi1.rd2addr, rfi1.ren2, rfo1.data2);

   rf2 : regfile_3p generic map (tech, 4, 32, 1, 16)
     port map (clk, rfi2.wraddr, rfi2.wrdata, rfi2.wren, clk, rfi2.rd1addr, rfi2.ren1, rfo2.data1,
               rfi2.rd2addr, rfi2.ren2, rfo2.data2);

   rfi1.wpar <= (others => '0'); rfi1.wecb <= (others => '0'); rfi1.einj1 <= '0'; rfi1.einj2 <= '0';
   rfi2.wpar <= (others => '0'); rfi2.wecb <= (others => '0'); rfi2.einj1 <= '0'; rfi2.einj2 <= '0';      
   rfo1.decb1 <= (others => '0'); rfo1.dpar1 <= (others => '0'); rfo1.decb2 <= (others => '0'); rfo1.dpar2 <= (others => '0');
   rfo2.decb1 <= (others => '0'); rfo2.dpar1 <= (others => '0'); rfo2.decb2 <= (others => '0'); rfo2.dpar2 <= (others => '0');              
   cpo.restart <= '0';
   
end;
