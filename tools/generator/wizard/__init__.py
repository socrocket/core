from __future__ import print_function
from __future__ import absolute_import
from PyQt4 import QtGui
from .intro import IntroPage
from .license import LicensePage
from .template import TemplatePage
from .load import LoadPage
from .config import ConfigPage
from .save import SavePage
from .generate import GeneratePage
from ..template import TemplateCollection

class Wizard(QtGui.QWizard):
  def __init__(self, templates, parent=None):
    super(Wizard, self).__init__(parent)
    #self.setButtonLayout([QtGui.QWizard.Stretch, QtGui.QWizard.NextButton, QtGui.QWizard.FinishButton, QtGui.QWizard.CancelButton])
    #self.setStyleSheet("QWizard { background: blue }; QWizardPage { background: yellow;}")
    self.templates = templates
    self.addPage(IntroPage())
    #self.addPage(LicensePage())
    tmpl = TemplatePage(self.templates)
    self.addPage(tmpl)
    load = LoadPage(self.templates)
    self.addPage(load)
    conf = ConfigPage(self.templates)
    self.addPage(conf)
    save = SavePage(self.templates)
    self.addPage(save)
    gen  = GeneratePage(self.templates)
    self.addPage(gen)

def main(template, configuration):
    templates = TemplateCollection()
    if template:
      if template in list(templates.keys()):
        templates.setTemplate(template)
        if configuration:
          if configuration in templates.listConfigurations():
            templates.loadConfiguration(configuration)
          else:
            print("Configuration %s for template %s not found" % (configuration, template))
        templates.generate()
      else: 
        print("Template %s not found" % template)
    else:
      app = QtGui.QApplication([])
      wizard = Wizard(templates)
      wizard.setWindowTitle("SoCRocket - Configuration Wizard")
      wizard.show()

      import sys
      sys.exit(wizard.exec_())
      QtGui.app = None

