from PyQt4 import QtCore, QtGui

from item import Item

class IntegerItem(Item):
    def __init__(self, model, name = None, var = None, value = None, type = None, range = None, default = None, description = None, parent=None, data=None):
        super(IntegerItem, self).__init__(model, name, var, value, type, range, default, description, parent, data)
        
        self.widget = QtGui.QWidget(model.widget)
        self.layout = QtGui.QFormLayout(self.widget)
        #r = QtCore.QVariant(range).toString().split('..')
        #print r[0]
        self.min = 0 #r[0]
        self.max = 1000 #r[1]
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
        self.layout.addRow(self.name_label, self.name_obj)
        self.layout.addRow(self.value_label, self.value_obj)
        self.layout.addRow(self.type_label, self.type_obj)
        self.layout.addRow(self.range_label, self.range_obj)
        self.layout.addRow(self.default_label, self.default_obj)
        self.value_obj.setMinimum(self.min)
        self.value_obj.setMaximum(self.max)
        self.value_obj.setValue(self.value.toInt()[0])
        self.value_obj.valueChanged.connect(self.setValue)
        def setData(value):
          self.value = value
          self.value_obj.setValue(value.toInt()[0])
        self.setData = setData


