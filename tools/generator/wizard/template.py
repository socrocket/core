from builtins import str
from PyQt4 import QtCore, QtGui

class TemplatePage(QtGui.QWizardPage):
    def __init__(self, templates, parent=None):
        super(TemplatePage, self).__init__(parent)

        #self.setTitle("Template")
        #self.setSubTitle("Choose the template you whant to use.")
        #self.setPixmap(QtGui.QWizard.WatermarkPixmap,
        #        QtGui.QPixmap(':/images/watermark1.png'))
        #self.setStyleSheet("* { background: yellow;}")
        self.templates = templates
        layout = QtGui.QVBoxLayout(self)
        self.hsplit = QtGui.QSplitter(self)
        self.hsplit.setOrientation(QtCore.Qt.Horizontal)
        self.view = QtGui.QListWidget(self.hsplit)
        self.hsplit.addWidget(self.view)
        self.info = QtGui.QTextBrowser(self.hsplit)
        self.hsplit.addWidget(self.info)
        #self.registerField('template')
        self.template = None
        self.ready = False
        layout.addWidget(self.hsplit)
        #self.setLayout(layout)
        def select():
            items = self.view.selectedItems()
            if len(items)==0:
                self.template = Null
                self.ready = False
            else:
                self.template = self.templates[str(items[0].text())]
                self.templates.setTemplate(self.template)
                self.info.setText(("<h1>%s</h1><br/>" % (self.template.name)) + QtCore.QVariant(self.template.description).toString())
                self.ready = True
            self.completeChanged.emit()
        self.view.itemSelectionChanged.connect(select)
    
    def getTemplate(self):
        return self.template

    def isComplete(self):
        return self.ready

    def initializePage(self):
        self.view.clear()
        self.template = None
        self.ready = False
        self.completeChanged.emit()
        for base in list(self.templates.keys()):
          self.view.addItem(base)

