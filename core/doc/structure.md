Library Structure {#structure}
=============================

Next to the actual TLM simulation models, the library contains an extensive set of unit tests and support-functions for building and analyzing simulations. 
The top-level of the library is structured as follows:

* [<B>./core</B>](dir_7713b254b559bea8681f899f62bed013.html)
  * @ref core/common "/common"

    Contains utility classes and functions, which are jointly used by all models of the library.
    This mainly comprises timing monitor, power monitor,  endianess conversion and verbosity control. 
    More detailed explanations on the seperate files in this directory are given @ref common_doc "here". 

    * @ref core/common/grambasockets "/grambasockets"
    * @ref core/common/gs_config "/gs_config"
    * @ref core/common/signalkit_h "/signalkit_h"

      Home of the SoCRocket SignalKit. 
      The SignalKit comprises a set of functions/templates that allow signal communication in TLM-Style, without the overhead of maintaining payload objects.<BR> 
      Within the library it is mainly used to model interrupts and reset distribution. More detailed information can be found in @ref signalkit_doc "here".

    * @ref core/common/trapgen "/trapgen"
  * @ref core/doc "/doc"
    
    The documentation to this library which you are reading now.

  * @ref core/models "/models"
    
    The central folder containing all simulation models and unit tests. You can find documentation for the models linked in the @ref models_p "IP Models list". 
    Each model is located in a separate sub-directory. 
    The directory @ref core/models/extern "extern" contains external libraries which have been included in the models.

    * @ref core/models/ahbctrl "/ahbctrl"
    * @ref core/models/ahbin "/ahbin"
    * @ref core/models/ahbmem "/ahbmem"
    * @ref core/models/ahbout "/ahbout"
    * @ref core/models/ahbprof "/ahbprof"
    * @ref core/models/apbctrl "/apbctrl" 
    * @ref core/models/apbuart "/apbuart"
    * @ref core/models/extern "/extern"
    * @ref core/models/gptimer "/gptimer"
    * @ref core/models/irqmp "/irqmp"
    * @ref core/models/mctrl "/mctrl"
    * @ref core/models/memory "/memory"
    * @ref core/models/mmu_cache "/mmu_cache"
    * @ref core/common "/utils"
  * @ref core/platforms "/platforms"
    
    The platforms directory contains a subdirectory for all platforms. 
    These correlate to a .tpa file in the templates directory. At the moment only the leon3mp platform is working. @todo whats this tpa stuff? does it still work? 

    * @ref core/platforms/base "/base"
    * @ref core/platforms/leon3mp "/leon3mp"
    * @ref core/platforms/newleon3mp "/newleon3mp"
  * @ref core/software "/software"
 
    This directory contains benchmarks, tests and the OS for the platform. 

    * @ref core/software/fft64 "/fft64"
    * @ref core/software/grlib_tests "/grlib_tests"
    * @ref core/software/prom "/prom"
    * @ref core/software/trapgen "/trapgen"
  * @ref core/tools "/tools"
    
    A collection of small helpers for the SoCRocket library.
    
    * @ref core/tools/generator "/generator"

      The implementation of the Configuration Wizard. 
      The tool uses templates and configurations to generate platform instances. 
      Example templates can be found in the templates folder. 
      The platform sources are written to `./core/platforms/{name_of_platform}`. More detailed information is given in 2.4. @todo is the configuration wizard and generator documentation still needed?
 
  * @ref core/waf "/waf"
 
    Some extra modules for the waf build system to deal with modelsim, multi dependend tests and switching between host and SPARC compiler.

* @ref license_agpl "LICENSE.AGPL.md"  
* @ref license_com "LICENSE.COM.md"
* [<B>README.md</B>](index.html)
* waf

  WAF executable

* wscript

  Top-level build script

@startcomment
## adapters     

This folder contains a set of SystemC/VHDL adapters/transactors, which are used for the unit tests of the library. 
For more interformation on transactors, please see the Interconnection Methodology Summary [RD9]. 

## common

Contains utility classes and functions, which are jointly used by all models of the library.
This mainly comprises timing monitor, power monitor,  endianess conversion and verbosity control. 
More detailed explanantions on the seperate files in this directory are given in 3.2.

## doc

The documentation to this library.


## models

The central folder containing all simulation models and unit tests. 
Each model is located in a separate sub-directory. 
The directory extern contains external libraries which have been included in the models.

## platforms

The platforms directory contains a subdirectory for all platforms. 
These correlate to a .tpa file in the templates directory. At the moment there ist only the leon3mp platform. 

## signalkit

Home of the SoCRocket SignalKit. 
The SignalKit comprises a set of functions/templates that allow signal communication in TLM-Style, without the overhead of maintaining payload objects. 
Within the library it is mainly used to model interrupts and reset distribution. More detailed information can be found in 3.3.

## software

This directory contains benchmarks, tests and the OS for the platform. 

## templates

This directory contains example templates (.tpa) and configurations (.json), which can be used with the SoCRocket Configuration Wizard (tools/generator). 
User defined templates should also be stored in here.


## tools

A collection of small helpers for the SoCRocket library.


## tools/generator

The implementation of the Configuration Wizard. 
The tool uses templates and configurations to generate platform instances. 
Example templates can be found in the templates folder. 
The platform sources are written to ./platform/{name_of_platform}. More detailed information is given in 2.4.
 
## tools/waf

Some extra modules for the waf build system to deal with modelsim, multi dependend tests and switching between host and SPARC compiler.

## tools/get_json_attr

Get a specific JSON Attribute from a configuration

## tools/set_json_attr

Set some Attributes in a specific JSON File and dump the result to stdout

## tools/csv2json

Seperate a csv file line by line in JSON configurations.

## tools/doreport

Collect timing reports for a specific model test.

## Doxyfile

DoxyGen configuration file.

## waf

WAF executable

## wscript

Top-level build script
@endcomment

