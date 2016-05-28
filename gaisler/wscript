#! /usr/bin/env python
# vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 filetype=python :
top = '..'

def build(self):
  self.recurse_all()
  self.recurse_all_tests()
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
            'reset_irqmp/reset_irqmp.cpp'
        ] + self.path.ant_glob('memory/*.cpp'),
        include = self.top_dir,
        use          = [
                        'trap',
                        'sr_registry', 'sr_register', 'sr_report', 'sr_signal', 'common',
                        'AMBA', 'GREENSOCS', 'TLM', 'SYSTEMC', 'BOOST'
                       ],
    )
