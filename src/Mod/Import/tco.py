import FreeCAD
import os
import Mesh
import six
import json
import configparser
import FreeCADGui as Gui
import Import_rc
import datetime

if six.PY3:
    import builtins as builtin  #py3
else:  # six.PY2
    import __builtin__ as builtin #py2

__all__ = ["TCOFile","open"]

def open(filename, doc=None):
    split_file = os.path.split(filename)
    base = os.path.splitext(split_file[1])[0]
    json_file = os.path.join(split_file[0], base + ".json")
    json_text = builtin.open(json_file, 'rb')
    config = json.load(json_text)
    data = configparser.ConfigParser()
    data.read(filename)
    config["parts"]["base"] = ""
    for part in config["parts"]:
        tris = []
        padding = int(data[part]["KeyFormat"][1:])
        for line in data[part]:
            if line.startswith("t"):
                vecs = data[part][line].split(",")
                vecs = [data[part]["v" + vec.zfill(padding)] for vec in vecs]
                tris.append([[float(point) for point in vec.split(",")] for vec in vecs])
        mesh = Mesh.Mesh(tris)
        obj = doc.addObject("Mesh::Feature", part)
        obj.Mesh = mesh
    doc.recompute()

def insert(filename,doc):
    doc = FreeCAD.ActiveDocument
    open(filename, doc)

def export(objs,filename):
    """exporting to file folder"""

    form = TCOSaveDialog(objs, filename)
    Gui.Control.showDialog(form)

class TCOSaveDialog:
    def __init__(self, objs, filename):
        # this will create a Qt widget from our ui file
        self.form = Gui.PySideUic.loadUi(":/ui/tco_export.ui")
        self.form.objects.itemClicked.connect(self.objectsClicked)
        self.form.baseCheck.clicked.connect(self.baseClicked)
        self.objs = objs
        (self.path, self.filename) = os.path.split(filename)
        self.name = os.path.splitext(self.filename)[0]
        self.data = configparser.ConfigParser()
        self.data["general"] = {}
        self.data["general"]["date"] = datetime.datetime.now().strftime("%d.%m.%Y %H:%M:%S")
        self.data["general"]["prog"] = "FreeCAD"
        self.base = ""
        self.config = {}
        self.config["name"] = self.name
        self.config["model"] = self.filename
        self.config["tool"] = [1,1,1]
        self.config["workpiece"] = [0,0,0]
        self.config["reverse_winding"] = True
        self.config["parts"] = {}
        self.config["parts"]["x"] = {}
        self.config["parts"]["y"] = {}
        self.config["parts"]["z"] = {}
        for obj in self.objs:
            if obj.TypeId == 'Mesh::Feature':
                self.form.objects.addItems([obj.Name])
    
    def objectsClicked(self):
        if self.base == self.form.objects.currentItem().text():
            self.form.baseCheck.setChecked(True)
        else:
            self.form.baseCheck.setChecked(False)
    
    def baseClicked(self):
        if self.form.baseCheck.isChecked():
            self.base = self.form.objects.currentItem().text()
        else:
            self.base = ""
 
    def accept(self):
        if self.base == "":
            FreeCAD.Console.PrintMessage("Please select a base object")
            return
        for obj in self.objs:
            current = obj.Name
            if obj.Name == self.base:
                current = "base"
            else:
                self.config["parts"][obj.Name] = {}
            #todo: update padding to be variable
            padding = 4
            self.data[current] = {}
            self.data[current]["KeyFormat"] = "D%d" % padding
            self.data[current]["nv"] = str(obj.Mesh.CountPoints)
            self.data[current]["nt"] = str(obj.Mesh.CountFacets)
            self.data[current]["nl"] = "0"
            for point in obj.Mesh.Points:
                self.data[current]["v" + str(point.Index).zfill(padding)] = "%s,%s,%s" % (point.x, point.y, point.z)
            for facet in obj.Mesh.Facets:
                self.data[current]["t" + str(facet.Index).zfill(padding)] = "%s,%s,%s" % (facet.PointIndices[0], facet.PointIndices[1], facet.PointIndices[2])
        with builtin.open(os.path.join(self.path, self.filename), 'w') as datafile:
            self.data.write(datafile)
        with builtin.open(os.path.join(self.path, self.name + ".json"), 'w') as jsonfile:
            jsonfile.write(json.dumps(self.config, indent=4))
        Gui.Control.closeDialog()