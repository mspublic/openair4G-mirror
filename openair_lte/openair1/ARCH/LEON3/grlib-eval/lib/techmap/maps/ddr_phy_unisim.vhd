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
-- Entity: 	various
-- File:	clkgen_xilinx.vhd
-- Author:	Jiri Gaisler, Gaisler Research
-- Description:	DDR PHY for Virtex-2 and Virtex-4
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library grlib;
use grlib.stdlib.all;
-- pragma translate_off
library unisim;
use unisim.BUFG;
use unisim.DCM;
use unisim.ODDR;
use unisim.FD;
-- pragma translate_on

library techmap;
use techmap.gencomp.all;

------------------------------------------------------------------
-- Virtex4 DDR PHY -----------------------------------------------
------------------------------------------------------------------

entity virtex4_ddr_phy is
  generic (MHz : integer := 100; rstdelay : integer := 200;
	dbits : integer := 16; clk_mul : integer := 2 ;
	clk_div : integer := 2; rskew : integer := 0);

  port (
    rst       : in  std_ulogic;
    clk       : in  std_logic;          	-- input clock
    clkout    : out std_ulogic;			-- system clock
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
    cke       	: in  std_logic_vector(1 downto 0)
  );

end;

architecture rtl of virtex4_ddr_phy is

  component DCM
    generic (
      CLKDV_DIVIDE : real := 2.0;
      CLKFX_DIVIDE : integer := 1;
      CLKFX_MULTIPLY : integer := 4;
      CLKIN_DIVIDE_BY_2 : boolean := false;
      CLKIN_PERIOD : real := 10.0;
      CLKOUT_PHASE_SHIFT : string := "NONE";
      CLK_FEEDBACK : string := "1X";
      DESKEW_ADJUST : string := "SYSTEM_SYNCHRONOUS";
      DFS_FREQUENCY_MODE : string := "LOW";
      DLL_FREQUENCY_MODE : string := "LOW";
      DSS_MODE : string := "NONE";
      DUTY_CYCLE_CORRECTION : boolean := true;
      FACTORY_JF : bit_vector := X"C080";
      PHASE_SHIFT : integer := 0;
      STARTUP_WAIT : boolean := false 
    );
    port (
      CLKFB    : in  std_logic;
      CLKIN    : in  std_logic;
      DSSEN    : in  std_logic;
      PSCLK    : in  std_logic;
      PSEN     : in  std_logic;
      PSINCDEC : in  std_logic;
      RST      : in  std_logic;
      CLK0     : out std_logic;
      CLK90    : out std_logic;
      CLK180   : out std_logic;
      CLK270   : out std_logic;
      CLK2X    : out std_logic;
      CLK2X180 : out std_logic;
      CLKDV    : out std_logic;
      CLKFX    : out std_logic;
      CLKFX180 : out std_logic;
      LOCKED   : out std_logic;
      PSDONE   : out std_logic;
      STATUS   : out std_logic_vector (7 downto 0));
  end component;
  component BUFG port (O : out std_logic; I : in std_logic); end component;
  component ODDR
    generic
      ( DDR_CLK_EDGE : string := "OPPOSITE_EDGE";
--        INIT : bit := '0';
        SRTYPE : string := "SYNC");
    port
      (
        Q : out std_ulogic;
        C : in std_ulogic;
        CE : in std_ulogic;
        D1 : in std_ulogic;
        D2 : in std_ulogic;
        R : in std_ulogic;
        S : in std_ulogic
      );
  end component;
  component FD
	generic ( INIT : bit := '0');
	port (  Q : out std_ulogic;
		C : in std_ulogic;
		D : in std_ulogic);
  end component;
    component IDDR
      generic ( DDR_CLK_EDGE : string := "SAME_EDGE";
          INIT_Q1 : bit := '0';
          INIT_Q2 : bit := '0';
          SRTYPE : string := "ASYNC");
      port
        ( Q1 : out std_ulogic;
          Q2 : out std_ulogic;
          C : in std_ulogic;
          CE : in std_ulogic;
          D : in std_ulogic;
          R : in std_ulogic;
          S : in std_ulogic);
    end component;

signal vcc, gnd, dqsn, oe, lockl : std_ulogic;
signal ddr_clk_fb_outr : std_ulogic;
signal ddr_clk_fbl, fbclk : std_ulogic;
signal ddr_rasnr, ddr_casnr, ddr_wenr : std_ulogic;
signal ddr_clkl, ddr_clkbl : std_logic_vector(2 downto 0);
signal ddr_csnr, ddr_ckenr, ckel : std_logic_vector(1 downto 0);
signal clk_0ro, clk_90ro, clk_180ro, clk_270ro : std_ulogic;
signal clk_0r, clk_90r, clk_180r, clk_270r : std_ulogic;
signal clk0r, clk90r, clk180r, clk270r : std_ulogic;
signal locked, vlockl, ddrclkfbl, dllfb : std_ulogic;

signal ddr_dqin  	: std_logic_vector (dbits-1 downto 0); -- ddr data
signal ddr_dqout  	: std_logic_vector (dbits-1 downto 0); -- ddr data
signal ddr_dqoen  	: std_logic_vector (dbits-1 downto 0); -- ddr data
signal ddr_adr      	: std_logic_vector (13 downto 0);   -- ddr address
signal ddr_bar      	: std_logic_vector (1 downto 0);   -- ddr address
signal ddr_dmr      	: std_logic_vector (dbits/8-1 downto 0);   -- ddr address
signal ddr_dqsin  	: std_logic_vector (dbits/8-1 downto 0);    -- ddr dqs
signal ddr_dqsoen 	: std_logic_vector (dbits/8-1 downto 0);    -- ddr dqs
signal ddr_dqsoutl 	: std_logic_vector (dbits/8-1 downto 0);    -- ddr dqs
signal dqsdel, dqsclk 	: std_logic_vector (dbits/8-1 downto 0);    -- ddr dqs
signal da     		: std_logic_vector (dbits-1 downto 0); -- ddr data
signal dqinl		: std_logic_vector (dbits-1 downto 0); -- ddr data
signal dllrst		: std_logic_vector(0 to 3);
signal dll0rst, dll2rst	: std_logic_vector(0 to 3);
signal mlock, mclkfb, mclk, mclkfx, mclk0 : std_ulogic;
signal rclk270b, rclk90b, rclk0b : std_ulogic;
signal rclk270, rclk90, rclk0 : std_ulogic;

constant DDR_FREQ : integer := (MHz * clk_mul) / clk_div;

