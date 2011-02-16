from PyQt4 import QtCore, QtGui
import os

class SavePage(QtGui.QWizardPage):
    def __init__(self, template, name, conf, parent=None):
        super(SavePage, self).__init__(parent)

        self.setTitle("Save Configuration")
        self.setSubTitle("Under which name you whant to save your configuration?")
        #self.setPixmap(QtGui.QWizard.WatermarkPixmap,
        #        QtGui.QPixmap(':/images/watermark1.png'))

        self.label = QtGui.QLabel("Configuration name:")
        self.view = QtGui.QLineEdit()
        self.template = template
        self.name = name
        self.conf = conf
        layout = QtGui.QVBoxLayout()
        layout.addWidget(self.label)
        layout.addWidget(self.view)
        self.setLayout(layout)
    
    def initializePage(self):
        if self.name.conf:
            self.view.setText(self.name.conf)
        else:
            self.view.setText("unknown")
        self.view.selectAll()

    def validatePage(self):
        name = os.path.join("configurations", '.'.join((str((self.template.template.base)), str(self.view.text()), "cfg")))
        #if os.path.exists(name):
        self.conf = view.text()
        self.conf.save(name)
        return True
