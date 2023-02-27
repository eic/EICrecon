RICH Geometry Service
=====================

A common place for bindings between RICH geometry forms:
- `DD4hep`: simulation geometry
- `ACTS`:   track-projection planes
- `IRT`:    optical surfaces for Indirect Ray Tracing

`RichGeo_service` provides a JANA service for these bindings. The
JANA-independent code is contained in `richgeo/`, which can either be built as
an `EICrecon` plugin (using `richgeo/CMakeLists.txt`) or built as a standalone
library for external usage (using your own build configuration).
