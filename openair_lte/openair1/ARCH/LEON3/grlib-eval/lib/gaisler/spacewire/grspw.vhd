---------------------------------------------------------------------------
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
-- Entity: 	grspw
-- File:	grspw.vhd
-- Author:	Marko Isomaki - Gaisler Research 
-- Description: GRLIB wrapper for grspw core
------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
library grlib;
use grlib.amba.all;
library techmap;
use techmap.gencomp.all;
use grlib.stdlib.all;
use grlib.devices.all;
library gaisler;
use gaisler.spacewire.all;
library spw;
use spw.spwcomp.all;

entity grspw is
  generic(
    tech         : integer range 0 to NTECH     := inferred;
    hindex       : integer range 0 to NAHBMST-1 := 0;
    pindex       : integer range 0 to NAPBSLV-1 := 0;
    paddr        : integer range 0 to 16#FFF#   := 0;
    pmask        : integer range 0 to 16#FFF#   := 16#FFF#;
    pirq         : integer range 0 to NAHBIRQ-1 := 0;
    sysfreq      : integer := 10000;
    usegen       : integer range 0 to 1  := 1;
    nsync        : integer range 1 to 2  := 1; 
    rmap         : integer range 0 to 1  := 0;
    rmapcrc      : integer range 0 to 1  := 0;
    fifosize1    : integer range 4 to 32 := 32;
    fifosize2    : integer range 16 to 64 := 64;
    rxclkbuftype : integer range 0 to 2 := 0;
    rxunaligned  : integer range 0 to 1 := 0;
    rmapbufs     : integer range 2 to 8 := 4;
    ft           : integer range 0 to 2 := 0
  );
  port(
    rst        : in  std_ulogic;
    clk        : in  std_ulogic;
    txclk      : in  std_ulogic;
    ahbmi      : in  ahb_mst_in_type;
    ahbmo      : out ahb_mst_out_type;
    apbi       : in  apb_slv_in_type;
    apbo       : out apb_slv_out_type;
    swni       : in  grspw_in_type;
    swno       : out grspw_out_type
  );
end entity;

architecture rtl of grspw is
  constant fabits1      : integer := log2(fifosize1);
  constant fabits2      : integer := log2(fifosize2);
  constant rfifo        : integer := 5 + log2(rmapbufs);
  constant REVISION     : integer := 0; 
  constant pconfig      : apb_config_type := (
    0 => ahb_device_reg ( VENDOR_GAISLER, GAISLER_SPW, 0, REVISION, pirq),
    1 => apb_iobar(paddr, pmask));

  constant hconfig : ahb_config_type := (
    0 => ahb_device_reg ( VENDOR_GAISLER, GAISLER_SPW, 0, 0, 0),
  others => zero32);

  signal txclki, txclko, rxclki, rxclko : std_ulogic;
  --rx ahb fifo
  signal rxrenable    : std_ulogic;
  signal rxraddress   : std_logic_vector(4 downto 0);
  signal rxwrite      : std_ulogic;
  signal rxwdata      : std_logic_vector(31 downto 0);
  signal rxwaddress   : std_logic_vector(4 downto 0);
  signal rxrdata      : std_logic_vector(31 downto 0);    
  --tx ahb fifo
  signal txrenable    : std_ulogic;
  signal txraddress   : std_logic_vector(4 downto 0);
  signal txwrite      : std_ulogic;
  signal txwdata      : std_logic_vector(31 downto 0);
  signal txwaddress   : std_logic_vector(4 downto 0);
  signal txrdata      : std_logic_vector(31 downto 0);    
  --nchar fifo
  signal ncrenable    : std_ulogic;
  signal ncraddress   : std_logic_vector(5 downto 0);
  signal ncwrite      : std_ulogic;
  signal ncwdata      : std_logic_vector(8 downto 0);
  signal ncwaddress   : std_logic_vector(5 downto 0);
  signal ncrdata      : std_logic_vector(8 downto 0);
  --rmap buf
  signal rmrenable    : std_ulogic;
  signal rmraddress   : std_logic_vector(7 downto 0);
  signal rmwrite      : std_ulogic;
  signal rmwdata      : std_logic_vector(7 downto 0);
  signal rmwaddress   : std_logic_vector(7 downto 0);
  signal rmrdata      : std_logic_vector(7 downto 0);
  --misc
  signal irq          : std_ulogic;
  
