# EICrecon common flags



## Geometry: 


#### Environment variables:

- **DETECTOR_PATH** - path where DD4Hep detector
- **DETECTOR_CONFIG** - name of xml file with DD4Hep detector description without '.xml' extension

If both variables are set, EICrecon by default tries to open:

```bash
${DETECTOR_PATH}/${DETECTOR_CONFIG}.xml
```

`dd4hep:xml_files` (see below), overwrites the default.  


#### Flags:

**dd4hep:xml_files** - Comma separated list of XML files describing the DD4hep geometry.
**dd4hep:print_level** - Set DD4hep print level (see DD4hep/Printout.h)

If xml_files are given and DETECTOR_PATH is set, then EICrecon first tries to open xml_file[i] if it fails, it tries
`${DETECTOR_PATH}/file`

```bash

eicrecon ... -Pdd4hep:xml_files=/full/path/epic.xml   # good
eicrecon ... -Pdd4hep:xml_files=epic.xml              # good if $DETECTOR_PATH is set /full/path/
eicrecon ... -Pdd4hep:xml_files=epic                  # fail, contrary to DETECTOR_CONFIG, this should be with extension
```
