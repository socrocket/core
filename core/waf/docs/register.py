#!/usr/bin/python
# -*- coding: utf-8 -*-
import re
import sys
from collections import OrderedDict
from error_handler import ReportError
from doxycomment import ExtractCommentBlocks,\
		IsCommentBlockStart, IsCommentBlockEnd,\
		IsInCommentBlock, GetCommentBlock,\
		SplitLine, BlockHandler


def ProcessRegister(lineArray, registerPosition, CommentBlocks):
  startreg_ex = re.compile("^(?P<start>\W*(\*|///)?\W*)(@|\\|/)register (?P<id>\w+)(\W*(?P<title>.*))$")
  bits_ex = re.compile("^(?P<start>\W*(\*|///)?\W*)\[(?P<bit>\d+)(:(?P<endbit>\d+))?\]\W*\((?P<name>[^)]+)\)(?P<desc>.*)$")
  bitfollow_ex = re.compile("^(?P<start>\W*(\*|///)?\W*)(?P<desc>.*)$")
  endreg_ex = re.compile("^(?P<start>\W*(\*|///)?\W*)(@|\\|/)endregister\W*$")

  class Proxy(object):
    def __call__(self, res):
      self.__res = res
      return res

    def __getattr__(self, name):
      return getattr(self.__res,name)

  class Empty(object):
    pass

  lines = []
  curr = None
  field = None
  res = Proxy()
  for lineNumber in range(registerPosition["Start"], registerPosition["End"]+1):
    line = lineArray[lineNumber]
    if not curr:
      if res(startreg_ex.match(line)):
        curr = Empty()
        curr.id = res.group('id')
        curr.title = res.group('title')
        curr.start = res.group('start')
        curr.bits = []
      else:
        lines.append(line)
    else:
      if res(bits_ex.match(line)):
        field = Empty()
        field.bit = int(res.group('bit'))
        field.endbit = res.group('endbit')
        field.name = res.group('name')
        field.desc = res.group('desc').strip() + ' '
        curr.bits.append(field)
      elif res(endreg_ex.match(line)):
        start = curr.start
        ordered = sorted(curr.bits, key=lambda field: field.bit, reverse=True)
        lines.append(start+"@anchor %s" %(curr.id))
        lines.append(start+'<table class="register_view">')
        lines.append(start+"<caption>%s</caption><tr>" % (curr.title))
        begins = []
        ends = []
        ones = []
        for item in ordered:
          if item.endbit:
            begins.append(int(item.bit))
            ends.append(int(item.endbit))
          else:
            ones.append(int(item.bit))
        if (len(begins)==0 or begins[0] != 31) and (len(ones)==0 or ones[0] != 31):
          begins.append(31)
        if (len(ends)==0 or ends[-1] != 0) and (len(ones)==0 or ones[-1] != 0):
          ends.append(0)

        for i in range(31, -1, -1):
          if i in begins:
            thclass = """ class="begin" """
          elif i in ends:
            thclass = """ class="end" """
          elif i in ones:
            thclass = """ class="bit" """
          else:
            thclass = None
          if thclass:
            lines.append(start+"<th%s>%d</th>" % (thclass, i))
          else:
            lines.append(start+"<th></th>")
        index = 31
        lines.append(start+"</tr><tr>")
        for item in ordered:
          bit = int(item.bit)
          span = bit - (int(item.endbit or bit)-1)
          offset = index - bit
          if offset:
            lines.append(start+"""<td colspan="%d" class="empty">&nbsp;</td>""" % (offset))
          index = int(item.endbit or bit) -1
          lines.append(start+"""<td colspan="%d">%s</td>""" % (span, item.name))
        if index != -1:
          lines.append(start+"""<td colspan="%d" class="empty">&nbsp;</td>""" % (index+1))

        lines.append(start+"</tr></table>")

        lines.append(start+'<table class="register_list">')
        lines.append(start+"<caption>%s Description</caption>" % (curr.title))
        lines.append(start+'<tr class="register_list_header"><th class="register_table_bits">Bits</th><th>Id</th><th>Description</th></tr>')
        for item in ordered:
          if item.endbit:
            bits = "%s - %s" % (item.bit, item.endbit)
          else:
            bits = "%s" % (item.bit)
          lines.append(start+"""<tr><td>%s</td><td>%s</td><td>%s</td></tr>""" % (bits, item.name, item.desc))
        lines.append(start+"</table>")
        # Put Table into lines
        curr = None
      elif res(bitfollow_ex.match(line)):
        field.desc += res.group('desc').strip() + ' '

  return lines

def RegisterHandler(lineArray, options):
  return BlockHandler(lineArray, options, 
      StartDelimiter='@register',
      EndDelimiter='@endregister',
      Processor=ProcessRegister)

if __name__ == "__main__":
	# Parse command line
	from doxygen_preprocessor import CommandLineHandler
	options, remainder = CommandLineHandler()

	from filterprocessor import FilterFiles
	FilterFiles([RegisterHandler,], options, remainder)