begin

  oe <= not oen;
  vcc <= '1'; gnd <= '0';

  -- Optional DDR clock multiplication

  noclkscale : if clk_mul = clk_div generate
    mclk <= clk;
  end generate;

  clkscale : if clk_mul /= clk_div generate

    rstdel : process (clk, rst)
    begin
        if rst = '0' then dll0rst <= (others => '1');
        elsif rising_edge(clk) then
  	  dll0rst <= dll0rst(1 to 3) & '0';
        end if;
    end process;

    bufg0 : BUFG port map (I => mclkfx, O => mclk);
    bufg1 : BUFG port map (I => mclk0, O => mclkfb);

    dllm : DCM
      generic map (CLKFX_MULTIPLY => clk_mul, CLKFX_DIVIDE => clk_div)
      port map ( CLKIN => clk, CLKFB => mclkfb, DSSEN => gnd, PSCLK => gnd,
      PSEN => gnd, PSINCDEC => gnd, RST => dll0rst(0), CLK0 => mclk0,
      LOCKED => mlock, CLKFX => mclkfx );
  end generate;

  -- DDR clock generation

  ddrref_pad : clkpad generic map (tech => virtex4) 
	port map (ddr_clk_fb, ddrclkfbl); 
  
  bufg1 : BUFG port map (I => clk_0ro, O => clk_0r);
--  bufg2 : BUFG port map (I => clk_90ro, O => clk_90r);
  clk_90r <= not clk_270r;
--  bufg3 : BUFG port map (I => clk_180ro, O => clk_180r);
  clk_180r <= not clk_0r;
  bufg4 : BUFG port map (I => clk_270ro, O => clk_270r);

  clkout <= clk_270r; clk0r <= clk_270r; clk90r <= clk_0r;
  clk180r <= clk_90r; clk270r <= clk_180r;

  
  dllfb <= clk_0r;

  dll : DCM generic map (CLKFX_MULTIPLY => 2, CLKFX_DIVIDE => 2)
      port map ( CLKIN => mclk, CLKFB => dllfb, DSSEN => gnd, PSCLK => gnd,
      PSEN => gnd, PSINCDEC => gnd, RST => dllrst(0), CLK0 => clk_0ro, 
      CLK90 => clk_90ro, CLK180 => clk_180ro, CLK270 => clk_270ro, 
      LOCKED => lockl);

  rstdel : process (mclk, rst)
  begin
      if rst = '0' then dllrst <= (others => '1');
      elsif rising_edge(mclk) then
	dllrst <= dllrst(1 to 3) & '0';
      end if;
  end process;

  rdel : if rstdelay /= 0 generate
    rcnt : process (clk_0r)
    variable cnt : std_logic_vector(15 downto 0);
    variable vlock, co : std_ulogic;
    begin
      if rising_edge(clk_0r) then
        co := cnt(15);
        vlockl <= vlock;
        if lockl = '0' then
	  cnt := conv_std_logic_vector(rstdelay*DDR_FREQ, 16); vlock := '0';
        else
	  if vlock = '0' then
	    cnt := cnt -1;  vlock := cnt(15) and not co;
	  end if;
        end if;
      end if;
      if lockl = '0' then
	vlock := '0';
      end if;
    end process;
  end generate;

  locked <= lockl when rstdelay = 0 else vlockl;
  lock <= locked;

  -- Generate external DDR clock

  fbdclk0r : ODDR port map ( Q => ddr_clk_fb_outr, C => clk90r, CE => vcc,
		D1 => vcc, D2 => gnd, R => gnd, S => gnd);
  fbclk_pad : outpad generic map (tech => virtex4, level => sstl2_i) 
	port map (ddr_clk_fb_out, ddr_clk_fb_outr);

  ddrclocks : for i in 0 to 2 generate
    dclk0r : ODDR port map ( Q => ddr_clkl(i), C => clk90r, CE => vcc,
		D1 => vcc, D2 => gnd, R => gnd, S => gnd);
    ddrclk_pad : outpad generic map (tech => virtex4, level => sstl2_i) 
	port map (ddr_clk(i), ddr_clkl(i));

    dclk0rb : ODDR port map ( Q => ddr_clkbl(i), C => clk90r, CE => vcc,
		D1 => gnd, D2 => vcc, R => gnd, S => gnd);
    ddrclkb_pad : outpad generic map (tech => virtex4, level => sstl2_i) 
	port map (ddr_clkb(i), ddr_clkbl(i));
  end generate;

  ddrbanks : for i in 0 to 1 generate
    csn0gen : ODDR  generic map (DDR_CLK_EDGE => "SAME_EDGE")
	port map ( Q => ddr_csnr(i), C => clk0r, CE => vcc,
		D1 => csn(i), D2 => csn(i), R => gnd, S => gnd);
    csn0_pad : outpad generic map (tech => virtex4, level => sstl2_i) 
	port map (ddr_csb(i), ddr_csnr(i));

    ckel(i) <= cke(i) and locked;
    ckegen : ODDR  generic map (DDR_CLK_EDGE => "SAME_EDGE")
	port map ( Q => ddr_ckenr(i), C => clk0r, CE => vcc,
		D1 => ckel(i), D2 => ckel(i), R => gnd, S => gnd);
    cke_pad : outpad generic map (tech => virtex4, level => sstl2_i) 
	port map (ddr_cke(i), ddr_ckenr(i));
  end generate;

  rasgen : ODDR  generic map (DDR_CLK_EDGE => "SAME_EDGE")
	port map ( Q => ddr_rasnr, C => clk0r, CE => vcc,
		D1 => rasn, D2 => rasn, R => gnd, S => gnd);
  rasn_pad : outpad generic map (tech => virtex4, level => sstl2_i) 
	port map (ddr_rasb, ddr_rasnr);

  casgen : ODDR  generic map (DDR_CLK_EDGE => "SAME_EDGE")
	port map ( Q => ddr_casnr, C => clk0r, CE => vcc,
		D1 => casn, D2 => casn, R => gnd, S => gnd);
  casn_pad : outpad generic map (tech => virtex4, level => sstl2_i) 
	port map (ddr_casb, ddr_casnr);

  wengen : ODDR  generic map (DDR_CLK_EDGE => "SAME_EDGE")
	port map ( Q => ddr_wenr, C => clk0r, CE => vcc,
		D1 => wen, D2 => wen, R => gnd, S => gnd);
  wen_pad : outpad generic map (tech => virtex4, level => sstl2_i) 
	port map (ddr_web, ddr_wenr);

  dmgen : for i in 0 to dbits/8-1 generate
    da0 : ODDR generic map (DDR_CLK_EDGE => "SAME_EDGE")
               port map ( Q => ddr_dmr(i), C => clk0r, CE => vcc,
		D1 => dm(i+dbits/8), D2 => dm(i), R => gnd, S => gnd);
    ddr_bm_pad  : outpad generic map (tech => virtex4, level => sstl2_i) 
	port map (ddr_dm(i), ddr_dmr(i));
  end generate;

  bagen : for i in 0 to 1 generate
    da0 : ODDR generic map (DDR_CLK_EDGE => "SAME_EDGE")
               port map ( Q => ddr_bar(i), C => clk0r, CE => vcc,
		D1 => ba(i), D2 => ba(i), R => gnd, S => gnd);
    ddr_ba_pad  : outpad generic map (tech => virtex4, level => sstl2_i) 
	port map (ddr_ba(i), ddr_bar(i));
  end generate;

  dagen : for i in 0 to 13 generate
    da0 : ODDR generic map (DDR_CLK_EDGE => "SAME_EDGE")
               port map ( Q => ddr_adr(i), C => clk0r, CE => vcc,
		D1 => addr(i), D2 => addr(i), R => gnd, S => gnd);
    ddr_ad_pad  : outpad generic map (tech => virtex4, level => sstl2_i) 
	port map (ddr_ad(i), ddr_adr(i));

  end generate;

  -- DQS generation
  dsqreg : FD port map ( Q => dqsn, C => clk180r, D => oe);
  dqsgen : for i in 0 to dbits/8-1 generate
    da0 : ODDR generic map (DDR_CLK_EDGE => "SAME_EDGE")
    	port map ( Q => ddr_dqsin(i), C => clk90r, CE => vcc,
		D1 => dqsn, D2 => gnd, R => gnd, S => gnd);
    doen : FD port map ( Q => ddr_dqsoen(i), C => clk0r, D => dqsoen);
    dqs_pad : iopad generic map (tech => virtex4, level => sstl2_ii)
         port map (pad => ddr_dqs(i), i => ddr_dqsin(i), en => ddr_dqsoen(i), 
		o => ddr_dqsoutl(i));
  end generate;

  -- Data bus


    read_rstdel : process (clk_0r, lockl)
    begin
        if lockl = '0' then dll2rst <= (others => '1');
        elsif rising_edge(clk_0r) then
  	  dll2rst <= dll2rst(1 to 3) & '0';
        end if;
    end process;

    bufg7 : BUFG port map (I => rclk0, O => rclk0b);
    bufg8 : BUFG port map (I => rclk90, O => rclk90b);
