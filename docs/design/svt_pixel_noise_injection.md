# RandomNoisePixel: silicon noise injection explained

## 1. What problem does this algorithm solve?

A silicon tracker contains many electronic pixels. Even when no charged particle crosses a pixel,
electronics noise can occasionally make that pixel appear to fire. `RandomNoisePixel` adds these
noise-only hits to an EICrecon event.

The model starts from one probability:

```text
p = noise_rate_per_pixel_per_event
```

The default is `p = 2e-7` per pixel per event. In other words, a particular pixel has a probability
of approximately two in ten million of producing a noise hit in one event. The same `p` is used
throughout the SVT. Layers with more pixels have more expected noise hits.

The current implementation supports:

| Detector system | Segmentation coordinates | Sensor treatment |
| --- | --- | --- |
| BVTX | cylindrical phi-z | rectangular phi and z index ranges |
| BTRK | Cartesian x-y | rectangular x and y index ranges |
| ECTRK | Cartesian x-z | row-by-row trapezoid ranges |

The implementation is in
[`RandomNoisePixel.cc`](../EICrecon/src/algorithms/digi/RandomNoisePixel.cc), with data structures in
[`RandomNoisePixel.h`](../EICrecon/src/algorithms/digi/RandomNoisePixel.h) and user parameters in
[`RandomNoisePixelConfig.h`](../EICrecon/src/algorithms/digi/RandomNoisePixelConfig.h).

## 2. The statistical model

Let:

- `p` be the noise probability per pixel per event;
- `N_m` be the number of addressable pixels on sensitive component `m`;
- `N_l = sum_m N_m` be the total number of pixels in layer `l`.

The expected number of noise hits in a layer is

```text
lambda_l = p * N_l.
```

The event-by-event hit count is drawn from

```text
K_l ~ Poisson(lambda_l).
```

The exact independent-pixel model would be `Binomial(N_l, p)`. The Poisson model is an excellent
approximation because `p` is very small and `N_l` is very large. It also avoids a random trial for
every pixel.

### Worked rate example

Suppose one layer has two sensitive components:

```text
component A: N_A = 1,000,000 pixels
component B: N_B = 3,000,000 pixels
```

For `p = 2e-7`,

```text
N_l      = 4,000,000 pixels
lambda_l = 2e-7 * 4,000,000 = 0.8 noise hits/event.
```

The event may contain zero, one, two, or more noise hits; `0.8` is the long-run average. Component A
is selected with probability `1/4`, and component B with probability `3/4`. Every individual pixel
therefore has the same probability of being selected.

## 3. The two phases

`RandomNoisePixel` separates geometry work from event work:

```text
DD4hep geometry
      |
      v
init(): discover sensors and build compact pixel layouts
      |
      v
cached IDs, pixel ranges, and layer totals
      |
      v
process(): draw noise counts and create hits for each event
```

Geometry navigation is relatively expensive and involves shared TGeo state. It is done once in
`init()`. Event processing uses cached integer information and does not navigate TGeo.

## 4. Initialization: understand the detector once

### Step 1: validate the configuration

If `addNoise` is false, initialization stops without building a cache. Otherwise, the configured
rate must satisfy `0 <= p <= 1`, and the requested DD4hep readout must exist. Each factory instance
uses one readout, such as `VertexBarrelHits`, `SiBarrelHits`, or `TrackerEndcapHits`.

### Step 2: find sensitive silicon components

The code traverses the DD4hep detector hierarchy and keeps only sensitive placements whose readout
matches the factory configuration. A module may itself be sensitive, or it may contain sensitive
daughter volumes. The traversal handles both cases.

For every sensitive placement, initialization temporarily records:

- detector name and layer number;
- its `TGeoVolume` and shape;
- its local-to-world `TGeoHMatrix` transformation.

The transform identifies the placed sensor and validates the final cell position. It is discarded
after initialization, so it consumes no persistent event-time cache memory.

The transform includes two distinct steps:

```text
sensitive-solid local point
  -> add/rotate by the component placement inside its module
  -> apply the module and all ancestor placements to reach world coordinates
```

This distinction matters when, for example, left and right silicon pieces are offset from the module
center. The code composes the matrices once; it neither drops the component offset nor adds it a
second time.

### Step 3: determine the sensor's base volume ID

A cell ID contains two kinds of information:

```text
placement fields: detector, layer, module, sensor, ...
pixel fields:     x/y, x/z, or phi/z indices
```

