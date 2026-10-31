# Run metadata in output files

EICrecon records the configuration of each invocation in the `runs` category of
its podio output file, so that the output can be reproduced without access to
the original command line or log.

## Layout of the `runs` category

EICrecon first copies every `runs` frame from its input file(s) unchanged (for
example the ddsim frame with the simulation steering parameters), and then
**appends** one frame of its own. Entry 0 therefore stays the frame from the
first input file, as before.

Because merged or chained files can hold any number of `runs` frames, select
frames by their `producer` parameter rather than by index.

| Key | Type | Content |
|---|---|---|
| `producer` | string | Always `EICrecon` |
| `processingStage` | int | 1 + the highest `processingStage` of the EICrecon frames in the input (1 when reconstructing simulation) |
| `eicrecon:version` | string | `git describe` version of the EICrecon build |
| `eicrecon:janaVersion` | string | JANA2 version |
| `eicrecon:podioVersion` | string | podio version |
| `eicrecon:dateUTC` | string | Time the frame was written (ISO 8601, UTC) |
| `eicrecon:inputFiles` | string vector | Resource names of all event sources |
| `eicrecon:unusedParameters` | string vector | Parameters that were set but never used (often typos) |
| `eicrecon:param:<key>` | string vector | `{value, default}` for every JANA parameter, defaults included |

In C++, `frame.getParameter<std::string>("eicrecon:param:<key>")` returns just
the value (the first element).

No `runNumber` is written, since JANA run numbers are per event. If the input
`runs` frames contain collections, the EICrecon frame is not written (all frames
in a category must hold the same collections) and a warning is logged.

## Reading the configuration

```python
from podio.reading import get_reader

reader = get_reader("output.edm4eic.root")
for frame in reader.get("runs"):
    if "producer" not in frame.parameters or frame.get_parameter("producer") != "EICrecon":
        continue
    prefix = "eicrecon:param:"
    for key in sorted(frame.parameters):
        if key.startswith(prefix):
            value, default = (v.decode() if isinstance(v, bytes) else v
                              for v in frame.get_parameter(key))
            line = f"{key[len(prefix):]} {value}"
            # JANA's config reader hangs on lines over 8190 characters (e.g. the
            # default podio:output_collections) and reads empty values as "1"
            if len(line) > 8190:
                line = f"# {key[len(prefix):]} omitted: line too long ({len(line)} characters)"
            elif value == "":
                line = f"# {key[len(prefix):]} omitted: empty value"
            print(line)
```

The output of this loop has the `key value` format accepted by
`eicrecon --loadconfigs <file>`, with every parameter including defaults.
Two kinds of parameters are written as comments instead, because JANA's config
file reader cannot load them: lines longer than 8190 characters (the reader
hangs) and empty values (the reader turns them into `1`). If such a value
differs from its default, set it on the command line with `-P`.
