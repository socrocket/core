from PyQt4 import QtGui

class IntroPage(QtGui.QWizardPage):
    def __init__(self, parent=None):
        super(IntroPage, self).__init__(parent)

        self.setTitle("Introduction")
        #self.setPixmap(QtGui.QWizard.WatermarkPixmap,
        #        QtGui.QPixmap(':/images/watermark1.png'))

        label = QtGui.QLabel("Welcome to the SoCRocket - Configuration Wizard.\n\n"
                             "This wizard will guid you to the platform configuration. "
                             "First we will choose a template and load a configuration. "
                             "Afterwards we can alter the configuration and save it. "
                             "And finaly generate a TLM platform. ")
        label.setWordWrap(True)

        layout = QtGui.QVBoxLayout()
        layout.addWidget(label)
        self.setLayout(layout)
