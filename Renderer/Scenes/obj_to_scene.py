# Script to convert Wavefront OBJ's exported by Blender into the "Composer" ASCII format
# Blender export settings:
#  Forward: -Z Forward
#  Up: Y Up
#  Write Normals
#  Write Materials
#  Objects as OBJ Objects

import sys
import string
import json

if len(sys.argv) < 2:
    sys.stderr.write('Usage: python {} <scene>\n'.format(sys.argv[0]))
    sys.stderr.write('Reads <scene>.obj, <scene>.head, and <scene>.extra to produce <scene>.ascii\n')
    sys.stderr.write('<scene>.head should contain the desired header of the scene file\n')
    sys.stderr.write("<scene>.extra contains extra properties (e.g. emission) not defineable in the OBJ format\n")
    exit(1)

class Material:
    def __init__(self, name):
        self.name = name
        self.diffuse = None
        self.ambient = None
        self.specular = None
        self.emissive = None
        self.shininess = None

class Object:
    def __init__(self, name):
        self.name = name
        self.mtl = None

        # every three positions/normals define the vertices of a triangle
        self.positions = [] 
        self.normals = []

def parse_materials(filename):
    res = {}
    with open(filename) as mtl:
        lines = mtl.read().split('\n')
        mtl = None
        for line in lines:
            line = string.strip(line)
            if len(line) == 0:
                continue
            parts = line.split(' ')
            cmd = parts[0]
            if cmd[0] == '#':
                pass
            elif cmd == 'newmtl':
                if mtl is not None:
                    res[mtl.name] = mtl
                mtl = Material(parts[1])
            elif cmd == 'Ns':
                mtl.shininess = float(parts[1])
            elif cmd == 'Ka':
                mtl.ambient = float(parts[1]), float(parts[2]), float(parts[3])
            elif cmd == 'Kd':
                mtl.diffuse = float(parts[1]), float(parts[2]), float(parts[3])
            elif cmd == 'Ks':
                mtl.specular = float(parts[1]), float(parts[2]), float(parts[3])
            elif cmd == 'Ke':
                mtl.emissive = float(parts[1]), float(parts[2]), float(parts[3])
            elif cmd == 'Ni':
                pass
            elif cmd == 'd':
                pass
            elif cmd == 'illum':
                pass
            else:
                raise Exception('Unrecognized MTL command `{}`'.format(cmd))
        if mtl is not None:
            res[mtl.name] = mtl
    return res

with open('{}.obj'.format(sys.argv[1]), 'r') as obj, open('{}.ascii'.format(sys.argv[1]), 'w') as out:
    extra = None
    with open('{}.extra'.format(sys.argv[1]), 'r') as extra:
        extra = json.loads(extra.read())
    with open('{}.head'.format(sys.argv[1]), 'r') as head:
        out.write(head.read())
    lines = obj.read().split('\n')
    materials = None
    positions = []
    normals = []
    objects = {}
    current_object = None
    for line in lines:
        line = string.strip(line)
        if len(line) == 0:
            continue
        parts = line.split(' ')
        cmd = parts[0]
        if cmd[0] == '#':
            pass
        elif cmd == 'mtllib':
            assert materials is None
            materials = parse_materials(parts[1])
        elif cmd == 'o':
            if current_object is not None:
                objects[current_object.name] = current_object
            current_object = Object(parts[1])
        elif cmd == 'v':
            positions.append((float(parts[1]), float(parts[2]), float(parts[3])))
        elif cmd == 'vn':
            normals.append((float(parts[1]), float(parts[2]), float(parts[3])))
        elif cmd == 'usemtl':
            current_object.mtl = materials[parts[1]]
        elif cmd == 's':
            pass
        elif cmd == 'f':
            indices = []
            for part in parts[1:]:
                pn = part.split('//')
                indices.append((int(pn[0]), int(pn[1])))
            if len(indices) != 4:
                sys.stderr.write('Warning: Ignoring triangular face (only quads supported)\n')
            else:
                tri_indices = []
                tri_indices.append(indices[0])
                tri_indices.append(indices[1])
                tri_indices.append(indices[2])
                tri_indices.append(indices[0])
                tri_indices.append(indices[2])
                tri_indices.append(indices[3])
                for p, n in tri_indices:
                    current_object.positions.append(positions[p-1])
                    current_object.normals.append(normals[n-1])
        else:
            raise Exception('Unrecognized OBJ command `{}`'.format(cmd))
    if current_object is not None:
        objects[current_object.name] = current_object
    for name, extras in extra.items():
        obj = objects[name]
        if 'emisColor' in extras:
            emis = extras['emisColor']
            obj.mtl.emissive = emis[0], emis[1], emis[2]
    for name, obj in objects.items():
        out.write('poly_set {\n')
        out.write('  name "{}"\n'.format(name))
        out.write('  numMaterials 1\n')
        out.write('  material {\n')
        out.write('    diffColor {} {} {}\n'.format(obj.mtl.diffuse[0], obj.mtl.diffuse[1], obj.mtl.diffuse[2]))
        out.write('    ambColor 0.2 0.2 0.2\n') # ignore the ambient term offered by Blender 
        out.write('    specColor {} {} {}\n'.format(obj.mtl.specular[0], obj.mtl.specular[1], obj.mtl.specular[2]))
        out.write('    emisColor {} {} {}\n'.format(obj.mtl.emissive[0], obj.mtl.emissive[1], obj.mtl.emissive[2]))
        out.write('    shininess {}\n'.format(obj.mtl.shininess/32.0))
        out.write('    ktran 0\n')
        out.write('  }\n')
        out.write('  type POLYSET_TRI_MESH\n')
        out.write('  normType PER_VERTEX_NORMAL\n')
        out.write('  materialBinding PER_OBJECT_MATERIAL\n')
        out.write('  hasTextureCoords FALSE\n')
        out.write('  rowSize 0\n')
        assert len(obj.positions) == len(obj.normals)
        out.write('  numPolys {}\n'.format(len(obj.positions)/3))
        for i in xrange(len(obj.positions)/3):
            out.write('  poly {\n')
            out.write('    numVertices 3\n')
            for j in xrange(3):
                pos = obj.positions[i*3 + j]
                norm = obj.normals[i*3 + j]
                out.write('    pos {} {} {}\n'.format(pos[0], pos[1], pos[2]))
                out.write('    norm {} {} {}\n'.format(norm[0], norm[1], norm[2]))
            out.write('  }\n')
        out.write('}\n')
