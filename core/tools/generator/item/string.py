from __future__ import absolute_import
from builtins import str
from PyQt4 import QtCore, QtGui

from .item import Item

class StringItem(Item):
    def __init__(self, model, name = None, var = None, value = None, type = None, range = None, default = None, description = None, parent=None, data=None):
        super(StringItem, self).__init__(model, name, var, value, type, range, default, description, parent, data)
        
        if model.widget:
          self.widget = QtGui.QWidget(model.widget)
          self.layout = QtGui.QFormLayout(self.widget)
          self.name_label = QtGui.QLabel("Name: ", self.widget)
          self.name_obj = QtGui.QLabel(self.name, self.widget)
          self.value_label = QtGui.QLabel("Value: ", self.widget)
          self.value_obj = QtGui.QLineEdit(self.value.toString(), self.widget)
          self.type_label = QtGui.QLabel("Type: ", self.widget)
          self.type_obj = QtGui.QLabel("String", self.widget)
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
            self.value_obj.setText(value.toString())
        self.setData = setData
        
        def setValue(value):
          self.value = QtCore.QVariant(value)
          self.model.layoutChange() 
        self.setValue = setValue
        if self.widget:
          self.value_obj.textChanged.connect(self.setValue)
    
    def child(self, row):
            return Null

    def childCount(self):
            return 0

    def save(self):
        return self.value.toString()

    def load(self, data):
        ownData = data.get(str(self.var), None)
        if ownData and isinstance(ownData, string):
                self.setData(QtCore.QVariant(ownData))
