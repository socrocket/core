#!/usr/bin/env python
from PyQt4 import QtCore, QtGui
from item import *
import vconfig_rc

class TreeModel(QtCore.QAbstractItemModel):
    def __init__(self, path, widget, parent=None):
        super(TreeModel, self).__init__(parent)

        self.widget = widget
        self.stack = QtGui.QStackedLayout(widget)
        self.rootItem = NullItem(self, name="Name", var="root", value="Value", type="Type")
        self.readFile(path)

    def clicked(self, item):
        self.stack.setCurrentWidget(item.widget)
        item.widget.show()

    def layoutChange(self):
        self.layoutAboutToBeChanged.emit()
        self.layoutChanged.emit()

    def dataChange(self, index):
        self.dataChanged.emit(index, index)

    def treeChange(self):
        self.modelReset.emit()

    def columnCount(self, parent):
        if parent.isValid():
            return parent.internalPointer().columnCount()
        else:
            return self.rootItem.columnCount()

    def data(self, index, role):
        if not index.isValid():
            return None

        if role != QtCore.Qt.DisplayRole:
            return None

        item = index.internalPointer()

        return item.data(index.column())

    def setData(self, index, value, role):
      if not index.isValid():
        return False
      #elif role != QtCore.Qt.DisplayRole:
      #  print "setData elif" 
      #  return False
      else:
        item = index.internalPointer()
        item.setData(value)
        return True

    def flags(self, index):
        if not index.isValid():
            return QtCore.Qt.NoItemFlags
        if index.column() == 1:
          return QtCore.Qt.ItemIsEnabled | QtCore.Qt.ItemIsSelectable | QtCore.Qt.ItemIsEditable
        else:
          return QtCore.Qt.ItemIsEnabled | QtCore.Qt.ItemIsSelectable

    def headerData(self, section, orientation, role):
        if orientation == QtCore.Qt.Horizontal and role == QtCore.Qt.DisplayRole:
            return self.rootItem.data(section)

        return None

    def index(self, row, column, parent):
        if not self.hasIndex(row, column, parent):
            return QtCore.QModelIndex()

        if not parent.isValid():
            parentItem = self.rootItem
        else:
            parentItem = parent.internalPointer()

        childItem = parentItem.child(row)
        if childItem:
            i = self.createIndex(row, column, childItem)
            childItem.index = i
            return i
        else:
            return QtCore.QModelIndex()

    def parent(self, index):
        if not index.isValid():
            return QtCore.QModelIndex()

        childItem = index.internalPointer()
        parentItem = childItem.parent()

        if parentItem == self.rootItem:
            return QtCore.QModelIndex()

        i = self.createIndex(parentItem.row(), 0, parentItem)
        parentItem.inxed = i
        return i

    def rowCount(self, parent):
        if parent.column() > 0:
            return 0

        if not parent.isValid():
            parentItem = self.rootItem
        else:
            parentItem = parent.internalPointer()

        return parentItem.childCount()
      
    def hasChildren(self, index):
        item = index.internalPointer()
        if not hasattr(item, "childCount"):
            return True
        return item.childCount() > 0
      
    def readFile(self, path):
      def readItem(node, parent):
        name = node.getAttribute("name")
        var = node.getAttribute("var")
        type = node.getAttribute("type")
        range = node.getAttribute("range")
        default = node.getAttribute("default")
        value = node.getAttribute("value")
        if not value or value == "":
          value = default
        hint = node.getAttribute("hint")
        if type == "int":
            item = IntegerItem(self, name, var, value, type, range, default, hint, parent)
        elif type == "bool":
            item = BooleanItem(self, name, var, value, type, range, default, hint, parent)
        else:
            item = NullItem(self, name, var, value, type, range, default, hint, parent)

        for cnode in node.childNodes:
          if cnode.nodeName == "option":
            citem = readItem(cnode, item)
        parent.appendChild(item)
        self.stack.addWidget(item.widget)
        return item
        
      from xml.dom.minidom import parse
      dom = parse(path)
      root = dom.documentElement
      item = readItem(root, self.rootItem)
      #self.rootItem.appendChild(item)
      
def main(file, args):
    app = QtGui.QApplication(args)

    view = QtGui.QTreeView()
    panel = QtGui.QWidget()
    control = QtGui.QWidget()
    controllayout = QtGui.QHBoxLayout()
    info = QtGui.QTextBrowser()
    load = QtGui.QPushButton("Load ...")
    save = QtGui.QPushButton("Save ...")
    cancle = QtGui.QPushButton("Cancle ...")
    next = QtGui.QPushButton("Continue ...")
    controllayout.addWidget(load)
    controllayout.addWidget(save)
    controllayout.addStretch(1)
    controllayout.addWidget(cancle)
    controllayout.addWidget(next)
    control.setLayout(controllayout)
    hsplit = QtGui.QSplitter(QtCore.Qt.Vertical)
    vsplit = QtGui.QSplitter(QtCore.Qt.Horizontal)
    hsplit.addWidget(panel)
    hsplit.addWidget(info)
    hsplit.addWidget(control)
    vsplit.addWidget(view)
    vsplit.addWidget(hsplit)
    
    model = TreeModel(file, panel)
    
    def click(index):
      item = index.internalPointer()
      info.setText(QtCore.QVariant(item.description).toString())
      model.clicked(item)

    view.activated.connect(click)
    view.entered.connect(click)
    view.clicked.connect(click)
    view.setModel(model)
    vsplit.resize(800,600)
    vsplit.setWindowTitle("System Configurator")
    vsplit.show()
    return app.exec_()
    
if __name__ == '__main__':
    import sys

    sys.exit(main("default.xml", sys.argv))
