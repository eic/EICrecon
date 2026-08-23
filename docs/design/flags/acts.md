## ACTS flags

### Material map

Material map in JSon format can be loaded with **acts:MaterialMap** flag:

```yaml
acts:MaterialMap=/path/to/file/material.cbp
```

The default value for MaterialMap `calibrations/materials-map.cbor`.
When EICRecon runs, DD4Hep downloads calibrations to the current running directory
including material map to `calibrations/materials-map.cbor`.
