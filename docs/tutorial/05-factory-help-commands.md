# EICrecon Factory Help Commands

This document describes the new factory introspection commands added to the `eicrecon` CLI.

## New Commands

### `--list-available-factories <plugin>`

Lists all factories provided by a specific plugin.

**Usage:**
```bash
eicrecon --list-available-factories EEMC
eicrecon --list-available-factories BEMC
```

**Output:**
- Shows all factories provided by the specified plugin
- Displays factory object names and tags in a table format
- If plugin not found, shows list of available plugins with factories
- Includes a summary count of factories provided by the plugin

**Example:**
```
$ eicrecon --list-available-factories EEMC

Factories provided by plugin 'EEMC':

| Object name                  | Tag                                      | Description                                                    |
|------------------------------|------------------------------------------|---------------------------------------------------------------|
| CalorimeterHitDigi          | EcalEndcapNRawHits                      | Produces: CalorimeterHitDigi (tag: EcalEndcapNRawHits)       |
| CalorimeterHitReco          | EcalEndcapNRecHits                      | Produces: CalorimeterHitReco (tag: EcalEndcapNRecHits)       |
| CalorimeterClusterRecoCoG   | EcalEndcapNTruthClustersWithoutShapes   | Produces: CalorimeterClusterRecoCoG (tag: EcalEndcapN...)    |

Summary: Plugin 'EEMC' provides 15 factories.
```

### `--print-factory-info`

Shows detailed information about all factories in the system.

**Usage:**
```bash
eicrecon --print-factory-info
```

**Output:**
- Complete table of all factories with plugin, object name, tag, and type info
- Factory summary statistics including total count
- Breakdown of factories by plugin
- Collection naming pattern analysis (heuristic)
- Guidance on how to get more detailed input/output information

**Example:**
```
$ eicrecon --print-factory-info

Detailed factory information:

| Plugin | Object name              | Tag                        | Type Info                           |
|--------|--------------------------|----------------------------|-------------------------------------|
| EEMC   | CalorimeterHitDigi      | EcalEndcapNRawHits        | CalorimeterHitDigi [EcalEndcapN...] |
| BEMC   | CalorimeterHitDigi      | EcalBarrelRawHits         | CalorimeterHitDigi [EcalBarrel...]  |
| ...    | ...                      | ...                        | ...                                 |

Factory Summary:
  Total factories: 157
  Factories by plugin:
    EEMC: 15 factories
    BEMC: 12 factories
    tracking: 8 factories
    ...

  Collection naming patterns (heuristic analysis):
    Ecal*: 45 collections (EcalEndcapNRawHits, EcalBarrelRawHits, EcalEndcapNRecHits, ...)
    Hcal*: 23 collections (HcalEndcapNRawHits, HcalBarrelRawHits, ...)
    ...

For detailed input/output collection information:
  1. Use --list-available-factories <plugin> to see factories by plugin
  2. Inspect factory source code for precise input/output definitions
  3. Look at plugin registration code (e.g., EEMC.cc) for I/O specifications
```

## Integration with Existing Commands

These new commands work alongside existing EICrecon CLI commands:

- `--list-available-plugins` - Lists all available plugins
- `--list-factories` (`-L`) - Lists all factories (basic format)
- `--list-default-plugins` - Lists default plugins

## Technical Notes

### Factory Input/Output Collections

While these commands provide comprehensive factory listings, detailed input/output collection information requires examination of:

1. **Factory source code**: Look at the `PodioInput<>` and `PodioOutput<>` declarations in factory headers
2. **Plugin registration**: Check the plugin's `.cc` file (e.g., `EEMC.cc`) for `JOmniFactoryGeneratorT` calls that specify input and output collections
3. **Factory documentation**: Some factories may have additional documentation in their source files

### Example Factory Registration

In plugin registration code, you'll find patterns like:
```cpp
app->Add(new JOmniFactoryGeneratorT<CalorimeterHitDigi_factory>(
    "EcalEndcapNRawHits",                          // Factory tag
    {"EventHeader", "EcalEndcapNHits"},            // Input collections
    {"EcalEndcapNRawHits", "EcalEndcapNRawHitAssociations"}, // Output collections
    config_object
));
```

This provides the precise input/output relationships that are not currently accessible through the runtime CLI introspection.

## Future Enhancements

These commands provide a foundation for factory introspection. Potential future enhancements could include:

- Runtime access to input/output collection information through extended JANA APIs
- Factory dependency graph visualization
- Collection type information
- Factory parameter inspection
- Performance metrics integration