begin  
  grspwc0 : grspwc 
    generic map(
      sysfreq      => sysfreq,
      usegen       => usegen,
      nsync        => nsync,
      rmap         => rmap,
      rmapcrc      => rmapcrc,
      fifosize1    => fifosize1,
      fifosize2    => fifosize2,
      rxunaligned  => rxunaligned,
      rmapbufs     => rmapbufs)
    port map(
      rst          => rst,
      clk          => clk,
      txclk        => txclk,
      --ahb mst in
      hgrant       => ahbmi.hgrant(hindex),
      hready       => ahbmi.hready,   
      hresp        => ahbmi.hresp,
      hrdata       => ahbmi.hrdata,
      --ahb mst out
      hbusreq      => ahbmo.hbusreq,
      hlock        => ahbmo.hlock,
      htrans       => ahbmo.htrans,
      haddr        => ahbmo.haddr,
      hwrite       => ahbmo.hwrite,
      hsize        => ahbmo.hsize,
      hburst       => ahbmo.hburst,
      hprot        => ahbmo.hprot,
      hwdata       => ahbmo.hwdata,
      --apb slv in 
      psel	   => apbi.psel(pindex),
      penable	   => apbi.penable,
      paddr	   => apbi.paddr,
      pwrite	   => apbi.pwrite,
      pwdata	   => apbi.pwdata,
      --apb slv out
      prdata       => apbo.prdata,
      --spw in
      di           => swni.d,
      si           => swni.s,
      --spw out
      do           => swno.d,
      so           => swno.s,
      --time iface
      tickin       => swni.tickin,
      tickout      => swno.tickout,
      --clk bufs
      rxclki       => rxclki,
      rxclko       => rxclko,
      --irq
      irq          => irq,
      --misc     
      clkdiv10     => swni.clkdiv10,
      dcrstval     => swni.dcrstval,
      timerrstval  => swni.timerrstval,
      --rmapen    
      rmapen       => swni.rmapen, 
      --rx ahb fifo
      rxrenable    => rxrenable,
      rxraddress   => rxraddress, 
      rxwrite      => rxwrite,
      rxwdata      => rxwdata, 
      rxwaddress   => rxwaddress,
      rxrdata      => rxrdata,  
      --tx ahb fifo
      txrenable    => txrenable,
      txraddress   => txraddress, 
      txwrite      => txwrite,
      txwdata      => txwdata, 
      txwaddress   => txwaddress,
      txrdata      => txrdata,  
      --nchar fifo
      ncrenable    => ncrenable,
      ncraddress   => ncraddress, 
      ncwrite      => ncwrite,
      ncwdata      => ncwdata, 
      ncwaddress   => ncwaddress,
      ncrdata      => ncrdata,  
      --rmap buf
      rmrenable    => rmrenable,
      rmraddress   => rmraddress, 
      rmwrite      => rmwrite,
      rmwdata      => rmwdata, 
      rmwaddress   => rmwaddress,
      rmrdata      => rmrdata
      );
 
  irqdrv : process(irq)
  begin
    apbo.pirq        <= (others => '0');
    apbo.pirq(pirq)  <= irq;
  end process;

  ahbmo.hirq   	   <= (others => '0');
  ahbmo.hconfig    <= hconfig;
  ahbmo.hindex     <= hindex;
  
  apbo.pconfig <= pconfig;
  apbo.pindex  <= pindex;

  rx_clkbuf : techbuf generic map(tech => tech, buftype => rxclkbuftype)
    port map(i => rxclko, o => rxclki);

  ------------------------------------------------------------------------------
  -- FIFOS ---------------------------------------------------------------------
  ------------------------------------------------------------------------------

  nft : if ft = 0 generate
    --receiver AHB FIFO
    rx_ram0 : syncram_2p generic map(tech, fabits1, 32)
    port map(clk, rxrenable, rxraddress(fabits1-1 downto 0),
      rxrdata, clk, rxwrite,
      rxwaddress(fabits1-1 downto 0), rxwdata);
  
    --receiver nchar FIFO
    rx_ram1 : syncram_2p generic map(tech, fabits2, 9)
    port map(clk, ncrenable, ncraddress(fabits2-1 downto 0),
      ncrdata, clk, ncwrite,
      ncwaddress(fabits2-1 downto 0), ncwdata);
    
    --transmitter FIFO
    tx_ram0 : syncram_2p generic map(tech, fabits1, 32)
    port map(clk, txrenable, txraddress(fabits1-1 downto 0),
      txrdata, clk, txwrite, txwaddress(fabits1-1 downto 0), txwdata);

    --RMAP Buffer
    rmap_ram : if (rmap = 1) generate
      ram0 : syncram_2p generic map(tech, rfifo, 8)
      port map(clk, rmrenable, rmraddress(rfifo-1 downto 0),
        rmrdata, clk, rmwrite, rmwaddress(rfifo-1 downto 0),
        rmwdata);
    end generate;
  end generate;

  ft1 : if ft /= 0 generate
    --receiver AHB FIFO
    rx_ram0 : syncram_2pft generic map(tech, fabits1, 32, 0, 0, ft)
    port map(clk, rxrenable, rxraddress(fabits1-1 downto 0),
      rxrdata, clk, rxwrite,
      rxwaddress(fabits1-1 downto 0), rxwdata);
  
    --receiver nchar FIFO
    rx_ram1 : syncram_2p generic map(0, fabits2, 9)
    port map(clk, ncrenable, ncraddress(fabits2-1 downto 0),
      ncrdata, clk, ncwrite,
      ncwaddress(fabits2-1 downto 0), ncwdata);
    
    --transmitter FIFO
    tx_ram0 : syncram_2pft generic map(tech, fabits1, 32, 0, 0, ft)
    port map(clk, txrenable, txraddress(fabits1-1 downto 0),
      txrdata, clk, txwrite, txwaddress(fabits1-1 downto 0), txwdata);

    --RMAP Buffer
    rmap_ram : if (rmap = 1) generate
      ram0 : syncram_2pft generic map(tech, rfifo, 8, 0, 0, ft)
      port map(clk, rmrenable, rmraddress(rfifo-1 downto 0),
        rmrdata, clk, rmwrite, rmwaddress(rfifo-1 downto 0),
        rmwdata);
    end generate;
  end generate;

-- pragma translate_off
    msg0 : if (rmap = 0) generate
      bootmsg : report_version
        generic map ("grspw" & tost(pindex) &
	  ": Spacewire link rev " & tost(REVISION) & ", AHB fifos 2x" &
          tost(fifosize1*4)  & " bytes, rx fifo " & tost(fifosize2) &
         " bytes, irq " & tost(pirq));
    end generate;

    msg1 : if (rmap = 1) generate
      bootmsg : report_version
        generic map ("grspw" & tost(pindex) &
	  ": Spacewire link rev " & tost(REVISION) & ", AHB fifos 2x " &
          tost(fifosize1*4)  & " bytes, rx fifo " & tost(fifosize2) &
         " bytes, irq " & tost(pirq) & " , RMAP Buffer " &
         tost(rmapbufs*32) & " bytes");
    end generate;

    pr0 : process is
    begin
      wait for 100 ns;
      if sysfreq < 10000 then
        print("WARNING: System frequency too low for GRSPW");
      end if;
      wait;
    end process;
  
-- pragma translate_on
 
end architecture;
