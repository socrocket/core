from __future__ import absolute_import
from builtins import str
from PyQt4 import QtCore, QtGui

from .item import Item

class BooleanItem(Item):
    def __init__(self, model, name = None, var = None, value = None, type = None, range = None, default = None, description = None, parent=None, data=None):
        super(BooleanItem, self).__init__(model, name, var, value, type, range, default, description, parent, data)
        
        if model.widget:
          self.widget = QtGui.QWidget(model.widget)
          self.layout = QtGui.QFormLayout(self.widget)
          self.name_label = QtGui.QLabel("Name: ", self.widget)
          self.name_obj = QtGui.QLabel(self.name, self.widget)
          self.value_label = QtGui.QLabel("Value: ", self.widget)
          self.value_obj = QtGui.QCheckBox("", self.widget)
          self.type_label = QtGui.QLabel("Type: ", self.widget)
          self.type_obj = QtGui.QLabel("Boolean", self.widget)
          self.default_label = QtGui.QLabel("Default: ", self.widget)
          self.default_obj = QtGui.QLabel(self.default.toString(), self.widget)
          self.layout.addRow(self.name_label)
          self.layout.addRow(self.name_obj)
          self.layout.addRow(self.value_label, self.value_obj)
          self.layout.addRow(self.type_label, self.type_obj)
          self.layout.addRow(self.default_label, self.default_obj)
          self.value_obj.setCheckState(self.value.toBool() * 2)
        else:
          self.widget = None

        def setData(value):
          self.value = value
          if self.widget:
            self.value_obj.setCheckState(value.toBool() * 2)
        self.setData = setData
        
        def setValue(value):
          if value == 0:
            self.value = QtCore.QVariant(False)
          elif value == 1:
            self.value = QtCore.QVariant(False)
          else:
            self.value = QtCore.QVariant(True)
          self.model.layoutChange() 
        self.setValue = setValue
        if self.widget:
          self.value_obj.stateChanged.connect(self.setValue)
    
    def child(self, row):
        if self.value.toInt() != 0:
            return self.childItems[row]
        else:
            return Null

    def childCount(self):
        if self.value.toBool() != False:
            return len(self.childItems)
        else:
            return 0

    def save(self):
        if len(self.childItems)>0:
            if self.value.toBool():
                return dict([[str(n.var), n.save()] for n in self.childItems])
            else:
                return False
        else:
            return self.value.toBool()

    def load(self, data):
        ownData = data.get(str(self.var), None)
        if ownData != None:
            if isinstance(ownData, bool):
                self.setData(QtCore.QVariant(ownData))
            elif len(ownData) > 0:
                self.setData(QtCore.QVariant(True))
                for child in self.childItems:
                    child.load(ownData)
            else:
                self.setData(QtCore.QVariant(False))
