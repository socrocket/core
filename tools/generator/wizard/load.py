from builtins import str
from PyQt4 import QtCore, QtGui

class LoadPage(QtGui.QWizardPage):
    def __init__(self, templates, parent=None):
        super(LoadPage, self).__init__(parent)

        #self.setTitle("Load Configuration")
        #self.setSubTitle("Choose the configuration you whant to use to alter it.")
        #self.setPixmap(QtGui.QWizard.WatermarkPixmap,
        #        QtGui.QPixmap(':/images/watermark1.png'))

        self.label1 = QtGui.QLabel("Please choose between ...")
        self.radio1 = QtGui.QRadioButton("default configuration")
        self.label2 = QtGui.QLabel("or one of the following configuration settings to start with.")
        self.radio2 = QtGui.QRadioButton("vvv")
        self.radio1.setChecked(True)
        self.view = QtGui.QListWidget()
        self.templates = templates
        self.conf = None
        layout = QtGui.QVBoxLayout()
        layout.addWidget(self.label1)
        layout.addWidget(self.radio1)
        layout.addWidget(self.label2)
        layout.addWidget(self.radio2)
        layout.addWidget(self.view)
        self.setLayout(layout)
        def clicked(checked):
            self.conf = None
        self.radio1.clicked.connect(clicked)
        
        def select():
            items = self.view.selectedItems()
            if len(items)==0:
                self.radio1.setChecked(True)
                self.conf = None
            else:
                self.radio2.setChecked(True)
                self.conf = items[0].text()
            self.completeChanged.emit()
        self.view.itemSelectionChanged.connect(select)
        self.view.setFocus(QtCore.Qt.ActiveWindowFocusReason)
    
    def initializePage(self):
        self.view.clear()
        self.conf = None
        self.ready = False
        self.view.setFocus(QtCore.Qt.ActiveWindowFocusReason)
        self.completeChanged.emit()
        for conf in self.templates.listConfigurations():
          self.view.addItem(conf)
    
    def validatePage(self):
        if self.radio2.isChecked():
          self.templates.loadConfiguration(str(self.conf))
        return True
    
