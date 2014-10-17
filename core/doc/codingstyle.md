SoCRocket Coding Style Guide {#codingstyle_p}
============================

The SoCRocket Coding Style Guide is derived from the Google C++ Style Guide 
(Revision 3.274, http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml)

Many thanks to Google and Benjy Weinberger, Craig Silverstein, 
Gregory Eitzmann, Mark Mentovai andTashana Landray for their excellent style guide.

To ensure the usage of that Style Guide several helper tools are in place to support 
the development cycle.

To check the coding style use the script `tools/cpplint`. It is simply called:
    
    $ ./tool/cpplint <filename> ...
       
All files can be checked with:
   
    $ ./waf cpplint
       
Furthermore to simplify the migration process of old code to the codingstyle you may use the tool `uncrustify` which is available here: http://uncrustify.sourceforge.net/

A configuration is availabe in `tools/uncrustify.cfg`. Make sure you use at least version 0.60 of `uncrustify`. It can be used like this:
   
    $ uncrustify -c tools/uncrustify.cfg --replace <filename>

To update/add header guards in a coding style compliant way the script `tools/fixheaderguards.py` can be used. The script was originally created by Martin Preisler and slightly adapted to our coding style. The original is available here: https://bitbucket.org/mpreisler/include-guard-fixer/

Just run it from the project root like this:
   
    $ tools/fixheaderguards.py <filename>
       

Coding Style adjustments done in the SoCRocket Week 2014-6
----------------------------------------------------------

| Model        | Lint | GUARDS | DOCS | DOCSUP | DOXY | CONF | REGS |  
|--------------|------|--------|------|--------|------|------|------|
| ahbcamera    | OK   |  OK    | OK   |        | OK   |      |      |
| ahbctrl      | OK   |  OK    | OK   |        | OK   |      |      |
| ahbdisplay   | OK   |  OK    | OK   |        | OK   |      |      |
| ahbin        | OK   |  OK    | OK   |        |      |      |      |
| ahbmem       | OK   |  OK    | OK   |        | OK   |      |      |
| ahbout       | OK   |  OK    | OK   |        |      |      |      |
| ahbprof      | OK   |  OK    | OK   |        | OK   |      |      |
| ahbshuffler  | OK   |  OK    | OK   |        | OK   |      |      |
| ahbspacewire |      |  OK    |      |        |      |      |      |
| ahbudp       | OK   |  OK    |      |        | OK   |      |      |
| apbctrl      | OK   |  OK    | OK   |        |      |      |      |
| apbuart      | OK   |  OK    | OK   |        |      |      |      |
| gptimer      | OK   |  OK    | OK   |        | OK   |      |      |
| greth        |      |        |      |        |      |      |      |
| irqmp        | OK   |  OK    | OK   | OK     |      |      |      |
| mctrl        | RM   |  OK    | OK   |        |      |      |      |
| memory       | OK   |  OK    | OK   | OK     | OK   |      |      |
| mips         | OK   |        |      | OK     |      |      |      |
| mmu_cache    | TS   |        |      |        |      |      |      |
| socwire      |      |  OK    |      |        |      |      |      |
| stimgen      | OK   |  OK    |      |        |      |      |      |
| utils        | OK   |  OK    | OK   |        | OK   | --   |      |
| common       | RM   |        |      |        |      | --   |      |
| leon3mp      |      |        |      |        |      |      |      |