--    bufg9 : BUFG port map (I => rclk270, O => rclk270b);
    rclk270b <= not rclk90b;

  nops : if rskew = 0 generate
    read_dll : DCM
      generic map (clkin_period => 10.0, DESKEW_ADJUST => "SOURCE_SYNCHRONOUS")
      port map ( CLKIN => ddrclkfbl, CLKFB => rclk0b, DSSEN => gnd, PSCLK => gnd,
      PSEN => gnd, PSINCDEC => gnd, RST => dll2rst(0), CLK0 => rclk0,
      CLK90 => rclk90, CLK270 => rclk270);
  end generate;

  ps : if rskew /= 0 generate
    read_dll : DCM
      generic map (clkin_period => 10.0, DESKEW_ADJUST => "SOURCE_SYNCHRONOUS", 
	CLKOUT_PHASE_SHIFT => "FIXED", PHASE_SHIFT => rskew)
      port map ( CLKIN => ddrclkfbl, CLKFB => rclk0b, DSSEN => gnd, PSCLK => gnd,
      PSEN => gnd, PSINCDEC => gnd, RST => dll2rst(0), CLK0 => rclk0,
      CLK90 => rclk90, CLK270 => rclk270);
  end generate;

    ddgen : for i in 0 to dbits-1 generate
      qi : IDDR generic map (DDR_CLK_EDGE => "OPPOSITE_EDGE")
      port map ( Q1 => dqinl(i), --(i+dbits), -- 1-bit output for positive edge of clock 
               Q2 => dqin(i), -- 1-bit output for negative edge of clock 
	        C => rclk90b, --clk270r, --dqsclk((2*i)/dbits), -- 1-bit clock input 
	       CE => vcc, -- 1-bit clock enable input 
		D => ddr_dqin(i), -- 1-bit DDR data input 
		R => gnd, -- 1-bit reset 
		S => gnd -- 1-bit set 
	      );

      dinq1 : FD port map ( Q => dqin(i+dbits), C => rclk270b, D => dqinl(i));
      dout : ODDR generic map (DDR_CLK_EDGE => "SAME_EDGE")
	port map ( Q => ddr_dqout(i), C => clk0r, CE => vcc,
		D1 => dqout(i+dbits), D2 => dqout(i), R => gnd, S => gnd);
      doen : FD port map ( Q => ddr_dqoen(i), C => clk0r, D => oen);
      dq_pad : iopad generic map (tech => virtex4, level => sstl2_ii)
         port map (pad => ddr_dq(i), i => ddr_dqout(i), en => ddr_dqoen(i), o => ddr_dqin(i));
    end generate;

end;

library ieee;
use ieee.std_logic_1164.all;
-- pragma translate_off
library unisim;
use unisim.BUFG;
use unisim.DCM;
use unisim.FDDRRSE;
use unisim.IFDDRRSE;
use unisim.FD;
-- pragma translate_on

library grlib;
use grlib.stdlib.all;
library techmap;
use techmap.gencomp.all;
use techmap.oddrv2;

------------------------------------------------------------------
-- Virtex2 DDR PHY -----------------------------------------------
------------------------------------------------------------------

entity virtex2_ddr_phy is
  generic (MHz : integer := 100; rstdelay : integer := 200;
	dbits : integer := 16; clk_mul : integer := 2 ;
	clk_div : integer := 2; rskew : integer := 0);

  port (
    rst       : in  std_ulogic;
    clk       : in  std_logic;          	-- input clock
    clkout    : out std_ulogic;			-- system clock
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
    cke       	: in  std_logic_vector(1 downto 0)
  );

end;

