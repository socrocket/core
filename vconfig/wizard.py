from PyQt4 import QtGui
from intro import IntroPage
from license import LicensePage
from template import TemplatePage
from load import LoadPage
from config import ConfigPage
from save import SavePage
from generate import GeneratePage

class Wizard(QtGui.QWizard):
  def __init__(self, parent=None):
    super(Wizard, self).__init__(parent)
    #self.setStyleSheet("* { background: blue }")
    self.addPage(IntroPage())
    self.addPage(LicensePage())
    tmpl = TemplatePage()
    self.addPage(tmpl)
    load = LoadPage(tmpl)
    self.addPage(load)
    conf = ConfigPage(tmpl, load)
    self.addPage(conf)
    save = SavePage(tmpl, load, conf)
    self.addPage(save)
    gen  = GeneratePage(tmpl, save, conf)
    self.addPage(gen)
    self.addPage(IntroPage())

def main():
    import sys
    app = QtGui.QApplication([])

    wizard = Wizard()
    wizard.setWindowTitle("SoCRocket - Configuration Wizard")
    wizard.show()

    sys.exit(wizard.exec_())

if __name__ == '__main__':
    main
  
