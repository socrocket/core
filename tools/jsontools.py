#!/usr/bin/env python
"""
@addtogroup tools
@file jsontools.py
"""

import json
import sys

def get_json_attr(jsonfile, attrname = None):
  """
  Find a json attribute
  """
  filename = jsonfile.strip('"').strip("'")
  filehandler = open(filename)
  obj = json.load(filehandler)

  if attrname:
    for idx in attrname.split('.') or []:
      obj = obj[idx]

  if isinstance(obj, str) or isinstance(obj, str):
    return obj

  else:
    return json.dumps(obj, sort_keys=True, indent=2)


def set_json_attr(jsonfile, attributes=[]):
  filename = jsonfile.strip('"').strip("'")
  filehandler = open(filename)
  root = json.load(filehandler)
  for param in attributes:
    obj = root
    pair = param.split('=')
    
    if len(pair) != 2:
      sys.exit("Parameter %s is wrong please use 'key=value' as pattern" % (param))

    (key, value) = pair
    if value.isdigit():
      value = int(value)
    elif value == "true":
      value = True
    elif value == "false":
      value = False

    path = key.split('.') or []
    for idx in path:
      if idx in obj:
        if idx != path[-1]:
          obj = obj[idx]
        else:
          obj[idx] = value

      else:
        if idx != path[-1]:
          obj[idx] = {}
          obj = obj[idx]
        else:
          obj[idx] = value

  return json.dumps(root, sort_keys=True, indent=2)