architecture rtl of virtex2_ddr_phy is

  component DCM
    generic (
      CLKDV_DIVIDE : real := 2.0;
      CLKFX_DIVIDE : integer := 1;
      CLKFX_MULTIPLY : integer := 4;
      CLKIN_DIVIDE_BY_2 : boolean := false;
      CLKIN_PERIOD : real := 10.0;
      CLKOUT_PHASE_SHIFT : string := "NONE";
      CLK_FEEDBACK : string := "1X";
      DESKEW_ADJUST : string := "SYSTEM_SYNCHRONOUS";
      DFS_FREQUENCY_MODE : string := "LOW";
      DLL_FREQUENCY_MODE : string := "LOW";
      DSS_MODE : string := "NONE";
      DUTY_CYCLE_CORRECTION : boolean := true;
      FACTORY_JF : bit_vector := X"C080";
      PHASE_SHIFT : integer := 0;
      STARTUP_WAIT : boolean := false 
    );
    port (
      CLKFB    : in  std_logic;
      CLKIN    : in  std_logic;
      DSSEN    : in  std_logic;
      PSCLK    : in  std_logic;
      PSEN     : in  std_logic;
      PSINCDEC : in  std_logic;
      RST      : in  std_logic;
      CLK0     : out std_logic;
      CLK90    : out std_logic;
      CLK180   : out std_logic;
      CLK270   : out std_logic;
      CLK2X    : out std_logic;
      CLK2X180 : out std_logic;
      CLKDV    : out std_logic;
      CLKFX    : out std_logic;
      CLKFX180 : out std_logic;
      LOCKED   : out std_logic;
      PSDONE   : out std_logic;
      STATUS   : out std_logic_vector (7 downto 0));
  end component;
  component BUFG port (O : out std_logic; I : in std_logic); end component;
  component FDDRRSE
--    generic ( INIT : bit := '0');
    port
      (
        Q : out std_ulogic;
        C0 : in std_ulogic;
        C1 : in std_ulogic;
        CE : in std_ulogic;
        D0 : in std_ulogic;
        D1 : in std_ulogic;
        R : in std_ulogic;
        S : in std_ulogic
      );
  end component;

  component IFDDRRSE
	port (
		Q0 : out std_ulogic;
		Q1 : out std_ulogic;
		C0 : in std_ulogic;
		C1 : in std_ulogic;
		CE : in std_ulogic;
		D : in std_ulogic;
		R : in std_ulogic;
		S : in std_ulogic);
  end component;
  component FD
	generic ( INIT : bit := '0');
	port (  Q : out std_ulogic;
		C : in std_ulogic;
		D : in std_ulogic);
  end component;

  component oddrv2
  generic ( tech : integer := virtex4); 
  port
    ( Q : out std_ulogic;
      C1 : in std_ulogic;
      C2 : in std_ulogic;
      CE : in std_ulogic;
      D1 : in std_ulogic;
      D2 : in std_ulogic;
      R : in std_ulogic;
      S : in std_ulogic);
  end component;

signal vcc, gnd, dqsn, oe, lockl : std_ulogic;
signal ddr_clk_fb_outr : std_ulogic;
signal ddr_clk_fbl, fbclk : std_ulogic;
signal ddr_rasnr, ddr_casnr, ddr_wenr : std_ulogic;
signal ddr_clkl, ddr_clkbl : std_logic_vector(2 downto 0);
signal ddr_csnr, ddr_ckenr, ckel : std_logic_vector(1 downto 0);
signal clk_0ro, clk_90ro, clk_180ro, clk_270ro : std_ulogic;
signal clk_0r, clk_90r, clk_180r, clk_270r : std_ulogic;
signal clk0r, clk90r, clk180r, clk270r : std_ulogic;
signal locked, vlockl, ddrclkfbl : std_ulogic;
signal ddr_dqin  	: std_logic_vector (dbits-1 downto 0); -- ddr data
signal ddr_dqout  	: std_logic_vector (dbits-1 downto 0); -- ddr data
signal ddr_dqoen  	: std_logic_vector (dbits-1 downto 0); -- ddr data
signal ddr_adr      	: std_logic_vector (13 downto 0);   -- ddr address
signal ddr_bar      	: std_logic_vector (1 downto 0);   -- ddr address
signal ddr_dmr      	: std_logic_vector (dbits/8-1 downto 0);   -- ddr address
signal ddr_dqsin  	: std_logic_vector (dbits/8-1 downto 0);    -- ddr dqs
signal ddr_dqsoen 	: std_logic_vector (dbits/8-1 downto 0);    -- ddr dqs
signal ddr_dqsoutl 	: std_logic_vector (dbits/8-1 downto 0);    -- ddr dqs
signal dqsdel, dqsclk 	: std_logic_vector (dbits/8-1 downto 0);    -- ddr dqs
signal da     		: std_logic_vector (dbits-1 downto 0); -- ddr data
signal dqinl		: std_logic_vector (dbits-1 downto 0); -- ddr data
signal dllrst		: std_logic_vector(0 to 3);
signal dll0rst, dll2rst	: std_logic_vector(0 to 3);
signal mlock, mclkfb, mclk, mclkfx, mclk0 : std_ulogic;
signal rclk270b, rclk90b, rclk0b : std_ulogic;
signal rclk270, rclk90, rclk0 : std_ulogic;

constant DDR_FREQ : integer := (MHz * clk_mul) / clk_div;

begin

  oe <= not oen;
  vcc <= '1'; gnd <= '0';

  -- Optional DDR clock multiplication

  noclkscale : if clk_mul = clk_div generate
    mclk <= clk; mlock <= rst;
  end generate;

  clkscale : if clk_mul /= clk_div generate

    rstdel : process (clk, rst)
    begin
        if rst = '0' then dll0rst <= (others => '1');
        elsif rising_edge(clk) then
  	  dll0rst <= dll0rst(1 to 3) & '0';
        end if;
    end process;

    bufg0 : BUFG port map (I => mclkfx, O => mclk);
    bufg1 : BUFG port map (I => mclk0, O => mclkfb);

    dllm : DCM
      generic map (CLKFX_MULTIPLY => clk_mul, CLKFX_DIVIDE => clk_div,
	CLKIN_PERIOD => 10.0)
      port map ( CLKIN => clk, CLKFB => mclkfb, DSSEN => gnd, PSCLK => gnd,
      PSEN => gnd, PSINCDEC => gnd, RST => dll0rst(0), CLK0 => mclk0,
      LOCKED => mlock, CLKFX => mclkfx );
  end generate;

  -- DDR output clock generation
  
  bufg1 : BUFG port map (I => clk_0ro, O => clk_0r);
--  bufg2 : BUFG port map (I => clk_90ro, O => clk_90r);
  clk_90r <= not clk_270r;
