library ieee;
use ieee.std_logic_1164.all;
library grlib, techmap;
use grlib.amba.all;
use grlib.stdlib.all;
use techmap.gencomp.all;
library gaisler;
use gaisler.memctrl.all;
use gaisler.leon3.all;
use gaisler.misc.all;

library esa;
use esa.memoryctrl.all;

use work.config.all;

entity leon3mp is
  generic (
    fabtech       : integer := CFG_FABTECH;
    memtech       : integer := CFG_MEMTECH;
    padtech       : integer := CFG_PADTECH;
    clktech       : integer := CFG_CLKTECH;
    disas         : integer := CFG_DISAS;	-- Enable disassembly to console
    pclow         : integer := CFG_PCLOW
  );
  port (
    resetn	  : in  std_ulogic;
    clk		    : in  std_ulogic; 	-- 50 MHz main clock
    pllref 	  : in  std_ulogic; 
    errorn	  : out std_ulogic;
    wdogn  	  : out std_ulogic;
    address 	: out std_logic_vector(27 downto 0);
    data	    : inout std_logic_vector(31 downto 0);
    ramsn  	  : out std_logic_vector (4 downto 0);
    ramoen 	  : out std_logic_vector (4 downto 0);
    rwen   	  : out std_logic_vector (3 downto 0);
    oen    	  : out std_ulogic;
    writen 	  : out std_ulogic;
    read   	  : out std_ulogic;
    iosn   	  : out std_ulogic;
    romsn  	  : out std_logic_vector (1 downto 0);
    sdclk  	  : out std_ulogic;
    sdcsn  	  : out std_logic_vector (1 downto 0);    -- sdram chip select
    sdwen  	  : out std_ulogic;                       -- sdram write enable
    sdrasn  	: out std_ulogic;                       -- sdram ras
    sdcasn  	: out std_ulogic;                       -- sdram cas
    sddqm   	: out std_logic_vector (3 downto 0)     -- sdram dqm
	);
end;

architecture rtl of leon3mp is

attribute syn_netlist_hierarchy : boolean;
attribute syn_netlist_hierarchy of rtl : architecture is false;

constant blength : integer := 12;
constant fifodepth : integer := 8;
constant maxahbm : integer := CFG_NCPU;

signal vcc, gnd   : std_logic_vector(4 downto 0);
signal memi  : memory_in_type;
signal memo  : memory_out_type;
signal wpo   : wprot_out_type;
signal sdo   : sdram_out_type;

signal apbi  : apb_slv_in_type;
signal apbo  : apb_slv_out_vector := (others => apb_none);
signal ahbsi : ahb_slv_in_type;
signal ahbso : ahb_slv_out_vector := (others => ahbs_none);
signal ahbmi : ahb_mst_in_type;
signal ahbmo : ahb_mst_out_vector := (others => ahbm_none);

signal clkm, rstn, rstraw, sdclkl : std_ulogic;
signal cgi, cgi2   : clkgen_in_type;
signal cgo, cgo2   : clkgen_out_type;

signal irqi : irq_in_vector(0 to CFG_NCPU-1);
signal irqo : irq_out_vector(0 to CFG_NCPU-1);

signal dbgi : l3_debug_in_vector(0 to CFG_NCPU-1);
signal dbgo : l3_debug_out_vector(0 to CFG_NCPU-1);

signal gpti : gptimer_in_type;
signal gpto : gptimer_out_type;

signal lclk, rst, wdogl : std_ulogic;

constant BOARD_FREQ : integer := 50000;   -- input frequency in KHz
constant CPU_FREQ : integer := BOARD_FREQ * CFG_CLKMUL / CFG_CLKDIV;  -- cpu frequency in KHz
constant IOAEN : integer := CFG_CAN + CFG_ATA + CFG_GRUSBDC;

signal clk50 : std_logic;  -- signals to vga_clkgen.
signal clk_sel : std_logic_vector(1 downto 0);
                         
attribute keep : boolean;
attribute syn_keep : boolean;
attribute syn_preserve : boolean;
attribute syn_keep of clk50 : signal is true;
attribute syn_preserve of clk50 : signal is true;
attribute keep of clk50 : signal is true;

begin