The code chooses a point inside the sensor, transforms it from local to global coordinates, and asks
`CellIDPositionConverter` for its complete cell ID. The segmentation's `volumeID()` operation
removes the local pixel fields, leaving `baseVolumeID`.

During event processing, the algorithm starts from this base ID and writes new pixel indices into
it. Packed numerical cell IDs are never assumed to form one continuous integer interval.

### Step 4: read the actual segmentation

Pixel pitch and offset come directly from DD4hep. The algorithm has no separate 20 micrometre pitch
parameter. If the segmentation changes, the cached pixel count changes automatically. The supplied
per-pixel noise rate is not silently rescaled.

For a `MultiSegmentation`, the placement fields select the correct concrete sub-segmentation before
the pixel layout is calculated.

## 5. Compact representations of valid pixels

The algorithm never stores a cell ID for every physical pixel. Instead, it stores index ranges.

### Rectangular Cartesian sensor

For a rectangle, all pixel centers inside these limits are valid:

```text
firstMin  <= firstIndex  <= firstMax
secondMin <= secondIndex <= secondMax
```

The pixel count is

```text
N = (firstMax - firstMin + 1) * (secondMax - secondMin + 1).
```

For example, x indices `2...5` and y indices `10...12` describe `4 * 3 = 12` pixels. Only the four
limits and total count are stored.

### Trapezoidal sensor

An ECTRK trapezoid does not fill its rectangular bounding box. Its valid x range changes with z.
The code examines each candidate z row and checks pixel centers with `TGeoShape::Contains()`.

For each non-empty row it stores:

```text
z index    minimum x    maximum x    cumulative pixel total
-------    ---------    ---------    ----------------------
   10         -1            1                  3
   11         -2            2                  8
   12         -3            3                 15
```

The left and right boundaries are found with binary searches. This works for the current convex,
centered trapezoids, where each row contains one continuous x interval.

To select a pixel, draw one flat index from `0...14`. The cumulative totals identify its row. Flat
index 6 belongs to the second row because `3 <= 6 < 8`. Its offset is `6 - 3 = 3`, giving

```text
x = xMin + 3 = -2 + 3 = 1
z = 11
```

Every valid pixel has equal probability even though rows have different widths. Empty rows and
invalid bounding-box corners are never sampled.

### Cylindrical phi-z sensor

For `CylindricalGridPhiZ`, phi and z are coordinates in the sensitive volume's local cylindrical
frame. The code obtains local bounds directly from the sensor shape and converts them into discrete
indices. DD4hep applies the component's placement transform later when converting the cell ID into a
global hit position; applying that transform while building the indices would place the sensor
twice.

Phi requires care near the `-pi`/`+pi` boundary. Angular differences are measured around the
sensor's central phi so that a sensor crossing this boundary is not mistaken for one spanning almost
the entire circle.

### Pixel-center convention

A pixel is counted when its segmentation-defined center is inside the sensitive TGeo shape.
Partial geometrical overlap at a sensor edge is not treated as a fractional pixel. This deterministic
convention treats pixels as electronic channels.

Initialization checks representative pixels at the beginning, quarters, middle, and end of every
placed component's address range. Each cell ID is converted by DD4hep to a global center and then
transformed back into that exact sensitive solid. The point must be inside the TGeo shape. A point
on a face is accepted only within ROOT's geometry tolerance, which avoids false failures from
floating-point round trips while still detecting incorrect offsets or rotations.

## 6. Sharing layouts between repeated sensors

Many placed sensors have the same logical TGeo volume and Cartesian segmentation. Their pixel
ranges are identical even though their placement IDs differ. These components share one immutable
`PixelLayout` through a `shared_ptr`.

Each placed component retains only:

- detector name and layer;
- `baseVolumeID`;
- a shared pixel-layout pointer;
- its pixel count.

The full transform and TGeo volume pointer are initialization-only data. The cache therefore scales
mainly with the number of components plus rows in unique trapezoid layouts, not with the total number
of pixels.

## 7. Building layer selection tables

Components are grouped by detector name and layer number. Each layer stores component indices and
cumulative pixel totals. For component sizes

```text
N_0 = 100, N_1 = 300, N_2 = 200,
```

the cumulative table is `[100, 400, 600]`. A layer-wide random number in `0...599` selects:

- component 0 for `0...99`;
- component 1 for `100...399`;
- component 2 for `400...599`.

