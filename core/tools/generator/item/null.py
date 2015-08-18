from __future__ import absolute_import
from builtins import str
from PyQt4 import QtCore, QtGui

from .item import Item

class NullItem(Item):
    def __init__(self, model, name = None, var = None, value = None, type = None, range = None, default = None, description = None, parent=None, data=None):
        super(NullItem, self).__init__(model, name, var, value, type, range, default, description, parent, data)
        if model.widget: 
          self.widget = QtGui.QWidget(model.widget)
        else:
          self.widget = None

    def save(self):
        return dict([[str(n.var), n.save()] for n in self.childItems])

    def load(self, data):
        ownData = data.get(str(self.var), None)
        if ownData:
            for child in self.childItems:
                child.load(ownData)
