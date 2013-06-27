Configuration Wizard {#wizard}
==============================

The SoCRocket configuration wizard can be used to generate and reconfigure system simulations. 
The tool can be started with following command:
~~~
./waf generate
~~~
The wizard offers a dialog, which allows the user to select a template and a configuration, or create a new configuration for an existing template. 
All parameters of the template will be displayed in an input mask. 

 
 Figure 1 - Configuration Wizard

The configurations created by the wizard can be copied and then modified with a simple text editor, due to the fact that they are pure JSON files. 
The tools `get_json_attr` and `set_json_attr` can help to modify them for example in automated design space explorations.

Change Attributes:
~~~
./tools/set_json_attr templates/leon3mp.singlecore.json conf.system.ncpu=2 > templates/leon3.dualcore.json
~~~

To navigate through a file use `get_json_attr`:
~~~
./tools/get_json_attr templates/leon3mp.json conf
~~~
*(Will show all attributes in conf)*

More detailed information about the SoCRocket configuration wizard can be found in RD10.
  

