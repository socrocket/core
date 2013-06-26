#!/usr/bin/env python
import json
import sys

def get_python_attr(pythonfile, attrname = None):
  filename = pythonfile.strip('"').strip("'")
  obj = {}
  execfile(filename, {}, obj)

  if attrname:
    for idx in attrname.split('.') or []:
      obj = obj[idx]

  if isinstance(obj, str) or isinstance(obj, unicode):
    return obj

  else:
    return json.dumps(obj, sort_keys=True, indent=2)

