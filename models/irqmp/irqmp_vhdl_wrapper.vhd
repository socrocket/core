--/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/--
-- Project:    HW-SW SystenC Co-Simulation SoC Validation Platform     --
--                                                                     --
-- File:       irqmp_testtop_ct.cpp                                    --
--             test file for vhdl implementation of irqmp              --
--             needs sc_wrapper around the vhdl module for simulation  --
--                                                                     --
-- Modified on $Date: 2010-05-19 14:17:08 +0200 (Wed, 19 May 2010) $   --
--          at $Revision: 7 $                                          --
--                                                                     --
-- Principal:  European Space Agency                                   --
-- Author:     VLSI working group @ IDA @ TUBS                         --
-- Maintainer: Dennis Bode                                             --
--/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/--


library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
library grlib;
use grlib.amba.all;
use grlib.stdlib.all;
use grlib.devices.all;
library gaisler;
use gaisler.leon3.all;


-- empty entity
entity irqmp_wrapper is
  generic (
    pindex  : integer := 0;
    paddr   : integer := 0;
    pmask   : integer := 16#fff#;
    ncpu    : integer := 2;
    eirq    : integer := 6
  );
    port (
      rst    : in  std_ulogic;
      clk    : in  std_ulogic;
      apbi   : in  apb_slv_in_type;
      apbo   : out apb_slv_out_type;

      irqi_0 : in  l3_irq_out_type;
      irqi_1 : in  l3_irq_out_type;

      irqo_0 : out l3_irq_in_type;
      irqo_1 : out l3_irq_in_type
    );
end entity irqmp_wrapper;

architecture bhv of irqmp_wrapper is

  --module declaration
  component irqmp is
  generic (
    pindex  : integer := 0;
    paddr   : integer := 0;
    pmask   : integer := 16#fff#;
    ncpu    : integer := 2;
    eirq    : integer := 6
  );
    port (
      rst  : in  std_ulogic;
      clk  : in  std_ulogic;
      apbi : in  apb_slv_in_type;
      apbo : out apb_slv_out_type;
      irqi : in  irq_out_vector(0 to 1);
      irqo : out irq_in_vector(0 to 1)
    );
  end component;

  --input signals
--  signal rst  : std_ulogic;
--  signal clk  : std_ulogic := '1';
--  signal apbi : apb_slv_in_type;
--  signal irqi : irq_out_vector(0 to 1);

  --output signals
--  signal apbo : apb_slv_out_type;
--  signal irqo : irq_in_vector(0 to 1);

begin --architecture

  --module instantiation
  dut : irqmp
    port map (
      rst  => rst,
      clk  => clk,
      apbi => apbi,
      apbo => apbo,
      irqi(0) => irqi_0,
      irqi(1) => irqi_1,
      irqo(0) => irqo_0,
      irqo(1) => irqo_1
    );

end architecture;
