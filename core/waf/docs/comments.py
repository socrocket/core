#!/usr/bin/python
# -*- coding: utf-8 -*-

'''
Usage:
If you want to temporarily comment out some markdown stuff
just add encapsulate it in @startcomment and @endcomment
delimeters like this:

@startcomment
this text wont show up later
@endcomment
'''

import sys
from collections import OrderedDict
from error_handler import ReportError
from doxycomment import ExtractCommentBlocks,\
		IsCommentBlockStart, IsCommentBlockEnd,\
		IsInCommentBlock, GetCommentBlock,\
		SplitLine, BlockHandler


def ProcessComments(lineArray, registerPosition, CommentBlocks):

  lines = []

  return lines

def CommentsHandler(lineArray, options):
  return BlockHandler(lineArray, options, 
      StartDelimiter='@startcomment',
      EndDelimiter='@endcomment',
      Processor=ProcessComments)

if __name__ == "__main__":
	# Parse command line
	from doxygen_preprocessor import CommandLineHandler
	options, remainder = CommandLineHandler()

	from filterprocessor import FilterFiles
	FilterFiles([RegisterHandler,], options, remainder)