`std::upper_bound` finds the component in logarithmic time. The selection probability is
automatically proportional to component pixel count.

## 8. Processing one event

### Step 1: create a reproducible random-number generator

The run and event identity come from the required `EventHeader`. `UniqueIDGenSvc` combines this
identity with the algorithm name to make a deterministic seed. There is no silent fallback seed.

### Step 2: draw one count per layer

For each layer:

```text
mean = p * layer.totalPixels
requestedHits ~ Poisson(mean).
```

This is much cheaper than visiting every sensor or pixel in every event.

### Step 3: select a component and pixel

For each requested hit:

1. Draw a uniform layer-wide pixel number.
2. Use the cumulative table to select a component with probability `N_m/N_l`.
3. Draw a uniform pixel index from that component's compact layout.
4. Convert the flat pixel index into two segmentation indices.
5. Copy `baseVolumeID` and set those fields with the DD4hep bit-field coder.

The first draw chooses a correctly weighted component. The second chooses a uniform pixel inside
that component. Together they give uniform sampling over all layer pixels.

### Step 4: reject duplicate pixels

Hits are stored in a map keyed by cell ID. If the same pixel is selected twice in one event, the
second selection is retried. At the default occupancy, duplicates are extremely rare.

Retries are bounded so an unexpectedly high occupancy cannot create an infinite loop. A warning is
emitted if the requested number of unique hits cannot be produced.

### Step 5: create `RawTrackerHit` objects

Each accepted pixel becomes an `edm4eic::RawTrackerHit`. The current charge and timestamp are

```text
charge    = 1.0e6
timestamp = 0
```

The map is iterated in increasing cell-ID order, giving deterministic output ordering.

## 9. Computational scaling

Let `M` be the number of sensitive components, `R` the number of cached rows across unique
non-rectangular layouts, `L` the number of layer groups, and `K` the generated noise-hit count.

| Operation | Scaling |
| --- | --- |
| Geometry traversal during initialization | `O(M)` |
| Rectangular layout construction | `O(1)` per unique layout |
| Trapezoid layout construction | `O(R)` plus boundary searches |
| Persistent cache | `O(M + R)` |
| Event count draws | `O(L)` |
| Event hit generation | `O(K log M_layer)` |

There is no array proportional to the total number of pixels and no per-event loop over every
component. This is why the method remains practical for 100,000 or more sensitive components.

## 10. Assumptions and limitations

1. Detector geometry and alignment remain static after `init()`.
2. Unsupported segmentation types raise an error instead of using a silent area approximation.
3. The row algorithm assumes a convex, centered shape with at most one valid interval per row. A
   non-convex sensor or one with holes would require multiple spans per row.
4. Pixel centers define active channels at boundaries.
5. The algorithm models occupancy, not pulse-height or timing distributions.
6. The Poisson approximation is intended for sparse occupancies such as `2e-7`.

## 11. Function map for reading the source

Read the implementation in this order:

| Function or structure | Purpose |
| --- | --- |
| `PixelLayout`, `PixelRow` | Represent valid segmentation indices compactly |
| `init()` | Top-level cache construction |
| `collectDetectorComponents()` | Enter detector layers |
| `collectLayerComponents()` | Find modules and sensitive descendants |
| `cachePixelLayouts()` | Determine base IDs and create layouts |
| `makeCartesianLayout()` | Build rectangular or row-span Cartesian layouts |
| `makeCylindricalLayout()` | Build local cylindrical phi-z layouts |
| `buildLayers()` | Construct pixel-weighted component tables |
| `pixelIndices()` | Map a flat pixel number to two segmentation indices |
| `randomCellID()` | Encode a sampled pixel into a complete cell ID |
| `addNoiseHitsForLayer()` | Draw the layer count and create unique hits |
| `process()` | Event entry point and deterministic output |

## 12. Checks a student can perform

1. Verify that `sum(component.pixelCount)` equals `layer.totalPixels`.
2. For a rectangle, compare the cache with the product of the two index counts.
3. For a trapezoid, plot `firstMin` and `firstMax` versus row and compare them with sensor edges.
4. Decode generated IDs and check their detector, layer, component, and pixel fields.
5. Over many events, confirm that the mean hit count approaches `p * N_l`.
6. Confirm that component hit fractions approach `N_m/N_l`.
7. Repeat the same event and configuration and confirm identical cell IDs.

These tests separately check geometry interpretation, statistical normalization, and
reproducibility: the three main correctness requirements of the algorithm.
