import FreeCAD
import os
import Mesh
import six
import json
import configparser

if six.PY3:
    import builtins as builtin  #py3
else:  # six.PY2
    import __builtin__ as builtin #py2

__all__ = ["TCOFile","open"]

def open(filename, doc=None):
    """Shorthand for GzipFile(filename, mode, compresslevel).
    The filename argument is required; mode defaults to 'rb'
    and compresslevel defaults to 9.
    """
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
        for line in data[part]:
            if line.startswith("t"):
                vecs = data[part][line].split(",")
                vecs = [data[part]["v%04d" % int(vec)] for vec in vecs]
                tris.append([[float(point) for point in vec.split(",")] for vec in vecs])
        # read tris here
        mesh = Mesh.Mesh(tris)
        obj = doc.addObject("Mesh::Feature", part)
        obj.Mesh = mesh
    doc.recompute()

def insert(filename,doc):
    doc = FreeCAD.ActiveDocument
    open(filename, doc)