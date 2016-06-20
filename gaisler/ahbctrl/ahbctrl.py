import usi
from usi.sc_object import attach
from sr_registry import USIDelegateBase

class AHBCtrl(USIDelegateBase):
    """This class ist an AHBCtrl"""
    __usi_group__ = "module"
    __usi_class__ = "AHBCtrl"

    sta_power_count = 0.0
    int_power_count = 0.0
    swi_power_count = 0.0

    def get_power(self):
        result = {}
        num_of_bindings = float((int(self.counters.num_of_slave_bindings.cci_read()) + int(self.counters.num_of_master_bindings.cci_read())))
        # Static power calculation (pW)
        result["static"] = int(self.power.sta_power_norm.cci_read()) * num_of_bindings
        # Dynamic power (switching independent internal power)
        result["internal"] = int(self.power.int_power_norm.cci_read()) * num_of_bindings * 1.0 / self.get_clk()
        # Energy per read access (uJ)
        dyn_read_energy = int(self.power.dyn_read_energy_norm.cci_read()) * num_of_bindings
        # Energy per write access (uJ)
        dyn_write_energy = int(self.power.dyn_write_energy_norm.cci_read()) * num_of_bindings
        result["switching"] = ((dyn_read_energy * int(self.counters.dyn_reads.cci_read())) + \
                               (dyn_write_energy * int(self.counters.dyn_writes.cci_read()))) / \
                               (sc_time_stamp() - int(self.power.power_frame_starting_time.cci_read())).to_seconds()

    def print_masters(self):
        ahbin = [child for child in self.children() if child.basename() == 'ahbIN']
        if any(ahbin):
            ahbin = ahbin[0]
            num = ahbin.socket_get_num_bindings()
            for i in range(0, num):
                master = ahbin.socket_get_other_side(i).parent()
                print master.name(), master.get_ahb_vendor_id(), master.get_ahb_device_id(), master.get_ahb_hindex()

    def print_slaves(self):
        ahbin = [child for child in self.children() if child.basename() == 'ahbOUT']
        if any(ahbin):
            ahbin = ahbin[0]
            num = ahbin.socket_get_num_bindings()
            for i in range(0, num):
                slvae =  ahbin.socket_get_other_side(i).parent()
                print slave.name(), slave.get_ahb_vendor_id(), slave.get_ahb_device_id(), slave.get_ahb_hindex(), slave.get_ahb_size_()

"""
# Old
def print_masters(self):
    ahbin = [child for child in self.children() if child.basename() == 'ahbIN']
    if any(ahbin):
        ahbin = ahbin[0]
        num = ahbin.socket_get_num_bindings()
        for i in range(0, num):
            master = ahbin.socket_get_other_side(i).parent()
            print master.name(), master.get_ahb_vendor_id(), master.get_ahb_device_id(), master.get_ahb_hindex()

attach("module.AHBCtrl", "print_masters", print_masters)

def print_slaves(self):
    ahbin = [child for child in self.children() if child.basename() == 'ahbOUT']
    if any(ahbin):
        ahbin = ahbin[0]
        num = ahbin.socket_get_num_bindings()
        for i in range(0, num):
            slvae =  ahbin.socket_get_other_side(i).parent()
            print slave.name(), slave.get_ahb_vendor_id(), slave.get_ahb_device_id(), slave.get_ahb_hindex(), slave.get_ahb_size_()
attach("module.AHBCtrl", "print_slaves", print_slaves)
"""
# New
"""
usi.load("ahbctrl")

class AHBCtrl(usi.Object("module.AHBCtrl")):
    def print_masters(self):
        pass
    def print_slaves(self):
        pass
"""
#usi.onCommandFrom("AHBCtrl")
