RICH Geometry Service
=====================

A common place for bindings between RICH geometry forms:
- `DD4hep`: simulation geometry
- `ACTS`:   track-projection planes
- `IRT`:    optical surfaces for Indirect Ray Tracing

`RichGeo_service` provides a JANA service for these bindings, with `richgeo.cc`
to define the plugin. All other source files are meant to be JANA-independent,
and can either be built with this `richgeo` plugin or as a standalone library
for external usage (using your own build configuration). The standalone
capability is currently used for legacy Juggler support.
