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
