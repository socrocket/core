from PyQt4 import QtCore, QtGui

class Item(object):
    def __init__(self, model, name = None, var = None, value = None, type = None, range = None, default = None, description = None, parent=None, widget=None):    
        self.parentItem = parent
        self.childItems = []
        
        self.model = model
        self.name = QtCore.QVariant(name).toString()
        self.var = QtCore.QVariant(var).toString()
        self.value = QtCore.QVariant(value)
        self.type = QtCore.QVariant(type).toString()
        self.range = QtCore.QVariant(range).toString()
        self.default = QtCore.QVariant(default)
        self.description = QtCore.QVariant(description).toString()

    def appendChild(self, item):
        self.childItems.append(item)

    def child(self, row):
        return self.childItems[row]

    def childCount(self):
        return len(self.childItems)

    def columnCount(self):
        return 3

    def data(self, column):
        if column == 0:
            return self.name
        elif column == 1:
            return self.value
        elif column == 2:
            return self.type
        else:
            return None

    def parent(self):
        return self.parentItem

    def row(self):
        if self.parentItem:
            return self.parentItem.childItems.index(self)

        return 0

    def getWidget(self):
        self.widget

    def value(self):
        return self.value

    def setValue(self, value):
        self.value = QtCore.QVariant(value)
        self.model.layoutChange()
        

    def name(self):
        return self.name

    def setName(self, value):
        self.name = name

    def type(self):
        return self.type

    def setType(self, value):
        self.type = value

    def range(self):
        return self.range

    def setRange(self, value):
        self.range = value

    def default(self):
        return default

    def setDefault(self, value):
        self.default = value

    def description(self):
        return self.description

    def setDescription(self, value):
        self.description = value
    
