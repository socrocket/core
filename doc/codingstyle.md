SoCRocket Coding Style Guide {#codingstyle_p}
============================

The SoCRocket Coding Style Guide is derived from the Google C++ Style Guide 
(Revision 3.274, http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml)

Many thanks to Google and Benjy Weinberger, Craig Silverstein, 
Gregory Eitzmann, Mark Mentovai andTashana Landray for their excelent style guide.

To ensure the usage of that Style Guide serveral helper tools are in place to support 
the development cycle.

To check the coding style use the script "tools/cpplint". It is simply called:
~~~
./tool/cpplint <filename> ...
~~~
All files can be checked with:
~~~
./waf cpplint
~~~
Furthermore to simpliefy the migration process of old/other code to the codingstyle you may use "uncrustify":
http://uncrustify.sourceforge.net/

A configuration is availabe in tools/uncrustify.cfg. It cn be used with:
~~~
uncrustify -c tools/uncrustify.cfg --replace <filename>
~~~

To update/add header guards in a coding style compliant way the script "tools/fixheaderguards.py" can be used. The script was originally created by Martin Preisler and slightly adapted to our coding style. The original is available here: https://bitbucket.org/mpreisler/include-guard-fixer/

Just run it from the project root with:
~~~
tools/fixheaderguards.py <filename>
~~~

Coding Style done in the SoCRocket Week 2014-6
----------------------------------------------

Model        | Lint | GUARDS | DOCS | DOCSUP | CONF | REGS  
------------ | ---- | ------ | ---- | ------ | ---- | ----
ahbcamera    | OK   |  OK    |      |        |      |
ahbctrl      |      |        |      |        |      |
ahbdisplay   | OK   |  OK    |      |        |      |
ahbin        | OK   |  OK    |      |        |      |
ahbmem       | RM   |        |      |        |      |
ahbout       | RM   |        |      |        |      |
ahbprof      | RM   |        |      |        |      |
ahbshuffler  | RM   |        |      |        |      |
ahbspacewire |      |        |      |        |      |
ahbudp       | RM   |        |      |        |      |
apbctrl      |      |        |      |        |      |
apbuart      | RM   |        |      |        |      |
extern       |      |        |      |        |      |
gptimer      | OK   | OK     |      |        |      |
greth        |      |        |      |        |      |
irqmp        | RM   |        |      |        |      |
mctrl        |      |        |      |        |      |
memory       | JW   |        |      |        |      |
mips         | RM   |        |      |        |      |
mmu_cache    | TS   |        |      |        |      |
socwire      |      |        |      |        |      |
stimgen      |      |        |      |        |      |
utils        |      |        |      |        |      |

