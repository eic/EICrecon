## ACTS flags


### Logging

The `acts:LogLevel` sets log level for most operations. Currently, some may require source code changes to update the verbosity.

### Material map

Material map in JSON format can be loaded with **acts:MaterialMap** flag:

```yaml
acts:MaterialMap=/path/to/file/material.cbor
```

If `acts:MaterialMap` is not specified, the material map path is determined in the following order:
1. DD4hep constant `material-map` (if defined in the detector XML)
2. Default path: `calibrations/materials-map.cbor`

When EICRecon runs, DD4hep downloads calibrations to the current running directory,
including the material map to `calibrations/materials-map.cbor`.

To explicitly disable material map loading, set an empty path:
```yaml
acts:MaterialMap=""
```
