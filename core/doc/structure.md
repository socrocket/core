Library Structure {#structure}
=============================

Next to the actual TLM simulation models, the library contains an extensive set of unit tests and support-functions for building and analyzing simulations. 
The top-level of the library is structured as follows:

## adapters     

This folder contains a set of SystemC/VHDL adapters/transactors, which are used for the unit tests of the library. 
For more interformation on transactors, please see the Interconnection Methodology Summary [RD9]. 

## common

Contains utility classes and functions, which are jointly used by all models of the library.
This mainly comprises timing monitor, power monitor,  endianess conversion and verbosity control. 
More detailed explanantions on the seperate files in this directory are given in 3.2.

## contrib

Contains a set of patches for GreenSocs 4.2.0 and newly developed TLM Sockets that enable GreenReg registers to be bound to Carbon AHB sockets.

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