--  bufg3 : BUFG port map (I => clk_180ro, O => clk_180r);
  clk_180r <= not clk_0r;
  bufg4 : BUFG port map (I => clk_270ro, O => clk_270r);

  clkout <= clk_270r; clk0r <= clk_270r; clk90r <= clk_0r;
  clk180r <= clk_90r; clk270r <= clk_180r;

  
  dll : DCM generic map (CLKFX_MULTIPLY => 2, CLKFX_DIVIDE => 2)
      port map ( CLKIN => mclk, CLKFB => clk_0r, DSSEN => gnd, PSCLK => gnd,
      PSEN => gnd, PSINCDEC => gnd, RST => dllrst(0), CLK0 => clk_0ro, 
      CLK90 => clk_90ro, CLK180 => clk_180ro, CLK270 => clk_270ro, 
      LOCKED => lockl);

  rstdel : process (mclk, mlock)
  begin
      if mlock = '0' then dllrst <= (others => '1');
      elsif rising_edge(mclk) then
	dllrst <= dllrst(1 to 3) & '0';
      end if;
  end process;

  rdel : if rstdelay /= 0 generate
    rcnt : process (clk_0r)
    variable cnt : std_logic_vector(15 downto 0);
    variable vlock, co : std_ulogic;
    begin
      if rising_edge(clk_0r) then
        co := cnt(15);
        vlockl <= vlock;
        if lockl = '0' then
	  cnt := conv_std_logic_vector(rstdelay*DDR_FREQ, 16); vlock := '0';
        else
	  if vlock = '0' then
	    cnt := cnt -1;  vlock := cnt(15) and not co;
	  end if;
        end if;
      end if;
      if lockl = '0' then
	vlock := '0';
      end if;
    end process;
  end generate;

  locked <= lockl when rstdelay = 0 else vlockl;
  lock <= locked;

  -- Generate external DDR clock

  fbdclk0r : FDDRRSE port map ( Q => ddr_clk_fb_outr, C0 => clk90r, C1 => clk270r,
	CE => vcc, D0 => vcc, D1 => gnd, R => gnd, S => gnd);
  fbclk_pad : outpad generic map (tech => virtex4, level => sstl2_i) 
	port map (ddr_clk_fb_out, ddr_clk_fb_outr);

  ddrclocks : for i in 0 to 2 generate
    dclk0r : FDDRRSE port map ( Q => ddr_clkl(i), C0 => clk90r, C1 => clk270r, 
	CE => vcc, D0 => vcc, D1 => gnd, R => gnd, S => gnd);
    ddrclk_pad : outpad generic map (tech => virtex4, level => sstl2_i) 
	port map (ddr_clk(i), ddr_clkl(i));

    dclk0rb : FDDRRSE port map ( Q => ddr_clkbl(i), C0 => clk90r, C1 => clk270r,
	CE => vcc, D0 => gnd, D1 => vcc, R => gnd, S => gnd);
    ddrclkb_pad : outpad generic map (tech => virtex4, level => sstl2_i) 
	port map (ddr_clkb(i), ddr_clkbl(i));
  end generate;

  ddrbanks : for i in 0 to 1 generate
    csn0gen : FD port map ( Q => ddr_csnr(i), C => clk0r, D => csn(i));
    csn0_pad : outpad generic map (tech => virtex4, level => sstl2_i) 
	port map (ddr_csb(i), ddr_csnr(i));

    ckel(i) <= cke(i) and locked;
    ckegen : FD port map ( Q => ddr_ckenr(i), C => clk0r, D => ckel(i));
    cke_pad : outpad generic map (tech => virtex4, level => sstl2_i) 
	port map (ddr_cke(i), ddr_ckenr(i));

  end generate;

  --  DDR single-edge control signals
  rasgen : FD port map ( Q => ddr_rasnr, C => clk0r, D => rasn);
  rasn_pad : outpad generic map (tech => virtex4, level => sstl2_i) 
	port map (ddr_rasb, ddr_rasnr);

  casgen : FD port map ( Q => ddr_casnr, C => clk0r, D => casn);
  casn_pad : outpad generic map (tech => virtex4, level => sstl2_i) 
	port map (ddr_casb, ddr_casnr);

  wengen : FD port map ( Q => ddr_wenr, C => clk0r, D => wen);
  wen_pad : outpad generic map (tech => virtex4, level => sstl2_i) 
	port map (ddr_web, ddr_wenr);

  dmgen : for i in 0 to dbits/8-1 generate
    da0 : oddrv2 port map ( Q => ddr_dmr(i), C1 => clk0r, C2 => clk180r,
	CE => vcc, D1 => dm(i+dbits/8), D2 => dm(i), R => gnd, S => gnd);
    ddr_bm_pad  : outpad generic map (tech => virtex4, level => sstl2_i) 
	port map (ddr_dm(i), ddr_dmr(i));
  end generate;

  bagen : for i in 0 to 1 generate
    da0 : FD port map ( Q => ddr_bar(i), C => clk0r, D => ba(i));
    ddr_ba_pad  : outpad generic map (tech => virtex4, level => sstl2_i) 
	port map (ddr_ba(i), ddr_bar(i));
  end generate;

  dagen : for i in 0 to 13 generate
    da0 : FD port map ( Q => ddr_adr(i), C => clk0r, D => addr(i));
    ddr_ad_pad  : outpad generic map (tech => virtex4, level => sstl2_i) 
	port map (ddr_ad(i), ddr_adr(i));

  end generate;

  -- DQS generation
  dsqreg : FD port map ( Q => dqsn, C => clk180r, D => oe);
  dqsgen : for i in 0 to dbits/8-1 generate
    da0 : oddrv2
    	port map ( Q => ddr_dqsin(i), C1 => clk90r,  C2 => clk270r, 
	CE => vcc, D1 => dqsn, D2 => gnd, R => gnd, S => gnd);
    doen : FD port map ( Q => ddr_dqsoen(i), C => clk0r, D => dqsoen);
    dqs_pad : iopad generic map (tech => virtex4, level => sstl2_ii)
         port map (pad => ddr_dqs(i), i => ddr_dqsin(i), en => ddr_dqsoen(i), 
		o => ddr_dqsoutl(i));
  end generate;

  -- Data bus

  ddrref_pad : clkpad generic map (tech => virtex2) 
	port map (ddr_clk_fb, ddrclkfbl); 

    read_rstdel : process (clk_0r, lockl)
    begin
        if lockl = '0' then dll2rst <= (others => '1');
        elsif rising_edge(clk_0r) then
  	  dll2rst <= dll2rst(1 to 3) & '0';
        end if;
    end process;

    bufg7 : BUFG port map (I => rclk0, O => rclk0b);
    bufg8 : BUFG port map (I => rclk90, O => rclk90b);
