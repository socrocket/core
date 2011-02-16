from PyQt4 import QtCore, QtGui
from model import TreeModel
import json, os

class ConfigPage(QtGui.QWizardPage):
    def __init__(self, template, conf, parent=None):
        super(ConfigPage, self).__init__(parent)

        self.setTitle("Configuration")
        self.setSubTitle("Alter configuration and build your own platform.")
        #self.setPixmap(QtGui.QWizard.WatermarkPixmap,
        #        QtGui.QPixmap(':/images/watermark1.png'))
    
        self.template = template
        self.conf = conf
        
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
        tmpl = str(self.template.template.base)
        self.panel.setParent(None)
        self.panel = QtGui.QWidget()
        self.hsplit.insertWidget(0, self.panel)
        self.model = self.template.template.model(self.panel)
        if self.conf.conf:
            conf = str(self.conf.conf)
            cname = os.path.join('configurations', '.'.join((tmpl, conf, 'cfg')))
            self.load(cname)
        self.view.setModel(self.model)
        self.view.expandAll()
        self.view.setColumnWidth(0, 220)
        self.view.setColumnWidth(1, 20)
        self.setLayout(self.layout)
        self.vsplit.moveSplitter(280,1)
        self.hsplit.moveSplitter(120,1)

    def save(self, name):
        data = self.model.save()
        file = open(name, 'w')
        file.write(json.dumps(data))
        file.close()

    def load(self, name):
        file = open(name, 'r')
        data = ' '.join(file.readlines())
        file.close()
        data = json.loads(data)
        self.model.load(data)
