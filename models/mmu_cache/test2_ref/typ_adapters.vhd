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

package typ_adapters is

    type ahb_mst_out_type_adapter is record
      hbusreq	 : std_ulogic;                         	-- bus request
      hlock	 : std_ulogic;                         	-- lock request
      htrans	 : std_logic_vector(1 downto 0); 	-- transfer type
      haddr	 : std_logic_vector(31 downto 0); 	-- address bus (byte)
      hwrite	 : std_ulogic;                         	-- read/write
      hsize	 : std_logic_vector(2 downto 0); 	-- transfer size
      hburst	 : std_logic_vector(2 downto 0); 	-- burst type
      hprot	 : std_logic_vector(3 downto 0); 	-- protection control
      hwdata	 : std_logic_vector(31 downto 0); 	-- write data bus
      hirq   	 : std_logic_vector(NAHBIRQ-1 downto 0);	-- interrupt bus
      hconfig 	 : ahb_config_type;	 		-- memory access reg.
      hindex     : integer;                              -- diagnostic use only
    end record;

end typ_adapters;
