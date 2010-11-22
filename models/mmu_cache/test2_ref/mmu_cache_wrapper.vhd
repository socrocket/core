library ieee;
use ieee.std_logic_1164.all;
library grlib;
use grlib.amba.all;
library techmap;
use techmap.gencomp.all;
library gaisler;
use gaisler.libiu.all;
use gaisler.libcache.all;
use gaisler.mmuconfig.all;
use gaisler.mmuiface.all;
use gaisler.libmmu.all;
library work;
use work.typ_adapters.all;


entity mmu_cache_wrapper is
  
  port (
    rst        : in  std_ulogic;
    clk        : in  std_ulogic;
    ici        : in  icache_in_type;
    ico        : out icache_out_type;
    dci        : in  dcache_in_type;
    dco        : out dcache_out_type;
    ahbi       : in  ahb_mst_in_type;
    ahbo       : out ahb_mst_out_type_adapter;
    fpuholdn   : in  std_ulogic;
    hclk, sclk : in std_ulogic;
    hclken     : in std_ulogic
  );
  
end mmu_cache_wrapper;

architecture test of mmu_cache_wrapper is

  -- define configuration constants
  -- ==============================
  -- Co-simulation can not map constructor parameters
  -- to VHDL generics.

  constant  hindex     : integer := 0;
  constant  memtech    : integer := 0;

  constant  dsu        : integer range 0 to 1 := 0;
  constant  icen       : integer range 0 to 1 := 1;
  constant  irepl      : integer range 0 to 2 := 2;
  constant  isets      : integer range 1 to 4 := 2;
  constant  ilinesize  : integer range 4 to 8 := 4;
  constant  isetsize   : integer range 1 to 256 := 64;
  constant  isetlock   : integer range 0 to 1 := 0;
  constant  dcen       : integer range 0 to 1 := 1;
  constant  drepl      : integer range 0 to 2 := 2;
  constant  dsets      : integer range 1 to 4 := 2;
  constant  dlinesize  : integer range 4 to 8 := 4;
  constant  dsetsize   : integer range 1 to 256 := 64;
  constant  dsetlock   : integer range 0 to 1 := 0;
  constant  dsnoop     : integer range 0 to 6 := 0;
  constant  ilram      : integer range 0 to 1 := 0;
  constant  ilramsize  : integer range 1 to 512 := 1;        
  constant  ilramstart : integer range 0 to 255 := 16#8e#;
  constant  dlram      : integer range 0 to 1 := 0;
  constant  dlramsize  : integer range 1 to 512 := 1;        
  constant  dlramstart : integer range 0 to 255 := 16#8f#;
  constant  cached     : integer := 0;
  constant  clk2x      : integer := 0;
  constant  scantest   : integer := 0;
  constant mmuen : integer := 0;
  
  signal icrami : cram_in_type;
  signal icramo : cram_out_type;
                                  

  signal ahbso_dummy : ahb_slv_out_vector;  -- do not route to systemc
  signal ahbsi_dummy : ahb_slv_in_type;
  signal ahbo_connect : ahb_mst_out_type;

begin  -- test

  ahbo.hbusreq <= ahbo_connect.hbusreq;
  ahbo.hlock   <= ahbo_connect.hlock;
  ahbo.htrans  <= ahbo_connect.htrans;
  ahbo.haddr   <= ahbo_connect.haddr;
  ahbo.hwrite  <= ahbo_connect.hwrite;
  ahbo.hsize   <= ahbo_connect.hsize;
  ahbo.hburst  <= ahbo_connect.hburst;
  ahbo.hprot   <= ahbo_connect.hprot;
  ahbo.hwdata  <= ahbo_connect.hwdata;
  ahbo.hirq    <= ahbo_connect.hirq;
  ahbo.hconfig <= ahbo_connect.hconfig;
  ahbo.hindex  <= ahbo_connect.hindex;
  
  mcache : cache
    generic map (
      hindex     => hindex,
      dsu        => dsu,
      icen       => icen,
      irepl      => irepl,
      isets      => isets,
      ilinesize  => ilinesize,
      isetsize   => isetsize,
      isetlock   => isetlock,
      dcen       => dcen,
      drepl      => drepl,
      dsets      => dsets,
      dlinesize  => dlinesize,
      dsetsize   => dsetsize,
      dsetlock   => dsetlock,
      dsnoop     => dsnoop,
      ilram      => ilram,
      ilramsize  => ilramsize,
      ilramstart => ilramstart,
      dlram      => dlram,
      dlramstart => dlramstart,
      dlramsize  => dlramsize,
      cached     => cached,
      clk2x      => clk2x,
      memtech    => memtech,
      scantest   => scantest )
    port map (
      rst      => rst,
      clk      => clk,
      ici      => ici,
      ico      => ico,
      dci      => dci,
      dco      => dco,
      ahbi     => ahbi,
      ahbo     => ahbo_connect,
      ahbsi    => ahbsi_dummy,
      ahbso    => ahbso_dummy,
      crami    => icrami,
      cramo    => icramo,
      fpuholdn => fpuholdn,
      hclk     => hclk,
      sclk     => sclk,
      hclken   => hclken);

  mcachemem : cachemem
    generic map (
      tech      => memtech,
      icen      => icen,
      irepl     => irepl,
      isets     => isets,
      ilinesize => ilinesize,
      isetsize  => isetsize,
      isetlock  => isetlock,
      dcen      => dcen,
      drepl     => drepl,
      dsets     => dsets,
      dlinesize => dlinesize,
      dsetsize  => dsetsize,
      dsetlock  => dsetlock,
      dsnoop    => dsnoop,
      ilram     => ilram,
      ilramsize => ilramsize,
      dlram     => dlram,
      dlramsize => dlramsize,
      mmuen     => mmuen)
    port map (
      clk       => clk,
      crami     => icrami,
      cramo     => icramo,
      sclk      => sclk);

end test;

