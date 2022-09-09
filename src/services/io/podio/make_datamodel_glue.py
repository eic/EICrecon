#!/usr/bin/env python3
#
# Copyright 2022, David Lawrence
# Subject to the terms in the LICENSE file found in the top-level directory.
#
#

#
#  This is a stop gap and not intended for long term.  2022-07-09  DL
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
        if '=' in arg: WORKING_DIR = arg.split('=')[1]
    if arg.startswith('EDM4HEP_INCLUDE_DIR'):
        if '=' in arg: EDM4HEP_INCLUDE_DIR = arg.split('=')[1]
    if arg.startswith('EDM4EIC_INCLUDE_DIR'):
        if '=' in arg: EDM4EIC_INCLUDE_DIR = arg.split('=')[1]

# Check if EDM4HEP_ROOT is set
if not EDM4HEP_INCLUDE_DIR:
    print("ERROR: EDM4HEP_INCLUDE_DIR not spcified on command line (with \n"
          "EDM4HEP_INCLUDE_DIR=/path/to/edm4hep/include) and EDM4HEP_ROOT\n"
          "env. variable is None or empty\n"
          "       Please specify the EDM4HEP_INCLUDE_DIR value explicitly\n"
          "       or point EDM4HEP_ROOT envar to edm4hep installation root.\n"
          "       This script looks for '{EDM4HEP_INCLUDE_DIR}/edm4hep/*Collection.h'\n"
          "                          or '{EDM4HEP_ROOT}/include/edm4hep/*Collection.h'\n")
    sys.exit(1)

# Check if EDM4EIC_ROOT is set
if not EDM4EIC_INCLUDE_DIR:
    print("ERROR: EDM4EIC_INCLUDE_DIR not spcified on command line (with \n"
          "EDM4EIC_INCLUDE_DIR=/path/to/EDM4EIC/include) and EDM4EIC_ROOT\n"
          "env. variable is None or empty\n"
          "       Please specify the EDM4EIC_INCLUDE_DIR value explicitly\n"
          "       or point EDM4EIC_ROOT envar to EDM4EIC installation root.\n"
          "       This script looks for '{EDM4EIC_INCLUDE_DIR}/EDM4EIC/*Collection.h'\n"
          "                          or '{EDM4EIC_ROOT}/include/EDM4EIC/*Collection.h'\n")
    sys.exit(1)


def AddCollections(datamodelName, collectionfiles):
    for f in collectionfiles:
        header_fname = f.split('/'+datamodelName)[-1]
        basename = header_fname.split('/')[-1].split('Collection.h')[0]

        header = '#include <'+ datamodelName + header_fname + '>'
        header_lines.append(header)

        make_lines.append('    if( className == "vector<'+datamodelName+'::'+basename+'Data>"){ return new EICEventStore::DataVectorT<'+datamodelName+'::'+basename+'Data>(name, className, collectionID); }')

        copy_lines.append( '    if( dv->className == "vector<'+datamodelName+'::'+basename+'Data>") {' )
        copy_lines.append( '        auto *dvt = reinterpret_cast<EICEventStore::DataVectorT<'+datamodelName+'::'+basename+'Data>*>( dv );' )
        copy_lines.append( '        return CopyToJEventT<'+datamodelName+'::'+basename+', '+datamodelName+'::'+basename+'Obj, '+datamodelName+'::'+basename+'Data, '+datamodelName+'::'+basename+'Collection>(dvt, event, podio_objs);' )
        copy_lines.append( '    }')

        copy_simple_lines.append( '    if( className == "'+datamodelName+'::'+basename+'Collection") {' )
        copy_simple_lines.append( '        auto *collection_typed = reinterpret_cast<const '+datamodelName+'::'+basename+'Collection*>( collection );' )
        copy_simple_lines.append( '        return CopyToJEventSimpleT<'+datamodelName+'::'+basename+', '+datamodelName+'::'+basename+'Collection>(collection_typed, name, event);' )
        copy_simple_lines.append( '    }')

        put_lines.append('    if( ! fac->GetAs<'+datamodelName+'::'+basename+'>().empty() )')
        put_lines.append('       {return PutPODIODataT<'+datamodelName+'::'+basename+', '+datamodelName+'::'+basename+'Collection>( writer, fac, store );}')

        put_simple_lines.append('    if( ! fac->GetAs<'+datamodelName+'::'+basename+'>().empty() )')
        put_simple_lines.append('       {return writer->PutPODIODataT<'+datamodelName+'::'+basename+', '+datamodelName+'::'+basename+'Collection>( fac, store );}')


collectionfiles_edm4hep = glob.glob(EDM4HEP_INCLUDE_DIR+'/edm4hep/*Collection.h')
collectionfiles_edm4eic    = glob.glob(EDM4EIC_INCLUDE_DIR+'/edm4eic/*Collection.h')
header_lines      = []
copy_lines        = []
copy_simple_lines = []
make_lines        = []
put_lines         = []
put_simple_lines  = []
AddCollections('edm4hep', collectionfiles_edm4hep)
AddCollections('edm4eic'   , collectionfiles_edm4eic   )


