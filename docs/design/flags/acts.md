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

### Geometry generation

The Acts tracking geometry can be built with the Gen1 (layer-based) or Gen3 (blueprint) construction,
selected with **acts:Generation** (`1` = Gen1, the default; `3` = Gen3, requires Acts v46+;
`0` = auto-detect, which picks Gen3 when available):

```yaml
acts:Generation=3
```

The alias **acts:geometry_representation** (`gen1`, `gen3` or `auto`) overrides `acts:Generation` when set.
