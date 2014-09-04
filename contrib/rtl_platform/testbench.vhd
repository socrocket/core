-----------------------------------------------------------------------------
--  LEON3 Demonstration design test bench
--  Copyright (C) 2004 Jiri Gaisler, Gaisler Research
------------------------------------------------------------------------------
--  This file is a part of the GRLIB VHDL IP LIBRARY
--  Copyright (C) 2003 - 2008, Gaisler Research
--  Copyright (C) 2008, 2009, Aeroflex Gaisler
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
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library gaisler;
use gaisler.libdcom.all;
use gaisler.sim.all;
library techmap;
use techmap.gencomp.all;
library micron;
use micron.components.all;

use work.config.all;	-- configuration

entity testbench is
  generic (
    fabtech   : integer := CFG_FABTECH;
    memtech   : integer := CFG_MEMTECH;
    padtech   : integer := CFG_PADTECH;
    clktech   : integer := CFG_CLKTECH;
    disas     : integer := CFG_DISAS;	-- Enable disassembly to console
    dbguart   : integer := CFG_DUART;	-- Print UART on console
    pclow     : integer := CFG_PCLOW;

    clkperiod : integer := 20;		-- system clock period
    romwidth  : integer := 32;		-- rom data width (8/32)
    romdepth  : integer := 16;		-- rom address depth
    sramwidth  : integer := 32;		-- ram data width (8/16/32)
    sramdepth  : integer := 18;		-- ram address depth
    srambanks  : integer := 2		-- number of ram banks
  );
end; 

architecture behav of testbench is

constant promfile  : string := "prom.srec";  -- rom contents
constant sramfile  : string := "sram.srec";  -- ram contents
constant sdramfile : string := "sdram.srec"; -- sdram contents

component leon3mp
  generic (
    fabtech   : integer := CFG_FABTECH;
    memtech   : integer := CFG_MEMTECH;
    padtech   : integer := CFG_PADTECH;
    clktech   : integer := CFG_CLKTECH;
    disas     : integer := CFG_DISAS;	-- Enable disassembly to console
    pclow     : integer := CFG_PCLOW
  );
  port (
    resetn	: in  std_ulogic;
    clk		: in  std_ulogic;
    pllref 	: in  std_ulogic; 
    errorn	: out std_ulogic;
    wdogn 	: out std_ulogic;

    address 	: out std_logic_vector(27 downto 0);
    data	: inout std_logic_vector(31 downto 0);
    ramsn  	: out std_logic_vector (4 downto 0);
    ramoen 	: out std_logic_vector (4 downto 0);
    rwen   	: out std_logic_vector (3 downto 0);
    oen    	: out std_ulogic;
    writen 	: out std_ulogic;
    read   	: out std_ulogic;
    iosn   	: out std_ulogic;
    romsn  	: out std_logic_vector (1 downto 0);
    sdclk  	: out std_ulogic;
    sdcsn  	: out std_logic_vector (1 downto 0);    -- sdram chip select
    sdwen  	: out std_ulogic;                       -- sdram write enable
    sdrasn  	: out std_ulogic;                     -- sdram ras
    sdcasn  	: out std_ulogic;                     -- sdram cas
    sddqm   	: out std_logic_vector (3 downto 0)   -- sdram dqm
  );

end component;

signal clk : std_logic := '0';
signal rst : std_logic := '1';
constant ct : integer := clkperiod/2;

signal address  : std_logic_vector(27 downto 0);
signal data     : std_logic_vector(31 downto 0);
signal romsn  	: std_logic_vector(1 downto 0);
signal ramsn  	: std_logic_vector(4 downto 0);
signal ramoen 	: std_logic_vector(4 downto 0);
signal rwen 	: std_logic_vector(3 downto 0);
signal oen      : std_ulogic;
signal writen   : std_ulogic;
signal read   	: std_ulogic;
signal iosn   	: std_ulogic;
signal wdogn    : std_logic;
    
signal sdcsn    : std_logic_vector ( 1 downto 0); 
signal sdwen    : std_ulogic;                       -- write en
signal sdrasn   : std_ulogic;                       -- row addr stb
signal sdcasn   : std_ulogic;                       -- col addr stb
signal sddqm    : std_logic_vector ( 3 downto 0);   -- data i/o mask
signal sdclk    : std_ulogic;       
signal pllref   : std_ulogic;       
signal errorn   : std_logic;       

constant lresp : boolean := false;

begin

-- clock and reset

  clk  <= not clk after ct * 1 ns;
  rst <= wdogn; 
  pllref <= sdclk;
  wdogn <= 'H';
  
  cpu : leon3mp
      generic map (fabtech, memtech, padtech, clktech, disas, pclow)
      port map (rst, clk, pllref, errorn, wdogn, address(27 downto 0), data, ramsn, ramoen, rwen, oen, writen, read, iosn, romsn, sdclk, sdcsn, sdwen, sdrasn, sdcasn, sddqm);

  u0: mt48lc16m16a2
      generic map (
          index => 0, 
          fname => sdramfile
      )
	    port map (
          Dq => data(31 downto 16), 
          Addr => address(14 downto 2),
          Ba => address(16 downto 15), 
          Clk => sdclk, 
          Cke => '1',
          Cs_n => sdcsn(0),
          Ras_n => sdrasn,
          Cas_n => sdcasn,
          We_n => sdwen,
          Dqm => sddqm(3 downto 2)
      );
      
  u1: mt48lc16m16a2
      generic map (
          index => 16, 
          fname => sdramfile
      )
	    port map (
          Dq => data(15 downto 0), 
          Addr => address(14 downto 2), 
          Ba => address(16 downto 15), 
          Clk => sdclk, 
          Cke => '1',
          Cs_n => sdcsn(0), 
          Ras_n => sdrasn, 
          Cas_n => sdcasn, 
          We_n => sdwen,
          Dqm => sddqm(1 downto 0)
      );

  prom0 : sram
      generic map (index => 6, abits => romdepth, fname => promfile)
	    port map (address(romdepth-1 downto 0), data(31 downto 24), romsn(0), writen, oen);

  errorn <= 'H';			  -- ERROR pull-up

  data <= buskeep(data) after 5 ns;

  --begin
  --  wait;
  --end process;
end ;

