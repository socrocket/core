from PyQt4 import QtGui

class IntroPage(QtGui.QWizardPage):
    def __init__(self, parent=None):
        super(IntroPage, self).__init__(parent)

        self.setTitle("Introduction")
        #self.setPixmap(QtGui.QWizard.WatermarkPixmap,
        #        QtGui.QPixmap(':/images/watermark1.png'))

        label = QtGui.QLabel("Welcome to the SoCRocket - Configuration Wizard.\n\n"
                             "This tool facilitates the generation of TLM system simulations from SoCRocket platform templates. "
                             "The user is asked to select a template and enter the required configuration parameters. "
                             "Configurations may be dynamically loaded, stored and modified. "
                             "TLM system simulations can be generated at the push of a button.")
        label.setWordWrap(True)

        layout = QtGui.QVBoxLayout()
        layout.addWidget(label)
        self.setLayout(layout)
