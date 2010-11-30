from PyQt4 import QtCore, QtGui

from item import Item

class NullItem(Item):
    def __init__(self, model, name = None, var = None, value = None, type = None, range = None, default = None, description = None, parent=None, data=None):
        super(NullItem, self).__init__(model, name, var, value, type, range, default, description, parent, data)
 
        self.widget = QtGui.QWidget(model.widget)
