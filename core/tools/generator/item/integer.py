from __future__ import absolute_import
from builtins import str
from builtins import range
from PyQt4 import QtCore, QtGui
from copy import copy, deepcopy
from .item import Item
from .null import NullItem
import sys

class Null(NullItem):
    def row(self):
        if self.parentItem:
            return self.parentItem.numbers.index(self)
        return 0

    def getVar(self):
        if self.parentItem:
            return int(self.parentItem.numbers.index(self))
        
    def load(self, data):
        for child in self.childItems:
            child.load(data)

class IntegerItem(Item):
    #def str2int(self, s):
    #    #s = str(s)
    #    if s.startswith("0x"):
    #      return int(s[2:], 16)
    #    else:
    #      return int(s)
    #
    def __init__(self, model, name = None, var = None, value = None, type = None, range_ = None, default = None, description = None, parent=None, data=None):
        super(IntegerItem, self).__init__(model, name, var, value, type, range_, default, description, parent, data)
        self.numbers = []
        for i in range(0, self.value.toInt()[0]):
          item = Null(model, name = str(i), parent=self)
          if model.widget:
            self.model.stack.addWidget(item.widget)
          for child in self.childItems:
            c = child.__class__(child.model, child.name, child.var, child.value, child.type, child.range, child.default, child.description, item)
            item.appendChild(c)
            if model.widget:
              self.model.stack.addWidget(c.widget)
          self.numbers.append(item)
       
        if model.widget:
          self.widget = QtGui.QWidget(model.widget)
          self.layout = QtGui.QFormLayout(self.widget)
          r = str(QtCore.QVariant(range_).toString()).split('..')
          if len(r)<2:
            r = ["-2147483648","2147483647"]
          self.min = int(r[0], 0)
          self.max = int(r[1], 0)
          
          self.name_label = QtGui.QLabel("Name: ", self.widget)
          self.name_obj = QtGui.QLabel(self.name, self.widget)
          self.value_label = QtGui.QLabel("Value: ", self.widget)
          self.value_obj = QtGui.QSpinBox(self.widget)
          self.type_label = QtGui.QLabel("Type: ", self.widget)
          self.type_obj = QtGui.QLabel("Integer", self.widget)
          self.range_label = QtGui.QLabel("Range: ", self.widget)
          self.range_obj = QtGui.QLabel("From %s to %s" % (self.min, self.max), self.widget)
          self.default_label = QtGui.QLabel("Default: ", self.widget)
          self.default_obj = QtGui.QLabel(self.default.toString(), self.widget)
          self.layout.addRow(self.name_label)
          self.layout.addRow(self.name_obj)
          self.layout.addRow(self.value_label, self.value_obj)
          self.layout.addRow(self.type_label, self.type_obj)
          self.layout.addRow(self.range_label, self.range_obj)
          self.layout.addRow(self.default_label, self.default_obj)
          self.value_obj.setMinimum(self.min)
          self.value_obj.setMaximum(self.max)
          self.value_obj.setValue(self.value.toInt()[0])
        else:
          self.widget = None
      
        def setData(value):
          self.value = value
          if self.widget:
            self.value_obj.setValue(value.toInt()[0])
          if value.toInt()[0] > len(self.numbers):
            for i in range(len(self.numbers), value.toInt()[0]):
              item = Null(self.model, name=str(i), parent=self)
              if self.widget:
                self.model.stack.addWidget(item.widget)
              for child in self.childItems:
                c = child.__class__(child.model, child.name, child.var, child.value, child.type, child.range, child.default, child.description, item)
                item.appendChild(c)
                if self.widget:
                  self.model.stack.addWidget(c.widget)
              self.numbers.append(item)
          self.model.layoutChange() 
        self.setData = setData
        
        def setValue(value):
          self.value = QtCore.QVariant(value)
          if len(self.childItems)!=0 and value > len(self.numbers):
            for i in range(len(self.numbers), value):
              item = Null(self.model, name=str(i), parent=self)
              self.model.stack.addWidget(item.widget)
              for child in self.childItems:
                c = child.__class__(child.model, child.name, child.var, child.value, child.type, child.range, child.default, child.description, item)
                item.appendChild(c)
                if self.widget:
                  self.model.stack.addWidget(c.widget)
              self.numbers.append(item)      
          self.model.layoutChange()
        self.setValue = setValue
        if self.widget:
          self.value_obj.valueChanged.connect(self.setValue)

    def appendChild(self, child):
        super(IntegerItem, self).appendChild(child)
        for i in self.numbers:
            c = child.__class__(child.model, child.name, child.var, child.value, child.type, child.range, child.default, child.description, i)
            i.appendChild(c)
            if self.widget:
              self.model.stack.addWidget(c.widget)

    def child(self, row):
        if self.value != 0 or self.value != False:
            return self.numbers[row]
        else:
            return None

    def childCount(self):
        if len(self.childItems) != 0:
            return self.value.toInt()[0]
        else:
            return 0
   
    def save(self):
        if len(self.childItems)>0:
            return [self.numbers[n].save() for n in range(self.value.toInt()[0])]
        else:
            return int(str(self.value.toString()), 0)

    def load(self, data):
        ownData = data.get(str(self.var), None)
        if ownData!=None:
            if isinstance(ownData, int) or isinstance(ownData, int):
                self.setData(QtCore.QVariant(ownData))
            elif len(ownData)>0:
                self.setData(QtCore.QVariant(len(ownData)))
                for i in range(self.value.toInt()[0]):
                    self.numbers[i].load(ownData[i])
            else:
                self.setData(QtCore.QVariant(0))