--    bufg9 : BUFG port map (I => rclk270, O => rclk270b);
    rclk270b <= not rclk90b;

  nops : if rskew = 0 generate
    read_dll : DCM
      generic map (clkin_period => 10.0, DESKEW_ADJUST => "SOURCE_SYNCHRONOUS")
      port map ( CLKIN => ddrclkfbl, CLKFB => rclk0b, DSSEN => gnd, PSCLK => gnd,
      PSEN => gnd, PSINCDEC => gnd, RST => dll2rst(0), CLK0 => rclk0,
      CLK90 => rclk90, CLK270 => rclk270);
  end generate;

  ps : if rskew /= 0 generate
    read_dll : DCM
      generic map (clkin_period => 10.0, DESKEW_ADJUST => "SOURCE_SYNCHRONOUS", 
	CLKOUT_PHASE_SHIFT => "FIXED", PHASE_SHIFT => rskew)
      port map ( CLKIN => ddrclkfbl, CLKFB => rclk0b, DSSEN => gnd, PSCLK => gnd,
      PSEN => gnd, PSINCDEC => gnd, RST => dll2rst(0), CLK0 => rclk0,
      CLK90 => rclk90, CLK270 => rclk270);
  end generate;

  ddgen : for i in 0 to dbits-1 generate
    qi : IFDDRRSE
    port map ( Q0 => dqinl(i), --(i+dbits), -- 1-bit output for positive edge of clock 
               Q1 => dqin(i), -- 1-bit output for negative edge of clock 
	       C0 => rclk90b, -- clk270r, --dqsclk((2*i)/dbits), -- 1-bit clock input 
	       C1 => rclk270b, -- clk90r, --dqsclk((2*i)/dbits), -- 1-bit clock input 
	       CE => vcc, -- 1-bit clock enable input 
		D => ddr_dq(i), -- 1-bit DDR data input 
		R => gnd, -- 1-bit reset 
		S => gnd -- 1-bit set 
	      );

--    dinq1 : FD port map ( Q => dqin(i+dbits), C => clk90r, D => dqinl(i));
    dinq1 : FD port map ( Q => dqin(i+dbits), C => rclk270b, D => dqinl(i));
    dout : oddrv2
	port map ( Q => ddr_dqout(i), C1 => clk0r, C2 => clk180r, CE => vcc,
		D1 => dqout(i+dbits), D2 => dqout(i), R => gnd, S => gnd);
    doen : FD port map ( Q => ddr_dqoen(i), C => clk0r, D => oen);
    dq_pad : iopad generic map (tech => virtex4, level => sstl2_ii)
         port map (pad => ddr_dq(i), i => ddr_dqout(i), en => ddr_dqoen(i), o => open); -- o => ddr_dqin(i));
  end generate;


  
end;

library ieee;
use ieee.std_logic_1164.all;
-- pragma translate_off
library unisim;
use unisim.BUFG;
use unisim.DCM;
use unisim.ODDR2;
use unisim.IDDR2;
use unisim.FD;
-- pragma translate_on

library grlib;
use grlib.stdlib.all;
library techmap;
use techmap.gencomp.all;
use techmap.oddrc3e;

------------------------------------------------------------------
-- Spartan3E DDR PHY -----------------------------------------------
------------------------------------------------------------------

entity spartan3e_ddr_phy is
  generic (MHz : integer := 100; rstdelay : integer := 200;
	dbits : integer := 16; clk_mul : integer := 2 ;
	clk_div : integer := 2; rskew : integer := 0);

  port (
    rst       : in  std_ulogic;
    clk       : in  std_logic;          	-- input clock
    clkout    : out std_ulogic;			-- DDR state clock
    clkread   : out std_ulogic;			-- DDR read clock
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
    cke       	: in  std_logic_vector(1 downto 0)
  );

end;

architecture rtl of spartan3e_ddr_phy is

component oddrc3e
  generic ( tech : integer := virtex4); 
  port
    ( Q : out std_ulogic;
      C1 : in std_ulogic;
      C2 : in std_ulogic;
      CE : in std_ulogic;
      D1 : in std_ulogic;
      D2 : in std_ulogic;
      R : in std_ulogic;
      S : in std_ulogic);
end component;

  component DCM
    generic (
      CLKDV_DIVIDE : real := 2.0;
      CLKFX_DIVIDE : integer := 1;
      CLKFX_MULTIPLY : integer := 4;
      CLKIN_DIVIDE_BY_2 : boolean := false;
      CLKIN_PERIOD : real := 10.0;
      CLKOUT_PHASE_SHIFT : string := "NONE";
      CLK_FEEDBACK : string := "1X";
      DESKEW_ADJUST : string := "SYSTEM_SYNCHRONOUS";
      DFS_FREQUENCY_MODE : string := "LOW";
      DLL_FREQUENCY_MODE : string := "LOW";
      DSS_MODE : string := "NONE";
      DUTY_CYCLE_CORRECTION : boolean := true;
      FACTORY_JF : bit_vector := X"C080";
      PHASE_SHIFT : integer := 0;
      STARTUP_WAIT : boolean := false 
    );
    port (
      CLKFB    : in  std_logic;
      CLKIN    : in  std_logic;
      DSSEN    : in  std_logic;
      PSCLK    : in  std_logic;
      PSEN     : in  std_logic;
      PSINCDEC : in  std_logic;
      RST      : in  std_logic;
      CLK0     : out std_logic;
      CLK90    : out std_logic;
      CLK180   : out std_logic;
      CLK270   : out std_logic;
      CLK2X    : out std_logic;
      CLK2X180 : out std_logic;
      CLKDV    : out std_logic;
      CLKFX    : out std_logic;
      CLKFX180 : out std_logic;
      LOCKED   : out std_logic;
      PSDONE   : out std_logic;
      STATUS   : out std_logic_vector (7 downto 0));
  end component;
  component BUFG port (O : out std_logic; I : in std_logic); end component;
  component FD
	generic ( INIT : bit := '0');
	port (  Q : out std_ulogic;
		C : in std_ulogic;
		D : in std_ulogic);
  end component;

  component ODDR2
	generic
	(
		DDR_ALIGNMENT : string := "NONE";
		INIT : bit := '0';
		SRTYPE : string := "SYNC"
	);
	port
	(
		Q : out std_ulogic;
		C0 : in std_ulogic;
		C1 : in std_ulogic;
		CE : in std_ulogic;
		D0 : in std_ulogic;
		D1 : in std_ulogic;
		R : in std_ulogic;
		S : in std_ulogic
	);
  end component;
  component IDDR2
	generic
	(
		DDR_ALIGNMENT : string := "NONE";
		INIT_Q0 : bit := '0';
		INIT_Q1 : bit := '0';
		SRTYPE : string := "SYNC"
	);
	port
	(
		Q0 : out std_ulogic;
		Q1 : out std_ulogic;
		C0 : in std_ulogic;
		C1 : in std_ulogic;
		CE : in std_ulogic;
		D : in std_ulogic;
		R : in std_ulogic;
		S : in std_ulogic
	);
  end component;

