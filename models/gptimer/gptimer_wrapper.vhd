library ieee;
use ieee.std_logic_1164.all;
library grlib;
use grlib.amba.all;
use grlib.stdlib.all;
use grlib.devices.all;
library gaisler;
use gaisler.misc.all;
--pragma translate_off
use std.textio.all;
--pragma translate_on

--constant NAPBIR  : integer := 1;  -- maximum APB configuration words
--constant NAPBAMR : integer := 1;  -- maximum APB configuration words
--constant NAPBCFG : integer := NAPBIR + NAPBAMR;  -- words in APB config block
--subtype amba_config_word is std_logic_vector(31 downto 0);
--type apb_config_type is array (0 to NAPBCFG-1) of amba_config_word;

entity gptimer_wrapper is
  generic (
    gpindex  : integer := 0;
    gpaddr   : integer := 0;
    gpmask   : integer := 16#fff#;
    gpirq    : integer := 0;
    sepirq   : integer := 0;    -- use separate interrupts for each timer
    sbits    : integer := 16;                   -- scaler bits
    ntimers  : integer range 1 to 7 := 1;       -- number of timers
    nbits    : integer := 32;                   -- timer bits
    gwdog    : integer := 0
  );
  port (
    rst      : in  std_ulogic;
    clk      : in  std_ulogic;
    -- apbi   : in  apb_slv_in_type;
    ipsel    : in std_logic_vector(0 to NAPBSLV-1);     -- slave select
    ipenable : in std_ulogic;                           -- strobe
    ipaddr   : in std_logic_vector(31 downto 0);        -- address bus (byte)
    ipwrite  : in std_ulogic;                           -- write
    ipwdata  : in std_logic_vector(31 downto 0);        -- write data bus
    ipirq    : in std_logic_vector(NAHBIRQ-1 downto 0); -- interrupt result bus
    testen   : in std_ulogic;                           -- scan test enable
    testrst  : in std_ulogic;                           -- scan test reset
    scanen   : in std_ulogic;                           -- scan enable
    testoen  : in std_ulogic;                           -- test output enable
    -- apbo   : out apb_slv_out_type;
    oprdata  : out std_logic_vector(31 downto 0);        -- read data bus
    opirq    : out std_logic_vector(NAHBIRQ-1 downto 0); -- interrupt bus
    --  pconfig : out apb_config_type;                 -- memory access reg.
    opconfir : out std_logic_vector(31 downto 0);
    opconfamr: out std_logic_vector(31 downto 0);
    --opindex  : out integer range 0 to NAPBSLV -1;        -- diag use only
    -- gpti : in gptimer_in_type;
    dhalt    : in  std_ulogic;
    extclk   : in  std_ulogic;
    -- gpto : out gptimer_out_type
    tick     : out std_logic_vector(0 to 7);
    timer1   : out std_logic_vector(31 downto 0);
    wdogn    : out std_ulogic;
    wdog     : out std_ulogic
  );
end; 
 
architecture behaviour of gptimer_wrapper is
component grtimer
  generic (
    pindex   : integer := 0;
    paddr    : integer := 0;
    pmask    : integer := 16#fff#;
    pirq     : integer := 0;
    sepirq   : integer := 0;    -- use separate interrupts for each timer
    sbits    : integer := 16;                   -- scaler bits
    ntimers  : integer range 1 to 7 := 1;       -- number of timers
    nbits    : integer := 32;                   -- timer bits
    wdog     : integer := 0
  );
  port (
    rst    : in  std_ulogic;
    clk    : in  std_ulogic;
    apbi   : in  apb_slv_in_type;
    apbo   : out apb_slv_out_type;
    gpti   : in  gptimer_in_type;
    gpto   : out gptimer_out_type
  );
end component;

signal apbi : apb_slv_in_type;
signal apbo : apb_slv_out_type;
signal gpti : gptimer_in_type;
signal gpto : gptimer_out_type;

begin
  apbi.psel    <= ipsel;
  apbi.penable <= ipenable;
  apbi.paddr   <= ipaddr;
  apbi.pwrite <= ipwrite;
  apbi.pwdata  <= ipwdata;
  apbi.pirq    <= ipirq;
  apbi.testen  <= testen;
  apbi.testrst <= testrst;
  apbi.scanen  <= scanen;
  apbi.testoen <= testoen;

  oprdata      <= apbo.prdata;
  opirq        <= apbo.pirq;
  opconfir     <= apbo.pconfig(0);
  opconfamr    <= apbo.pconfig(1);
  --opindex      <= apbo.pindex;

  gpti.dhalt   <= dhalt;
  gpti.extclk  <= extclk;

  tick         <= gpto.tick;
  timer1       <= gpto.timer1;
  wdogn        <= gpto.wdogn;
  wdog         <= gpto.wdog;

  timer : gptimer
    generic map (gpindex, gpaddr, gpmask, gpirq, sepirq, sbits, ntimers, nbits, gwdog)
    port map (rst, clk, apbi, apbo, gpti, gpto);
end;

