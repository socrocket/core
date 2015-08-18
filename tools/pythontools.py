#!/usr/bin/env python
import json
import sys

def get_python_attr(pythonfile, attrname = None):
  filename = pythonfile.strip('"').strip("'")
  obj = {}
  with open(filename, "r") as script:
      code = compile(script.read(), filename, 'exec')
      exec(code, {}, obj)

  if attrname:
    for idx in attrname.split('.') or []:
      obj = obj[idx]

  if isinstance(obj, str):
    return obj

  else:
    return json.dumps(obj, sort_keys=True, indent=2)
