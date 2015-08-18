from builtins import str
from PyQt4 import QtCore, QtGui
import os

class SavePage(QtGui.QWizardPage):
    def __init__(self, templates, parent=None):
        super(SavePage, self).__init__(parent)

        #self.setTitle("Save Configuration")
        #self.setSubTitle("Under which name you whant to save your configuration?")
        #self.setPixmap(QtGui.QWizard.WatermarkPixmap,
        #        QtGui.QPixmap(':/images/watermark1.png'))

        self.label = QtGui.QLabel("Configuration name:")
        self.view = QtGui.QLineEdit()
        self.templates = templates
        layout = QtGui.QVBoxLayout()
        layout.addWidget(self.label)
        layout.addWidget(self.view)
        self.setLayout(layout)
    
    def initializePage(self):
        self.view.setText(self.templates.getConfiguration())
        self.view.selectAll()

    def validatePage(self):
        name = str(self.view.text())
        self.templates.storeConfiguration(name)
        return True
