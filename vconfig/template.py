from PyQt4 import QtCore, QtGui
import os
from model import TreeModel 
from xml.parsers.expat import ExpatError

class File:
    pass

class FileGen(File):
    def generate(self, model):
        pass

class SystemCGen(FileGen):
    def generate(self, model):
        def gen(base, node):
            var = '_'.join((base,str(node.getVar()))).strip('_').strip()
            result = "#define " + var + ' ' + str(node.getValue().toString()) + '\n'
            for i in range(node.childCount()):
                result += gen(var, node.child(i))
            return result
        result = ""
        for i in range(model.rootItem.child(0).childCount()):
            result += gen('', model.rootItem.child(0).child(i))
        return "#ifndef %(file)s\n#define %(file)s\n\n%(content)s\n\n#endif // %(file)s\n" % { "file": self.name.replace('.', '_').upper(), "content" : result}
        
class Template:
    def __init__(self, base, file):
        from xml.dom.minidom import parse
        self.base = base
        try:
            self.dom = parse(file)
        except ExpatError:
            print "'%s' is not a valid template for a platform." % file
        if hasattr(self, 'dom'):
            self.root = self.dom.documentElement
            self.name = self.root.getAttribute("name")
            self.hint = self.root.getAttribute("hint")
            self.description = ""
            self.options = None
            self._generators = []
            self._files = []
            for node in self.root.childNodes:
                if node.nodeName == "description":
                  self.description += node.toxml()[14:-15]
                elif node.nodeName == "option":
                    self.options = node
                elif node.nodeName == "generator":
                    type = node.getAttribute("type")
                    if type == "systemc":
                        file = SystemCGen()
                        file.type = "systemc"
                        file.name = node.getAttribute("name")
                        self._generators.append(file)
                elif node.nodeName == "file":
                    file = File()
                    file.name = node.getAttribute("name")
                    file.type = node.getAttribute("type")
                    file.data = node.childNodes[0].data
                    self._files.append(file)

    def model(self, widget, parent=None):
        return TreeModel(self.options, widget, parent)

    def generators(self):
        return self._generators

    def files(self):
        return self._files

class TemplatePage(QtGui.QWizardPage):
    def __init__(self, parent=None):
        super(TemplatePage, self).__init__(parent)

        self.setTitle("Template")
        self.setSubTitle("Choose the template you whant to use.")
        #self.setPixmap(QtGui.QWizard.WatermarkPixmap,
        #        QtGui.QPixmap(':/images/watermark1.png'))

        self.view = QtGui.QListWidget()
        self.info = QtGui.QTextBrowser()
        self.hsplit = QtGui.QSplitter(QtCore.Qt.Horizontal)
        self.hsplit.addWidget(self.view)
        self.hsplit.addWidget(self.info)
        #self.registerField('template')
        self.template = None
        self.ready = False
        layout = QtGui.QVBoxLayout()
        layout.addWidget(self.hsplit)
        self.setLayout(layout)
        def select():
            items = self.view.selectedItems()
            if len(items)==0:
                self.template = Null
                self.ready = False
            else:
                self.template = self.templates[str(items[0].text())]
                self.info.setText(("<h1>%s</h1><br/>" % (self.template.name)) + QtCore.QVariant(self.template.description).toString())
                self.ready = True
            self.completeChanged.emit()
        self.view.itemSelectionChanged.connect(select)
    
    def isComplete(self):
        return self.ready

    def initializePage(self):
        self.view.clear()
        self.template = None
        self.ready = False
        self.completeChanged.emit()
        self.templates = {}
        for file in os.listdir('templates'):
            base, ext = os.path.splitext(file)
            if ext == '.tpa':
                self.templates[base] = Template(base, os.path.join('templates', file))
                self.view.addItem(base)

