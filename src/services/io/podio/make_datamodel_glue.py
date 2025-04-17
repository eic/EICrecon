#!/usr/bin/env python3
#
# Copyright 2022, David Lawrence
# Subject to the terms in the LICENSE file found in the top-level directory.
#
# This is a stop gap and not intended for long term.  2022-07-09  DL
#
# This will scan the list of files in the EDM4hep datamodel directory
# (pointed to by the EDM4HEP_ROOT environment variable). Using the
# filenames, it will generate some C++ code that can be used by
# the JEventSourcePODIO and EDM4hepWriter classes to read and write all
# of those types.

import os
import sys
import glob

print('Generating datamodel_glue.h ...')

# Default to "not found"
WORKING_DIR = None
EDM4HEP_INCLUDE_DIR = None
EDM4EIC_INCLUDE_DIR = None

# Try getting from environment first so we can overwrite
# with command line below if available.
EDM4HEP_ROOT = os.environ.get("EDM4HEP_ROOT")
if EDM4HEP_ROOT :
    EDM4HEP_INCLUDE_DIR=EDM4HEP_ROOT+'/include'

EDM4EIC_ROOT = os.environ.get("EDM4EIC_ROOT")
if EDM4EIC_ROOT :
    EDM4EIC_INCLUDE_DIR=EDM4EIC_ROOT+'/include'


# poor man's command line parsing
for arg in sys.argv:
    if arg.startswith('WORKING_DIR'):
        if '=' in arg: WORKING_DIR = arg.split('=',1)[1]
    if arg.startswith('EDM4HEP_INCLUDE_DIR'):
        if '=' in arg: EDM4HEP_INCLUDE_DIR = arg.split('=',1)[1]
    if arg.startswith('EDM4EIC_INCLUDE_DIR'):
        if '=' in arg: EDM4EIC_INCLUDE_DIR = arg.split('=',1)[1]

# Check if EDM4HEP_ROOT is set
if not EDM4HEP_INCLUDE_DIR:
    print("ERROR: EDM4HEP_INCLUDE_DIR not specified on command line (with \n"
          "EDM4HEP_INCLUDE_DIR=/path/to/edm4hep/include) and EDM4HEP_ROOT\n"
          "env. variable is None or empty\n"
          "       Please specify the EDM4HEP_INCLUDE_DIR value explicitly\n"
          "       or point EDM4HEP_ROOT envar to edm4hep installation root.\n"
          "       This script looks for '{EDM4HEP_INCLUDE_DIR}/edm4hep/*Collection.h'\n"
          "                          or '{EDM4HEP_ROOT}/include/edm4hep/*Collection.h'\n")
    sys.exit(1)

# Check if EDM4EIC_ROOT is set
if not EDM4EIC_INCLUDE_DIR:
    print("ERROR: EDM4EIC_INCLUDE_DIR not specified on command line (with \n"
          "EDM4EIC_INCLUDE_DIR=/path/to/EDM4EIC/include) and EDM4EIC_ROOT\n"
          "env. variable is None or empty\n"
          "       Please specify the EDM4EIC_INCLUDE_DIR value explicitly\n"
          "       or point EDM4EIC_ROOT envar to EDM4EIC installation root.\n"
          "       This script looks for '{EDM4EIC_INCLUDE_DIR}/EDM4EIC/*Collection.h'\n"
          "                          or '{EDM4EIC_ROOT}/include/EDM4EIC/*Collection.h'\n")
    sys.exit(1)


def AddCollections(datamodelName, collectionfiles):
    for f in collectionfiles:
        # require at least a class definition, not a using alias
        with open(f, 'r') as file:
            if not "class" in file.read():
                continue

        header_fname = f.split('/'+datamodelName)[-1]
        basename = header_fname.split('/')[-1].split('Collection.h')[0]

        header = '#include <'+ datamodelName + header_fname + '>'
        header_lines.append(header)

        type_map.append('namespace ' + datamodelName + ' {')
        type_map.append('    class ' + basename + ';')
        type_map.append('    class ' + basename + 'Collection;')
        type_map.append('    class Mutable' + basename + ';')
        type_map.append('};')

        visitor.append('        if (podio_typename == "' + datamodelName + '::' + basename + 'Collection") {')
        visitor.append('            return visitor(*reinterpret_cast<const ' + datamodelName + '::' + basename + 'Collection*>(&collection));')
        visitor.append('        }')


collectionfiles_edm4hep = glob.glob(EDM4HEP_INCLUDE_DIR+'/edm4hep/*Collection.h')
collectionfiles_edm4eic = glob.glob(EDM4EIC_INCLUDE_DIR+'/edm4eic/*Collection.h')
header_lines = []
type_map = []
visitor = []
AddCollections('edm4hep', collectionfiles_edm4hep)
AddCollections('edm4eic', collectionfiles_edm4eic)


if WORKING_DIR:
    os.makedirs(WORKING_DIR, exist_ok=True)
    os.chdir(WORKING_DIR)

with open('datamodel_includes.h', 'w') as f:
    f.write('\n// This file automatically generated by the make_datamodel.py script\n\n')
    f.write('\n// IWYU pragma: begin_exports\n')
    f.write('\n'.join(header_lines))
    f.write('\n// IWYU pragma: end_exports\n')
    f.write('\n')
    f.close()

with open('datamodel_glue.h', 'w') as f:
    f.write('\n// This file automatically generated by the make_datamodel.py script\n')
    f.write('#pragma once\n')
    f.write('\n')
    f.write('#include <stdexcept>\n')
    f.write('#include <podio/CollectionBase.h>\n')
    f.write('\n\n')
    f.write('\n'.join(type_map))
    f.write('\n')
    f.write('\ntemplate <typename Visitor> struct VisitPodioCollection {')
    f.write('\n    void operator()(Visitor& visitor, const podio::CollectionBase& collection) {')
    f.write('\n        auto podio_typename = collection.getTypeName();\n')
    f.write('\n'.join(visitor))
    f.write('\n        throw std::runtime_error("Unrecognized podio typename!");')
    f.write('\n    }')
    f.write('\n};\n')
    f.close()