signal vcc, gnd, dqsn, oe, lockl : std_ulogic;
signal ddr_clk_fb_outr : std_ulogic;
signal ddr_clk_fbl, fbclk : std_ulogic;
signal ddr_rasnr, ddr_casnr, ddr_wenr : std_ulogic;
signal ddr_clkl, ddr_clkbl : std_logic_vector(2 downto 0);
signal ddr_csnr, ddr_ckenr, ckel : std_logic_vector(1 downto 0);
signal clk_0ro, clk_90ro, clk_180ro, clk_270ro : std_ulogic;
signal clk_0r, clk_90r, clk_180r, clk_270r : std_ulogic;
signal clk0r, clk90r, clk180r, clk270r : std_ulogic;
signal locked, vlockl, ddrclkfbl, dllfb : std_ulogic;
signal ddr_dqin  	: std_logic_vector (dbits-1 downto 0); -- ddr data
signal ddr_dqout  	: std_logic_vector (dbits-1 downto 0); -- ddr data
signal ddr_dqoen  	: std_logic_vector (dbits-1 downto 0); -- ddr data
signal ddr_adr      	: std_logic_vector (13 downto 0);   -- ddr address
signal ddr_bar      	: std_logic_vector (1 downto 0);   -- ddr address
signal ddr_dmr      	: std_logic_vector (dbits/8-1 downto 0);   -- ddr address
signal ddr_dqsin  	: std_logic_vector (dbits/8-1 downto 0);    -- ddr dqs
signal ddr_dqsoen 	: std_logic_vector (dbits/8-1 downto 0);    -- ddr dqs
signal ddr_dqsoutl 	: std_logic_vector (dbits/8-1 downto 0);    -- ddr dqs
signal dqsdel, dqsclk 	: std_logic_vector (dbits/8-1 downto 0);    -- ddr dqs
signal da     		: std_logic_vector (dbits-1 downto 0); -- ddr data
signal dqinl		: std_logic_vector (dbits-1 downto 0); -- ddr data
signal dllrst		: std_logic_vector(0 to 3);
signal dll0rst, dll2rst	: std_logic_vector(0 to 3);
signal mlock, mclkfb, mclk, mclkfx, mclk0 : std_ulogic;
signal rclk270b, rclk90b, rclk0b : std_ulogic;
signal rclk270, rclk90, rclk0 : std_ulogic;

constant DDR_FREQ : integer := (MHz * clk_mul) / clk_div;

begin

  oe <= not oen;
  vcc <= '1'; gnd <= '0';

  -- Optional DDR clock multiplication

  noclkscale : if clk_mul = clk_div generate
    mclk <= clk; mlock <= rst;
  end generate;

  clkscale : if clk_mul /= clk_div generate

    rstdel : process (clk, rst)
    begin
        if rst = '0' then dll0rst <= (others => '1');
        elsif rising_edge(clk) then
  	  dll0rst <= dll0rst(1 to 3) & '0';
        end if;
    end process;

    bufg0 : BUFG port map (I => mclkfx, O => mclk);
    bufg1 : BUFG port map (I => mclk0, O => mclkfb);

    dllm : DCM
      generic map (CLKFX_MULTIPLY => clk_mul, CLKFX_DIVIDE => clk_div,
	CLKIN_PERIOD => 10.0)
      port map ( CLKIN => clk, CLKFB => mclkfb, DSSEN => gnd, PSCLK => gnd,
      PSEN => gnd, PSINCDEC => gnd, RST => dll0rst(0), CLK0 => mclk0,
      LOCKED => mlock, CLKFX => mclkfx );
  end generate;

  -- DDR output clock generation
  
  bufg1 : BUFG port map (I => clk_0ro, O => clk_0r);
--  bufg2 : BUFG port map (I => clk_90ro, O => clk_90r);
  clk_90r <= not clk_270r;
--  bufg3 : BUFG port map (I => clk_180ro, O => clk_180r);
  clk_180r <= not clk_0r;
  bufg4 : BUFG port map (I => clk_270ro, O => clk_270r);

  clkout <= clk_270r; 
