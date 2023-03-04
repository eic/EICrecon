# EICrecon common flags and environment

This page does not provide all available run flags/parameters for EICrecon.
There are many more parameters that can be used in algorithms.
Moreover dynamically attached plugins add their own parameters.
Here is a list of common flags/parameters and environment variables to
control the core flow of EICrecon execution.

## EICrecon

EICrecon follows JANA2 principles of using flags. Components of reconstruction software
can declare parameters, which users can control with `-P` flag.

```bash
eicrecon -Pplugins=JTest  -PMyPlugin:MyVariable=5 input_file1.root input_file2.root
```

We can understand this command as follows:

* `eicrecon` is the default command-line tool for launching reconstruction.

* The `-P` flag specifies a configuration parameters, e.g.

* All free parameters are considered to be input file, e.g. `input_file1.root`, `input_file2.root`


[jana2](flags/jana2.md ':include')

[dd4hep](flags/dd4hep.md ':include')

[acts](flags/acts.md ':include')
