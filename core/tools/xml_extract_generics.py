from __future__ import print_function
import xml.etree.ElementTree as ET

tree = ET.parse('templates/leon3mp.tpa')
root = tree.getroot()

components = root.find('option')

for component in components:
  #print(component.tag, component.attrib)
  print("")
  print("void {0}::init_generics() {{".format(component.attrib['var']))
  print("  // set name, type, default, range, hint and description for gs_configs")
#        g_paddr.add_properties()
#          ("name", "APB Base Address")
#          ("range", "0..4095")
#          ("The 12bit MSB address at the APB bus");")
  attrs = component.findall('option')
  for attr in attrs:
    print("  g_{0}.add_properties()".format(attr.attrib['var']))
    
    if "name" in attr.attrib:
      print('    ("name", "{0}");'.format(attr.attrib['name']))
    
    if "range" in attr.attrib:
      print('    ("range", "{0}");'.format(attr.attrib['range']))
    
    if "hint" in attr.attrib:
      print('    ("{0}");'.format(attr.attrib['hint']))
    else:
      print('    ("{0}");'.format(attr.attrib['name']))
    #print(attr.tag, attr.attrib)

  print("}");
    
  
