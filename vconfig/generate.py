from PyQt4 import QtCore, QtGui
import os
import shutil

class GeneratePage(QtGui.QWizardPage):
    def __init__(self, template, name, conf, parent=None):
        super(GeneratePage, self).__init__(parent)

        self.setTitle("Generate Platform")
        self.setSubTitle("Please wait while the wizard is crafting your platform...")
        #self.setPixmap(QtGui.QWizard.WatermarkPixmap,
        #        QtGui.QPixmap(':/images/watermark1.png'))

        self.label1 = QtGui.QLabel("Progress ...")
        self.progress = QtGui.QProgressBar()
        self.label2 = QtGui.QLabel("Details:")
        self.list = QtGui.QListWidget()
        self.template = template
        self.name = name
        self.conf = conf
        layout = QtGui.QVBoxLayout()
        layout.addWidget(self.label1)
        layout.addWidget(self.progress)
        layout.addWidget(self.label2)
        layout.addWidget(self.list)
        self.setLayout(layout)
        self.ready = False
        self.progress.setValue(0)
    
    def isComplete(self):
        return self.ready

    def initializePage(self):
        self.list.clear()
        self.ready = False
        self.progress.setValue(0)
        # gather informations
        gen = self.template.template.generators()
        files = self.template.template.files()
        steps = 2 + len(files) + len(gen)
        self.progress.setValue(1 * 100 / steps)
        # mkdir
        path = os.path.join("platforms", '-'.join((str(self.template.template.base), str(self.name.conf.conf)))) 
        if os.path.isdir(path):
            shutil.rmtree(path)
        os.makedirs(path)
        self.progress.setValue(2 * 100 / steps)
        # writing files
        num = 2
        for f in files:
            fpath = os.path.join(path, f.type, f.name)
            if not os.path.isdir(os.path.join(path, f.type)):
                os.makedirs(os.path.join(path, f.type))
            file = open(fpath, "w")
            file.write(f.data)
            num += 1
            self.progress.setValue(num * 100 / steps)
        
        # generating files
        for f in gen:
            fpath = os.path.join(path, f.type, f.name)
            if not os.path.isdir(os.path.join(path, f.type)):
                os.makedirs(os.path.join(path, f.type))
            file = open(fpath, "w")
            file.write(f.generate(self.conf.model))
            num += 1
            self.progress.setValue(num * 100 / steps)
        
        self.ready = True
        self.completeChanged.emit()