# for f in collectionfiles:
#     header_fname = f.split('/edm4hep')[-1]
#     basename = header_fname.split('/')[-1].split('Collection.h')[0]
#
#     header = '#include <edm4hep' + header_fname + '>'
#     header_lines.append(header)
#
#     make_lines.append('    if( className == "vector<edm4hep::'+basename+'Data>"){ return new EICEventStore::DataVectorT<edm4hep::'+basename+'Data>(name, className, collectionID); }')
#
#     copy_lines.append( '    if( dv->className == "vector<edm4hep::'+basename+'Data>") {' )
#     copy_lines.append( '        auto *dvt = reinterpret_cast<EICEventStore::DataVectorT<edm4hep::'+basename+'Data>*>( dv );' )
#     copy_lines.append( '        return CopyToJEventT<edm4hep::'+basename+', edm4hep::'+basename+'Obj, edm4hep::'+basename+'Data, edm4hep::'+basename+'Collection>(dvt, event, podio_objs);' )
#     copy_lines.append( '    }')
#
#     copy_simple_lines.append( '    if( className == "edm4hep::'+basename+'Collection") {' )
#     copy_simple_lines.append( '        auto *collection_typed = reinterpret_cast<const edm4hep::'+basename+'Collection*>( collection );' )
#     copy_simple_lines.append( '        return CopyToJEventSimpleT<edm4hep::'+basename+', edm4hep::'+basename+'Collection>(collection_typed, name, event);' )
#     copy_simple_lines.append( '    }')
#
#     put_lines.append('    if( ! fac->GetAs<edm4hep::'+basename+'>().empty() )')
#     put_lines.append('       {return PutPODIODataT<edm4hep::'+basename+', edm4hep::'+basename+'Collection>( writer, fac, store );}')
#
#     put_simple_lines.append('    if( ! fac->GetAs<edm4hep::'+basename+'>().empty() )')
#     put_simple_lines.append('       {return writer->PutPODIODataT<edm4hep::'+basename+', edm4hep::'+basename+'Collection>( fac, store );}')

make_lines.append('    if( className == "vector<podio::ObjectID>"){ return new EICEventStore::DataVectorT<podio::ObjectID>(name, className); }')
make_lines.append('    std::cerr << "Unknown classname: " << className << " for branch " << name << std::endl;')
make_lines.append('    return nullptr;')
copy_lines.append('    std::cerr << "Unknown classname: " << dv->className << std::endl;')
copy_simple_lines.append('    std::cerr << "Unknown classname: " << className << std::endl;')
put_lines.append('    return "";')
put_simple_lines.append('    return "";')

if WORKING_DIR : os.chdir( WORKING_DIR )

with open('datamodel_includes.h', 'w') as f:
    f.write('\n// This file automatically generated by the make_datamodel.py script\n\n')
    f.write('\n'.join(header_lines))
    f.write('\n')
    f.close()

with open('datamodel_glue.h', 'w') as f:
    f.write('\n// This file automatically generated by the make_datamodel.py script\n')
    f.write('#include <JANA/JEvent.h>\n')
    f.write('#include <JANA/JFactory.h>\n')
    f.write('#include <podio/CollectionIDTable.h>\n')
    f.write('#include <services/io/podio/EICEventStore.h>\n')
    f.write('#include <services/io/podio/EICRootWriter.h>\n')
    f.write('#include <services/io/podio/EICRootWriterSimple.h>\n')
    f.write('#include <services/io/podio/datamodel_includes.h>\n')
    f.write('\n')
    f.write('\ntemplate <typename T, typename Tobj, typename Tdata, typename Tcollection> void CopyToJEventT(EICEventStore::DataVectorT<Tdata> *dvt, std::shared_ptr <JEvent> &jevent, std::vector<podio::ObjBase*> &podio_objs);')
    f.write('\ntemplate <typename T, typename Tcollection> void CopyToJEventSimpleT(const Tcollection *collection, const std::string &name, std::shared_ptr <JEvent> &jevent);')
    f.write('\ntemplate <class T, class C> std::string PutPODIODataT( EICRootWriter *writer, JFactory *fac, EICEventStore &store );\n')

    f.write('\nstatic EICEventStore::DataVector* MakeDataVector(const std::string &name, const std::string &className, int collectionID=-1){\n')
    f.write('\n'.join(make_lines))
    f.write('\n}\n')

    f.write('\n')
    f.write('\nstatic void CopyToJEvent(EICEventStore::DataVector *dv, std::shared_ptr<JEvent> &event, std::vector<podio::ObjBase*> &podio_objs){\n')
    f.write('\n'.join(copy_lines))
    f.write('\n}\n')

    f.write('\n')
    f.write('\nstatic void CopyToJEventSimple(const std::string &className, const std::string &name, const podio::CollectionBase *collection, std::shared_ptr<JEvent> &event){\n')
    f.write('\n'.join(copy_simple_lines))
    f.write('\n}\n')

    f.write('\n// Test data type held in given factory against being any of the known edm4hep or edm4eic data types.')
    f.write('\n// Call PutPODIODataT if match is found. (Factory must have called EnableAs for edm4hep type.)')
    f.write('\nstatic std::string PutPODIOData(EICRootWriter *writer, JFactory *fac, EICEventStore &store){\n')
    f.write('\n'.join(put_lines))
    f.write('\n}\n')

    f.write('\n// Test data type held in given factory against being any of the known edm4hep or edm4eic data types.')
    f.write('\n// Call PutPODIODataT if match is found. (Factory must have called EnableAs for the edm4hep or edm4eic type.)')
    f.write('\nstatic std::string PutPODIODataSimple(EICRootWriterSimple *writer, JFactory *fac, podio::EventStore &store){\n')
    f.write('\n'.join(put_simple_lines))
    f.write('\n}\n')
    f.close()