--  clkout <= clk_90r when DDR_FREQ > 120 else clk_0r; 

  clk0r <= clk_270r; clk90r <= clk_0r;
  clk180r <= clk_90r; clk270r <= clk_180r;

  dll : DCM generic map (CLKFX_MULTIPLY => 2, CLKFX_DIVIDE => 2)
      port map ( CLKIN => mclk, CLKFB => clk_0r, DSSEN => gnd, PSCLK => gnd,
      PSEN => gnd, PSINCDEC => gnd, RST => dllrst(0), CLK0 => clk_0ro, 
      CLK90 => clk_90ro, CLK180 => clk_180ro, CLK270 => clk_270ro, 
      LOCKED => lockl);

  rstdel : process (mclk, mlock)
  begin
      if mlock = '0' then dllrst <= (others => '1');
      elsif rising_edge(mclk) then
	dllrst <= dllrst(1 to 3) & '0';
      end if;
  end process;

  rdel : if rstdelay /= 0 generate
    rcnt : process (clk_0r)
    variable cnt : std_logic_vector(15 downto 0);
    variable vlock, co : std_ulogic;
    begin
      if rising_edge(clk_0r) then
        co := cnt(15);
        vlockl <= vlock;
        if lockl = '0' then
	  cnt := conv_std_logic_vector(rstdelay*DDR_FREQ, 16); vlock := '0';
        else
	  if vlock = '0' then
	    cnt := cnt -1;  vlock := cnt(15) and not co;
	  end if;
        end if;
      end if;
      if lockl = '0' then
	vlock := '0';
      end if;
    end process;
  end generate;

  locked <= lockl when rstdelay = 0 else vlockl;
  lock <= locked;

  -- Generate external DDR clock

  fbdclk0r : ODDR2 port map ( Q => ddr_clk_fb_outr, C0 => clk90r, C1 => clk270r,
	CE => vcc, D0 => vcc, D1 => gnd, R => gnd, S => gnd);
  fbclk_pad : outpad generic map (tech => virtex4, level => sstl2_i) 
	port map (ddr_clk_fb_out, ddr_clk_fb_outr);

  ddrclocks : for i in 0 to 2 generate
    dclk0r : ODDR2 port map ( Q => ddr_clkl(i), C0 => clk90r, C1 => clk270r, 
	CE => vcc, D0 => vcc, D1 => gnd, R => gnd, S => gnd);
    ddrclk_pad : outpad generic map (tech => virtex4, level => sstl2_i) 
	port map (ddr_clk(i), ddr_clkl(i));

    dclk0rb : ODDR2 port map ( Q => ddr_clkbl(i), C0 => clk90r, C1 => clk270r,
	CE => vcc, D0 => gnd, D1 => vcc, R => gnd, S => gnd);
    ddrclkb_pad : outpad generic map (tech => virtex4, level => sstl2_i) 
	port map (ddr_clkb(i), ddr_clkbl(i));
  end generate;

  ddrbanks : for i in 0 to 1 generate
    csn0gen : FD port map ( Q => ddr_csnr(i), C => clk0r, D => csn(i));
    csn0_pad : outpad generic map (tech => virtex4, level => sstl2_i) 
	port map (ddr_csb(i), ddr_csnr(i));

    ckel(i) <= cke(i) and locked;
    ckegen : FD port map ( Q => ddr_ckenr(i), C => clk0r, D => ckel(i));
    cke_pad : outpad generic map (tech => virtex4, level => sstl2_i) 
	port map (ddr_cke(i), ddr_ckenr(i));

  end generate;

  --  DDR single-edge control signals
  rasgen : FD port map ( Q => ddr_rasnr, C => clk0r, D => rasn);
  rasn_pad : outpad generic map (tech => virtex4, level => sstl2_i) 
	port map (ddr_rasb, ddr_rasnr);

  casgen : FD port map ( Q => ddr_casnr, C => clk0r, D => casn);
  casn_pad : outpad generic map (tech => virtex4, level => sstl2_i) 
	port map (ddr_casb, ddr_casnr);

  wengen : FD port map ( Q => ddr_wenr, C => clk0r, D => wen);
  wen_pad : outpad generic map (tech => virtex4, level => sstl2_i) 
	port map (ddr_web, ddr_wenr);

  dmgen : for i in 0 to dbits/8-1 generate
    da0 : oddrc3e 
        port map ( Q => ddr_dmr(i), C1 => clk0r, C2 => clk180r,
	CE => vcc, D1 => dm(i+dbits/8), D2 => dm(i), R => gnd, S => gnd);
    ddr_bm_pad  : outpad generic map (tech => virtex4, level => sstl2_i) 
	port map (ddr_dm(i), ddr_dmr(i));
  end generate;

  bagen : for i in 0 to 1 generate
    da0 : FD port map ( Q => ddr_bar(i), C => clk0r, D => ba(i));
    ddr_ba_pad  : outpad generic map (tech => virtex4, level => sstl2_i) 
	port map (ddr_ba(i), ddr_bar(i));
  end generate;

  dagen : for i in 0 to 13 generate
    da0 : FD port map ( Q => ddr_adr(i), C => clk0r, D => addr(i));
    ddr_ad_pad  : outpad generic map (tech => virtex4, level => sstl2_i) 
	port map (ddr_ad(i), ddr_adr(i));

  end generate;

  -- DQS generation
  dsqreg : FD port map ( Q => dqsn, C => clk180r, D => oe);
  dqsgen : for i in 0 to dbits/8-1 generate
    da0 : oddrc3e
    	port map ( Q => ddr_dqsin(i), C1 => clk90r,  C2 => clk270r, 
	CE => vcc, D1 => dqsn, D2 => gnd, R => gnd, S => gnd);
    doen : FD port map ( Q => ddr_dqsoen(i), C => clk0r, D => dqsoen);
    dqs_pad : iopad generic map (tech => virtex4, level => sstl2_i)
         port map (pad => ddr_dqs(i), i => ddr_dqsin(i), en => ddr_dqsoen(i), 
		o => ddr_dqsoutl(i));
  end generate;

  -- Data bus

  ddrref_pad : clkpad generic map (tech => virtex2) 
	port map (ddr_clk_fb, ddrclkfbl); 


    read_rstdel : process (clk_0r, lockl)
    begin
        if lockl = '0' then dll2rst <= (others => '1');
        elsif rising_edge(clk_0r) then
  	  dll2rst <= dll2rst(1 to 3) & '0';
        end if;
    end process;

    bufg7 : BUFG port map (I => rclk0, O => rclk0b);
    bufg8 : BUFG port map (I => rclk90, O => rclk90b);
--    bufg9 : BUFG port map (I => rclk270, O => rclk270b);
    rclk270b <= not rclk90b;
    clkread <= not rclk90b;

  nops : if rskew = 0 generate
    read_dll : DCM
      generic map (clkin_period => 10.0, DESKEW_ADJUST => "SOURCE_SYNCHRONOUS")
      port map ( CLKIN => ddrclkfbl, CLKFB => rclk0b, DSSEN => gnd, PSCLK => gnd,
      PSEN => gnd, PSINCDEC => gnd, RST => dll2rst(0), CLK0 => rclk0,
      CLK90 => rclk90, CLK270 => rclk270);
  end generate;
  ps : if rskew /= 0 generate
    read_dll : DCM
      generic map (clkin_period => 10.0, DESKEW_ADJUST => "SOURCE_SYNCHRONOUS", 
	CLKOUT_PHASE_SHIFT => "FIXED", PHASE_SHIFT => rskew)
      port map ( CLKIN => ddrclkfbl, CLKFB => rclk0b, DSSEN => gnd, PSCLK => gnd,
      PSEN => gnd, PSINCDEC => gnd, RST => dll2rst(0), CLK0 => rclk0,
      CLK90 => rclk90, CLK270 => rclk270);
  end generate;

  ddgen : for i in 0 to dbits-1 generate
    qi : IDDR2
    port map ( Q0 => dqinl(i), Q1 => dqin(i), C0 => rclk90b, C1 => rclk270b, 
	       CE => vcc, D => ddr_dqin(i), R => gnd, S => gnd );
    dinq1 : FD port map ( Q => dqin(i+dbits), C => rclk270b, D => dqinl(i));
    dout : oddrc3e
	port map ( Q => ddr_dqout(i), C1 => clk0r, C2 => clk180r, CE => vcc,
		D1 => dqout(i+dbits), D2 => dqout(i), R => gnd, S => gnd);
    doen : FD port map ( Q => ddr_dqoen(i), C => clk0r, D => oen);
    dq_pad : iopad generic map (tech => virtex4, level => sstl2_i)
       port map (pad => ddr_dq(i), i => ddr_dqout(i), en => ddr_dqoen(i), o => ddr_dqin(i));
  end generate;

end;



