SoCRocket Coding Style Guide {#codingstyle_p}
============================

The SoCRocket Coding Style Guide is derived from the Google C++ Style Guide 
(Revision 3.274, http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml)

Many thanks to Google and Benjy Weinberger, Craig Silverstein, 
Gregory Eitzmann, Mark Mentovai andTashana Landray for their excellent style guide.

To ensure the usage of that Style Guide several helper tools are in place to support 
the development cycle.

To check the coding style use the script `core/tools/cpplint`. It is simply called:
    
    $ ./core/tool/cpplint <filename> ...
       
All files can be checked with:
   
    $ ./waf cpplint
       
Furthermore to simplify the migration process of old code to the codingstyle you may use the tool `uncrustify` which is available here: http://uncrustify.sourceforge.net/

A configuration is availabe in `core/tools/uncrustify.cfg`. Make sure you use at least version 0.60 of `uncrustify`. It can be used like this:
   
    $ uncrustify -c core/tools/uncrustify.cfg --replace <filename>

To update/add header guards in a coding style compliant way the script `tools/fixheaderguards.py` can be used. The script was originally created by Martin Preisler and slightly adapted to our coding style. The original is available here: https://bitbucket.org/mpreisler/include-guard-fixer/

Just run it from the project root like this:
   
    $ core/tools/fixheaderguards.py <filename>
       

Coding Style adjustments done in the SoCRocket Week 2014-6
----------------------------------------------------------

| Model        | Lint | GUARDS | DOCS | DOCSUP | DOXY | CONF | REGS |   |
|--------------|------|--------|------|--------|------|------|------|---|
| AHBCamera    | OK   | OK     | OK   |        | OK   |      |      |   |
| AHBCtrl      | OK   | OK     | OK   |        | OK   |      |      |   |
| AHBDisplay   | OK   | OK     | OK   |        | OK   |      |      |   |
| AHBIn        | OK   | OK     | OK   |        |      |      |      |   |
| AHBMem       | OK   | OK     | OK   |        | OK   |      |      |   |
| AHBOut       | OK   | OK     | OK   |        |      |      |      |   |
| AHBProf      | OK   | OK     | OK   |        | OK   |      |      |   |
| AHBShuffler  | OK   | OK     | OK   |        | OK   |      |      |   |
| AHBSpacewire |      | OK     |      |        |      |      |      |   |
| AHBUdp       | OK   | OK     |      |        | OK   |      |      |   |
| APBCtrl      | OK   | OK     | OK   |        |      |      |      |   |
| APBUart      | OK   | OK     | OK   |        |      |      |      |   |
| GPTimer      | OK   | OK     | OK   |        | OK   |      |      |   |
| GREth        |      |        |      |        |      |      |      |   |
| Irqmp        | OK   | OK     | OK   | OK     |      |      |      |   |
| Mctrl        | RM   | OK     | OK   |        |      |      |      |   |
| Memory       | OK   | OK     | OK   | OK     | OK   |      |      |   |
| Mips         | OK   |        |      | OK     |      |      |      |   |
| mmu_cache    | TS   |        |      |        |      |      |      |   |
| SoCWire      |      | OK     |      |        |      |      |      |   |
| Stimgen      | OK   | OK     |      |        |      |      |      |   |
| utils        | OK   | OK     | OK   |        | OK   | --   |      |   |
| common       | RM   |        |      |        |      | --   |      |   |
| leon3mp      |      |        |      |        |      |      |      |   |
