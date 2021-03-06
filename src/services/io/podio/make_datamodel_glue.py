#!/usr/bin/env python
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

EDM4HEP_ROOT = os.environ.get("EDM4HEP_ROOT")

# Check if EDM4HEP_ROOT is set
if not EDM4HEP_ROOT:
    print("ERROR: EDM4HEP_ROOT env. variable is None or empty!\n"
          "       Please point EDM4HEP_ROOT to edm4hep installation root.\n"
          "       This script looks for '{EDM4HEP_ROOT}/include/edm4hep/*Collection.h'\n")
    exit(1)


collectionfiles = glob.glob(EDM4HEP_ROOT+'/include/edm4hep/*Collection.h')
header_lines = []
get_code_lines = []
put_code_lines = []
for f in collectionfiles:
    header_fname = f.split('/edm4hep')[-1]
    basename = header_fname.split('/')[-1].split('Collection.h')[0]
    header = '#include <edm4hep' + header_fname + '>'
    code1 = '    if( collection_type == "edm4hep::'+basename+'" )'
    code2 = '        {GetPODIODataT<edm4hep::'+basename+',  edm4hep::'+basename+'Collection>(collection_name.c_str(), event, store); return;}'
    header_lines += [header]
    get_code_lines += [code1, code2]

    code1 = '    if( ! fac->GetAs<edm4hep::'+basename+'>().empty() )'
    code2 = '       {return PutPODIODataT<edm4hep::'+basename+', edm4hep::'+basename+'Collection>( fac, store, include_collections, exclude_collections );}'
    put_code_lines += [code1, code2]



get_code_lines += ['    std::cerr << "Unknown collection type: " << collection_type << std::endl;']
put_code_lines += ['    return "";']

with open('datamodel_glue.h', 'w') as f:
    f.write('\n// This file automatically generated by the make_datamodel.py script\n')
    f.write('#include <JANA/JEvent.h>\n')
    f.write('#include <JANA/JFactory.h>\n')
    f.write('#include <podio/EventStore.h>\n')
    f.write('\ntemplate <class T, class C> void GetPODIODataT( const char *collection_name, std::shared_ptr <JEvent> &event, podio::EventStore &store);')
    f.write('\ntemplate <class T, class C> std::string PutPODIODataT( JFactory *fac, podio::EventStore &store, const std::set<std::string> &include_collections, const std::set<std::string> &exclude_collections);\n\n')
    f.write('\n'.join(header_lines))
    #f.write('\nusing namespace edm4hep;\n')
    f.write('\nstatic void GetPODIOData(const std::string &collection_name, const std::string &collection_type, std::shared_ptr <JEvent> &event, podio::EventStore &store){\n')
    f.write('\n'.join(get_code_lines))
    f.write('\n}\n')
    f.write('\n// Test data type held in given factory against being any of the known edm4hep data types.')
    f.write('\n// Call PutPODIODataT if match is found. (Factory must have called EnableAs for edm4hep type.)')
    f.write('\nstatic std::string PutPODIOData(JFactory *fac, podio::EventStore &store, const std::set<std::string> &include_collections, const std::set<std::string> &exclude_collections){\n')
    f.write('\n'.join(put_code_lines))
    f.write('\n}\n')
    f.close()