----------------------------------------------------------------------
---  Reset and Clock generation  -------------------------------------
----------------------------------------------------------------------
  
  vcc <= (others => '1'); gnd <= (others => '0');
  cgi.pllctrl <= "00"; cgi.pllrst <= rstraw;

  pllref_pad : clkpad 
    generic map (tech => padtech) 
    port map (pllref, cgi.pllref); 
  
  clk_pad : clkpad 
    generic map (tech => padtech) 
    port map (clk, lclk); 
  
  clkgen0 : clkgen  		-- clock generator
    generic map (clktech, CFG_CLKMUL, CFG_CLKDIV, CFG_MCTRL_SDEN, CFG_CLK_NOFB, 0, 0, 0, BOARD_FREQ)
    port map (lclk, lclk, clkm, open, open, sdclkl, open, cgi, cgo, open, clk50);

  sdclk_pad : outpad 
    generic map (tech => padtech, slew => 1, strength => 24) 
    port map (sdclk, sdclkl);

  resetn_pad : inpad generic map (tech => padtech) port map (resetn, rst); 
  rst0 : rstgen			-- reset generator
    port map (rst, clkm, cgo.clklock, rstn, rstraw);

----------------------------------------------------------------------
---  AHB CONTROLLER --------------------------------------------------
----------------------------------------------------------------------

  ahb0 : ahbctrl 		-- AHB arbiter/multiplexer
    generic map (defmast => CFG_DEFMST, split => CFG_SPLIT, rrobin => CFG_RROBIN, ioaddr => CFG_AHBIO, ioen => IOAEN, nahbm => maxahbm, nahbs => 8)
    port map (rstn, clkm, ahbmi, ahbmo, ahbsi, ahbso);

----------------------------------------------------------------------
---  LEON3 processor and DSU -----------------------------------------
----------------------------------------------------------------------

  l3 : if CFG_LEON3 = 1 generate
    cpu : for i in 0 to CFG_NCPU-1 generate
      u0 : leon3s			-- LEON3 processor      
        generic map (i, fabtech, memtech, CFG_NWIN, CFG_DSU, CFG_FPU, CFG_V8, 0, CFG_MAC, pclow, 0, CFG_NWP, CFG_ICEN, CFG_IREPL, CFG_ISETS, CFG_ILINE, CFG_ISETSZ, CFG_ILOCK, CFG_DCEN, CFG_DREPL, CFG_DSETS, CFG_DLINE, CFG_DSETSZ, CFG_DLOCK, CFG_DSNOOP, CFG_ILRAMEN, CFG_ILRAMSZ, CFG_ILRAMADDR, CFG_DLRAMEN, CFG_DLRAMSZ, CFG_DLRAMADDR, CFG_MMUEN, CFG_ITLBNUM, CFG_DTLBNUM, CFG_TLB_TYPE, CFG_TLB_REP,  CFG_LDDEL, disas, CFG_ITBSZ, CFG_PWD, CFG_SVT, CFG_RSTADDR, CFG_NCPU-1, 0, 0, CFG_MMU_PAGE)
        port map (clkm, rstn, ahbmi, ahbmo(i), ahbsi, ahbso, irqi(i), irqo(i), dbgi(i), dbgo(i));
    end generate;
    
    errorn_pad : odpad 
      generic map (tech => padtech) 
      port map (errorn, dbgo(0).error);
    
    nouah : apbo(7) <= apb_none;
  end generate;
  
----------------------------------------------------------------------
---  Memory controllers ----------------------------------------------
----------------------------------------------------------------------

  memi.writen <= '1'; memi.wrn <= "1111"; memi.bwidth <= "00";
  mctrl0 : mctrl generic map (hindex => 0, pindex => 0,
	paddr => 0, srbanks => 2, ram8 => CFG_MCTRL_RAM8BIT, 
	ram16 => CFG_MCTRL_RAM16BIT, sden => CFG_MCTRL_SDEN, 
	invclk => CFG_CLK_NOFB, sepbus => CFG_MCTRL_SEPBUS,
	pageburst => CFG_MCTRL_PAGE)
  port map (rstn, clkm, memi, memo, ahbsi, ahbso(0), apbi, apbo(0), wpo, sdo);
  sdpads : if CFG_MCTRL_SDEN = 1 generate 		-- SDRAM controller
      sdwen_pad : outpad generic map (tech => padtech) 
	   port map (sdwen, sdo.sdwen);
      sdras_pad : outpad generic map (tech => padtech) 
	   port map (sdrasn, sdo.rasn);
      sdcas_pad : outpad generic map (tech => padtech) 
	   port map (sdcasn, sdo.casn);
      sddqm_pad : outpadv generic map (width =>4, tech => padtech) 
	   port map (sddqm, sdo.dqm(3 downto 0));
  end generate;
  sdcsn_pad : outpadv generic map (width =>2, tech => padtech) 
	   port map (sdcsn, sdo.sdcsn); 

  addr_pad : outpadv generic map (width => 28, tech => padtech) 
	port map (address, memo.address(27 downto 0)); 
  rams_pad : outpadv generic map (width => 5, tech => padtech) 
	port map (ramsn, memo.ramsn(4 downto 0)); 
  roms_pad : outpadv generic map (width => 2, tech => padtech) 
	port map (romsn, memo.romsn(1 downto 0)); 
  oen_pad  : outpad generic map (tech => padtech) 
	port map (oen, memo.oen);
  rwen_pad : outpadv generic map (width => 4, tech => padtech) 
	port map (rwen, memo.wrn); 
  roen_pad : outpadv generic map (width => 5, tech => padtech) 
	port map (ramoen, memo.ramoen(4 downto 0));
  wri_pad  : outpad generic map (tech => padtech) 
	port map (writen, memo.writen);
  read_pad : outpad generic map (tech => padtech) 
	port map (read, memo.read); 
  iosn_pad : outpad generic map (tech => padtech) 
	port map (iosn, memo.iosn);
  bdr : for i in 0 to 3 generate
      data_pad : iopadv generic map (tech => padtech, width => 8)
      port map (data(31-i*8 downto 24-i*8), memo.data(31-i*8 downto 24-i*8),
	memo.bdrive(i), memi.data(31-i*8 downto 24-i*8));
  end generate;

