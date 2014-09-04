from PyQt4 import QtCore, QtGui
import os
import shutil

class GeneratePage(QtGui.QWizardPage):
    def __init__(self, templates, parent=None):
        super(GeneratePage, self).__init__(parent)

        #self.setTitle("Generate Platform")
        #self.setSubTitle("Please wait while the wizard is crafting your platform...")
        #self.setPixmap(QtGui.QWizard.WatermarkPixmap,
        #        QtGui.QPixmap(':/images/watermark1.png'))

        self.label2 = QtGui.QLabel("Instructions:")
        self.text = QtGui.QTextBrowser()
        self.templates = templates
        layout = QtGui.QVBoxLayout()
        layout.addWidget(self.label2)
        layout.addWidget(self.text)
        self.setLayout(layout)
        self.ready = False
    
    def isComplete(self):
        return self.ready

    def initializePage(self):
        self.text.setText(self.templates.getTemplate().instructions % (self.templates.getTemplate().getConfigurationFile()))
        self.ready = False
        self.templates.generate()
        self.ready = True
        self.completeChanged.emit()
