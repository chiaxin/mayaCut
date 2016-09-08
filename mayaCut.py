# -*- coding: utf-8 -*-
import sys
import argparse
from os.path import isfile, abspath
import maya.standalone
import maya.cmds as mc
import maya.OpenMaya as om
maya.standalone.initialize()

VERSION = "0.0.1"

def file_is_exists(file):
    if not isfile(file):
        msg = '{0} is not exists!'.format(file)
        raise argparse.ArgumentError(msg)
    return file

def parse_arg(command_line):
    parse = argparse.ArgumentParser(
        description='Clean up maya file script ver '+VERSION)
    parse.add_argument('source', type=file_is_exists,
       help='The source maya file')
    parse.add_argument('destination',
        help='Destination maya file')
    parse.add_argument('-f', '--freeze', action='store_true',
        help='Freeze all transform in scene')
    parse.add_argument('-i', '--instance', action='store_true',
        help='Convert all instance to individual')
    parse.add_argument('-c', '--copyTexture', action='store_true',
        help='Copy textures and re-direct them')
    parse.add_argument('-d', '--deleteIntermediate', action='store_true',
        help='Delete all intermediate object')
    parse.add_argument('-z', '--vertexZero', action='store_true',
        help='Set all mesh\'s vertex to zero')
    return parse.parse_args(command_line)
#
def freezeAll():
    # without instance polygon
    freeze_transforms = []
    all_transforms = mc.ls(l=True, tr=True)
    all_polygons = mc.filterExpand(all_transforms, sm=12, fp=True)
    all_instanced_mesh = getAllInstance()
    all_non_instance_polys = tuple(set(all_polygons) - set(all_instanced_mesh))
    for poly in all_non_instance_polys:
        freeze_transforms.extend(mc.listRelatives(poly, parent=True, f=True))
    mc.makeIdentity(freeze_transforms, apply=True, t=True, r=True, s=True)
    print '### Below objects have been freezed ###'
    print '\n'.join(freeze_transforms)

def convertInstance():
    print 'Convert instance!!'

def copyTexture():
    print 'Copy texture!!'

def deleteAllIntermediateObject():
    # Delete all objects it is intermediate object
    all_intermediateObjects = mc.ls(l=True, type='mesh', io=True)
    if all_intermediateObjects:
        mc.delete(all_intermediateObjects)
        print '### Below object has been deleted ###'
        print '\n'.join(all_intermediateObjects)
    else:
        print '# No any mesh is intermediate object'

def setAllVertexToZero():
    print 'Set all vertex to zero!!'

# Helper function
def getAllInstance():
    instanceList = []
    iter = om.MItDag(om.MItDag.kBreadthFirst)
    while not iter.isDone():
        is_instanced = om.MItDag.isInstanced(iter)
        if is_instanced:
            instanceList.append(iter.fullPathName())
        iter.next()
    return instanceList

def main():
    try:
        parser = parse_arg(sys.argv[1:])
        source = parser.source
        destination = parser.destination
    except Exception as e:
        sys.stderr.write('Make simple process was failed')
        raise
    # Open file
    mc.file(source, force=True, open=True)
    # Process
    if parser.freeze: freezeAll()
    if parser.instance: convertInstance()
    if parser.copyTexture: copyTexture()
    if parser.deleteIntermediate: deleteAllIntermediateObject()
    if parser.vertexZero: setAllVertexToZero()
    #
    mc.file(rename=destination)
    mc.file(save=True, force=True, type='mayaAscii', uiConfiguration=False)
    mc.quit(f=True)
    print '# --- successiful! --- #'
    return 0

if __name__=='__main__':
    main()