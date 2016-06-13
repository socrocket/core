#! /usr/bin/env python
# vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 filetype=python :
top = '..'

def build(self):
  self.recurse_all()
  self.recurse_all_tests()
  """
  self(
        target       = 'sr_gaisler',
        features     = 'cxx cxxshlib',
        #source       = ['ahbctrl',
        #                'ahbmem',
        #                'irqmp',
        #                'gptimer',
        #                'apbctrl',
        #                'apbuart',
        #                'reset_irqmp',
        #                'mctrl',
        #                'ahbin',
        #                'ahbprof', 
        #                'leon3',
        #               ],
        source = [
            'ahbctrl/ahbctrl.cpp',
            'ahbin/ahbin.cpp',
            'ahbmem/ahbmem.cpp',
            'ahbout/ahbout.cpp',
            'ahbprof/ahbprof.cpp',
            'apbctrl/apbctrl.cpp',
            'apbuart/apbuart.cpp',
            'apbuart/reportio.cpp',
            'apbuart/tcpio.cpp',
            'gptimer/gptimer.cpp',
            'gptimer/gpcounter.cpp',
            'irqmp/irqmp.cpp',
            'mctrl/mctrl.cpp',
            'reset_irqmp/reset_irqmp.cpp',
            'leon3/mmucache/icio_payload_extension.cpp', 
            'leon3/mmucache/dcio_payload_extension.cpp',
            'leon3/mmucache/localram.cpp',
            'leon3/mmucache/nocache.cpp',
            'leon3/mmucache/vectorcache.cpp',
            'leon3/mmucache/ivectorcache.cpp',
            'leon3/mmucache/dvectorcache.cpp',
            'leon3/mmucache/mmu.cpp', 
            'leon3/mmucache/mmu_cache.cpp',
            'leon3/mmucache/mmu_cache_base.cpp',
            'leon3/mmucache/defines.cpp',
            'leon3/intunit/instructions.cpp',
            'leon3/intunit/registers.cpp',
            'leon3/intunit/alias.cpp',
            'leon3/intunit/processor.cpp',
            'leon3/intunit/interface.cpp',
            'leon3/intunit/decoder.cpp',
            'leon3/intunit/memory.cpp',
            'leon3/intunit/irqPorts.cpp',
            'leon3/intunit/externalPins.cpp',
            'leon3/leon3.cpp',
        ] + self.path.ant_glob('memory/*.cpp'),
        include = self.top_dir,
        defines = 'ENABLE_HISTORY', 
        use          = [
                        'trap',
                        'sr_registry', 'sr_register', 'sr_report', 'sr_signal', 'common',
                        'AMBA', 'GREENSOCS', 'TLM', 'SYSTEMC', 'BOOST'
                       ],
  )
  """