----------------------------------------------------------------------
---  APB Bridge and various periherals -------------------------------
----------------------------------------------------------------------

  apb0 : apbctrl				-- AHB/APB bridge
    generic map (hindex => 1, haddr => CFG_APBADDR, nslaves => 16)
    port map (rstn, clkm, ahbsi, ahbso(1), apbi, apbo );

  noua0 : apbo(1) <= apb_none;
  noua1 : apbo(9) <= apb_none;

  irqctrl0 : irqmp			-- interrupt controller
    generic map (pindex => 2, paddr => 2, ncpu => CFG_NCPU)
    port map (rstn, clkm, apbi, apbo(2), irqo, irqi);
    
  irq3 : if CFG_IRQ3_ENABLE = 0 generate
    x : for i in 0 to CFG_NCPU-1 generate
      irqi(i).irl <= "0000";
    end generate;
    apbo(2) <= apb_none;
  end generate;

  gpt : if CFG_GPT_ENABLE /= 0 generate
    timer0 : gptimer 			-- timer unit
    generic map (pindex => 3, paddr => 3, pirq => CFG_GPT_IRQ, 
	sepirq => CFG_GPT_SEPIRQ, sbits => CFG_GPT_SW, ntimers => CFG_GPT_NTIM, 
	nbits => CFG_GPT_TW, wdog => CFG_GPT_WDOGEN*CFG_GPT_WDOG)
    port map (rstn, clkm, apbi, apbo(3), gpti, gpto);
    gpti.dhalt <= '0'; gpti.extclk <= '0';
  end generate;
  wden : if CFG_GPT_WDOGEN /= 0 generate
    wdogl <= gpto.wdogn or not rstn;
    wdogn_pad : odpad generic map (tech => padtech) port map (wdogn, wdogl);
  end generate;
  wddis : if CFG_GPT_WDOGEN = 0 generate
    wdogn_pad : odpad generic map (tech => padtech) port map (wdogn, vcc(0));
  end generate;

  nogpt : if CFG_GPT_ENABLE = 0 generate apbo(3) <= apb_none; end generate;
  nokbd : apbo(4) <= apb_none; apbo(5) <= apb_none;
  novga : apbo(6) <= apb_none;
  
-----------------------------------------------------------------------
---  Drive unused bus elements  ---------------------------------------
-----------------------------------------------------------------------

--  nam1 : for i in (CFG_NCPU+CFG_AHB_UART+CFG_GRETH+CFG_AHB_JTAG) to NAHBMST-1 generate
--    ahbmo(i) <= ahbm_none;
--  end generate;
--  nap0 : for i in 11 to NAPBSLV-1 generate apbo(i) <= apb_none; end generate;
--  nah0 : for i in 8 to NAHBSLV-1 generate ahbso(i) <= ahbs_none; end generate;

-----------------------------------------------------------------------
---  Boot message  ----------------------------------------------------
-----------------------------------------------------------------------

-- pragma translate_off
  x : report_version 
  generic map (
   msg1 => "LEON3 GR-XC3S-1500 Demonstration design",
      msg2 => "GRLIB Version " & tost(LIBVHDL_VERSION/1000) & "." & tost((LIBVHDL_VERSION mod 1000)/100)
        & "." & tost(LIBVHDL_VERSION mod 100) & ", build " & tost(LIBVHDL_BUILD),
   msg3 => "Target technology: " & tech_table(fabtech) & ",  memory library: " & tech_table(memtech),
   mdel => 1
  );
-- pragma translate_on
end;
