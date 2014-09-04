from PyQt4 import QtCore, QtGui
import os

class ConfigPage(QtGui.QWizardPage):
    def __init__(self, templates, parent=None):
        super(ConfigPage, self).__init__(parent)

        #self.setTitle("Configuration")
        #self.setSubTitle("Alter configuration and build your own platform.")
        #self.setPixmap(QtGui.QWizard.WatermarkPixmap,
        #        QtGui.QPixmap(':/images/watermark1.png'))
    
        self.templates = templates
        
        self.view = QtGui.QTreeView()
        self.panel = QtGui.QWidget()
        self.info = QtGui.QTextBrowser()
        self.hsplit = QtGui.QSplitter(QtCore.Qt.Vertical)
        self.vsplit = QtGui.QSplitter(QtCore.Qt.Horizontal)
        self.hsplit.addWidget(self.panel)
        self.hsplit.addWidget(self.info)
        self.vsplit.addWidget(self.view)
        self.vsplit.addWidget(self.hsplit)
    
        def click(index):
            item = index.internalPointer()
            self.info.setText(QtCore.QVariant(item.description).toString())
            self.model.clicked(item)

        self.view.activated.connect(click)
        self.view.entered.connect(click)
        self.view.clicked.connect(click)
        #self.view.setModel(model)
 
        self.layout = QtGui.QGridLayout()
        self.layout.addWidget(self.vsplit)
        #self.setStyleSheet("* { background: yellow }")
        #self.setMaximumHeight(0xFFFFFF)
        #self.vsplit.setMaximumHeight(0xFFFFFF)
        #self.hsplit.setMaximumHeight(0xFFFFFF)
        #self.view.setMaximumHeight(0xFFFFFF)
        self.setLayout(self.layout)
        #self.hsplit.moveSplitter(340,0)

    def initializePage(self):
        self.panel.setParent(None)
        self.panel = QtGui.QWidget()
        self.hsplit.insertWidget(0, self.panel)
        self.model = self.templates.getModel(self.panel)
        self.view.setModel(self.model)
        self.view.expandAll()
        self.view.setColumnWidth(0, 220)
        self.view.setColumnWidth(1, 20)
        self.setLayout(self.layout)
        #self.vsplit.moveSplitter(280,1)
        #self.hsplit.moveSplitter(120,1)
