from __future__ import print_function
from __future__ import absolute_import
from builtins import str
from builtins import range
from builtins import object
import os
from .model import TreeModel 
from xml.parsers.expat import ExpatError
import shutil
from string import Template as sTemplate

class File(object):
    pass

class FileGen(File):
    def generate(self, model):
        pass

class TemplateGen(FileGen):
    def generate(self, model, base = "", conf = ""):
        def gen(base, node):
            result = {}
            var = '_'.join((base,str(node.getVar()))).strip('_').strip()
            result[var] = str(node.getValue().toString())
            for i in range(node.childCount()):
                result.update(gen(var, node.child(i)))
            return result
        result = {}
        result.update(gen('', model.rootItem.child(0)))
        result.update({'template':base, 'configuration':conf})
        return sTemplate(self.data.strip()+'\n').safe_substitute(result)
 
class SystemCGen(FileGen):
    def generate(self, model, base = "", conf = ""):
        def gen(base, node):
            var = '_'.join((base,str(node.getVar()))).strip('_').strip()
            result = "#define " + var + ' ' + str(node.getValue().toString()) + '\n'
            for i in range(node.childCount()):
                result += gen(var, node.child(i))
            return result
        result = ""
        #for i in range(model.rootItem.child(0).childCount()):
        result += gen('', model.rootItem.child(0))
        return "#ifndef %(file)s\n#define %(file)s\n\n%(content)s\n\n#endif // %(file)s\n" % { "file": self.name.replace('.', '_').upper(), "content" : result}
        
class Template(object):
    def __init__(self, base, file):
        from xml.dom.minidom import parse
        self.base = base
        self.progress = 0
        self.configuration = 'unknown'
        self._model = None
        self.dirname = os.path.dirname(file)
        try:
            self.dom = parse(file)
        except ExpatError:
            print("'%s' is not a valid template for a platform." % file)
        if hasattr(self, 'dom'):
            self.root = self.dom.documentElement
            self.name = self.root.getAttribute("name")
            self.hint = self.root.getAttribute("hint")
            self.description = ""
            self.instructions = ""
            self.options = None
            self._generators = []
            self._files = []
            for node in self.root.childNodes:
                if node.nodeName == "description":
                  self.description += node.toxml()[14:-15]
                if node.nodeName == "instructions":
                  self.instructions += node.toxml()[14:-15]
                elif node.nodeName == "option":
                    self.options = node
                elif node.nodeName == "generator":
                    type = node.getAttribute("type")
                    if type == "systemc":
                        file = SystemCGen()
                        file.type = "systemc"
                        if node.hasAttribute("path"):
                            file.path = node.getAttribute("path")
                        else:
                            file.path = 'systemc'
                        file.name = node.getAttribute("name")
                        self._generators.append(file)
                    if type == "template":
                        file = TemplateGen()
                        file.type = "template"
                        if node.hasAttribute("path"):
                            file.path = node.getAttribute("path")
                        file.name = node.getAttribute("name")
                        if node.hasAttribute("src"):
                            src = node.getAttribute("src")
                            if not os.path.isabs(src):
                                src = os.path.normpath(os.path.join(self.dirname, src))
                            src = open(src, "r")
                            file.data = ''.join(src.readlines())
                        else:
                            file.data = ""
                            for child in node.childNodes:
                              file.data += child.data
                        self._generators.append(file)
                elif node.nodeName == "file":
                    file = File()
                    file.name = node.getAttribute("name")
                    if node.hasAttribute("type"):
                        file.type = node.getAttribute("type")
                    else:
                        file.type = None
                    if node.hasAttribute("src"):
                        src = node.getAttribute("src")
                        if not os.path.isabs(src):
                            src = os.path.normpath(os.path.join(self.dirname, src))
                        src = open(src, "r")
                        file.data = ''.join(src.readlines())
                    else:
                        file.data = ""
                        for child in node.childNodes:
                            file.data += child.data
                    
                    self._files.append(file)

    def getModel(self, widget=None, parent=None):
        if self._model == None:
            self._model = TreeModel(self.options, widget, parent)
            if self.configuration != 'unknown':
                self.loadConfiguration(self.configuration)
        return self._model

    def generators(self):
        return self._generators

    def files(self):
        return self._files

    def getConfiguration(self):
        return self.configuration
      
    def getConfigurationFile(self):
        return os.path.join("templates", '.'.join((str(self.base), str(self.configuration))))+".json"

    def listConfigurations(self):
        result = []
        for file in os.listdir('templates'):
            lst = file.split('.')
            if len(lst) == 3:
                tmpl, name, ext = file.split('.')
                if tmpl == self.base and ext == 'json':
                  result.append(name)
        return result
   
    def loadConfiguration(self, name):
        self.configuration = name
        file = self.getConfigurationFile()
        if self._model != None:
            self._model.loadFromJsonFile(file)

    def storeConfiguration(self, name):
        self.configuration = name
        file = self.getConfigurationFile()
        if self._model != None:
            self._model.saveToJsonFile(file)
        
    def generate(self, progress = None):
        if progress == None:
            def p(val):
              pass
            progress = p
        # gather informations
        gen = self.generators()
        files = self.files()
        steps = 2 + len(files) + len(gen)
        progress(1 * 100 / steps)
        # mkdir
        path = os.path.join("platforms", '-'.join((str(self.base), str(self.configuration)))) 
        if os.path.isdir(path):
            shutil.rmtree(path)
        if any(files + gen):
            os.makedirs(path)
        progress(2 * 100 / steps)
        # writing files
        num = 2
        for f in files:
            if f.type == None:
                fdir = path
                ffile = os.path.join(path, f.name)
            else:
                fdir = os.path.join(path, f.type)
                ffile = os.path.join(path, f.type, f.name)
            if not os.path.isdir(fdir):
                os.makedirs(fdir)
            file = open(ffile, "w")
            file.write(sTemplate(f.data.strip()).safe_substitute({'template':self.base, 'configuration':self.configuration}))
            #file.write(f.data)
            num += 1
            progress(num * 100 / steps)
        
        # generating files
        for f in gen:
            if f.path == None:
                fdir = path
                ffile = os.path.join(path, f.name)
            else:
                fdir = os.path.join(path, f.path)
                ffile = os.path.join(path, f.path, f.name)
            if not os.path.isdir(fdir):
                os.makedirs(fdir)
            file = open(ffile, "w")
            file.write(f.generate(self.getModel(), self.base, self.configuration))
            num += 1
            progress(num * 100 / steps)
 
class TemplateCollection(dict):
    def __init__(self):
        super(TemplateCollection, self).__init__()
        self._template = None
        for file in os.listdir('templates'):
            base, ext = os.path.splitext(file)
            if ext == '.tpa':
                self[base] = Template(base, os.path.join('templates', file))
                
    def getModel(self, parent):
        return self._template.getModel(parent)
        
    def setTemplate(self, value):
        if isinstance(value, Template):
          self._template = value
        else:
          if value in self:
            self._template = self[value]

    def getTemplate(self):
        return self._template
      
    def getConfiguration(self):
        return self._template.getConfiguration()
    
    def listConfigurations(self):
        return self._template.listConfigurations()
   
    def loadConfiguration(self, name):
        return self._template.loadConfiguration(name)

    def storeConfiguration(self, name):
        return self._template.storeConfiguration(name)
        
    def generate(self):
        return self._template.generate()
