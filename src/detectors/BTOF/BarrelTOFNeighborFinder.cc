
//
// Template for this file generated with eicmkplugin.py
//

#include "BarrelTOFNeighborFinder.h"
#include "services/geometry/richgeo/RichGeo.h"

// Include appropriate class headers. e.g.
#include "TAxis.h"
#include "TCanvas.h"
#include "TGeoManager.h"

#include <algorithm>
#include <iostream>

void BarrelTOFNeighborFinder::init(const dd4hep::Detector* detector) {
  // you need to init the class before calling any other methods
  // What about RAII?
  _detector  = detector;
  _converter = std::make_unique<dd4hep::rec::CellIDPositionConverter>(*detector);
}

// return cell bin (defined internally within DetPosProcessor) list for all neighbors in a sensor
void BarrelTOFNeighborFinder::_findAllNeighborsInSensor(
    dd4hep::rec::CellID hitCell, std::shared_ptr<std::vector<dd4hep::rec::CellID>>& ans,
    std::unordered_set<dd4hep::rec::CellID>& dp) {
  // use MST to find all neighbor within a sensor
  // I can probably write down the formula by hand, but why do things manually when computer do
  // everything for you?
  const std::vector<std::pair<int, int>> searchDirs{{0, 1}, {0, -1}, {1, 0}, {-1, 0}};
  ans->push_back(hitCell);
  dp.insert(hitCell);

  auto sensorID = _getSensorID(hitCell);
  auto xID = _decoder -> get(hitCell, "x");
  auto yID = _decoder -> get(hitCell, "y");
  for (const auto& dir : searchDirs) {
    auto testCell = hitCell;
    try {
      _decoder -> set(testCell, "x", xID + dir.first);
      _decoder -> set(testCell, "y", yID + dir.second);
    } catch (const std::runtime_error& err) {
      // catch overflow error
      // ignore if invalid position ID
      continue;
    }

    try {
      _converter -> position(testCell);
    } catch (const std::invalid_argument& err) {
      // Ignore CellID that is invalid
      continue;
    }

    // only look for cells that have not been searched
    if (dp.find(testCell) == dp.end()) {
      auto testSensorID = _getSensorID(testCell);
      if (testSensorID == sensorID) {
        // inside the same sensor
        this->_findAllNeighborsInSensor(testCell, ans, dp);
      }
    }
  }
}

void BarrelTOFNeighborFinder::_initWithCell(const dd4hep::rec::CellID& hitCell) {
  if (!_decoder) {
    // cell dimensions, cellID converter and bit decoder will be fetched on first use
    if (_log)
      _log->info("Retrieving cellID decoder.");
    // retrieve segmentation class
    const auto det = _converter->findContext(hitCell)->element;
    auto readout   = _converter->findReadout(det);
    auto seg       = readout.segmentation();
    auto type      = seg.type();
    if (type != "CartesianGridXY")
      throw std::runtime_error("Unsupported segmentation type: " + type +
                               ". BarrelTOF must use CartesianGridXY.");
    // retrieve meaning of cellID bits
    _decoder = seg.decoder();
  }
}


const std::shared_ptr<std::vector<dd4hep::rec::CellID>>
BarrelTOFNeighborFinder::findAllNeighborInSensor(const dd4hep::rec::CellID& hitCell) {
  _initWithCell(hitCell);
  // look for cache. If exist, return cache value
  if (_useCache) {
    auto it = _cache.find(hitCell);
    if (it != _cache.end())
      return it->second;
  }

  std::unordered_set<dd4hep::rec::CellID> dp;
  // fill cache
  std::shared_ptr<std::vector<dd4hep::rec::CellID>> neighbors =
      std::make_shared<std::vector<dd4hep::rec::CellID>>();
  this->_findAllNeighborsInSensor(hitCell, neighbors, dp);

  if (_useCache)
    for (auto cell : *neighbors)
      _cache[cell] = neighbors;

  return neighbors;
}

int BarrelTOFNeighborFinder::_getSensorID(const dd4hep::rec::CellID& hitCell) {
  _initWithCell(hitCell);
  // when x or y-index goes out of bound, sometimes the position will corresponds to a new sensor
  // will fetch the new cellID here
  auto newID = _converter->cellID(_converter->position(hitCell)); 
  return _decoder->get(newID, "sensor"); 
}

/****************************************
 * Functions below aren't needed anymore to find neighbors
 * They are still handy so I keep them
 * *************************************/
dd4hep::Position BarrelTOFNeighborFinder::cell2GlobalPosition(const dd4hep::rec::CellID& cell) {
  if (!_converter)
    throw std::runtime_error(
        "CellIDPositionConverter not initialized in BarrelTOFNeighborFinder. Have you called "
        "BarrelTOFNeighborFinder::init(/* detector */) first?");
  return _converter->position(cell);
}

dd4hep::rec::CellID BarrelTOFNeighborFinder::globalPosition2Cell(const dd4hep::Position& pos) {
  if (!_converter)
    throw std::runtime_error(
        "CellIDPositionConverter not initialized in BarrelTOFNeighborFinder. Have you called "
        "BarrelTOFNeighborFinder::init(/* detector */) first?");
  return _converter->cellID(pos);
}

dd4hep::Position
BarrelTOFNeighborFinder::local2GlobalInStaveFromCell(const dd4hep::rec::CellID& cell,
                                                     const dd4hep::Position& pos) {
  // convert local position to global position
  // assuming the local position is located at the same volume as cell
  if (!_detector)
    throw std::runtime_error("Detector ptr not initialized in BarrelTOFNeighborFinder. Have you "
                             "called BarrelTOFNeighborFinder::init(/* detector */) first?");

  auto geoManager = _detector->world().volume()->GetGeoManager();
  auto position   = this->cell2GlobalPosition(cell);
  auto node       = geoManager->FindNode(position.x(), position.y(), position.z());
  auto currMatrix = geoManager->GetCurrentMatrix();

  double g[3], l[3];
  pos.GetCoordinates(l);
  currMatrix->LocalToMaster(l, g); // <- only different from cell2LocalPosition
  position.SetCoordinates(g);      // <- only different from cell2LocalPosition
  return position;
}

dd4hep::Position
BarrelTOFNeighborFinder::global2Local(const dd4hep::Position& pos) {
  // convert local position to global position
  // assuming the local position is located at the same volume as cell
  if (!_detector)
    throw std::runtime_error("Detector ptr not initialized in BarrelTOFNeighborFinder. Have you "
                             "called BarrelTOFNeighborFinder::init(/* detector */) first?");
  auto geoManager = _detector->world().volume()->GetGeoManager();
  auto node       = geoManager->FindNode(pos.x(), pos.y(), pos.z());
  auto currMatrix = geoManager->GetCurrentMatrix();

  double g[3], l[3];
  pos.GetCoordinates(g);
  currMatrix->MasterToLocal(g, l); 
  dd4hep::Position position;
  position.SetCoordinates(l);      
  return position;
}



dd4hep::Position BarrelTOFNeighborFinder::cell2LocalPosition(const dd4hep::rec::CellID& cell) {
  if (!_detector)
    throw std::runtime_error("Detector ptr not initialized in BarrelTOFNeighborFinder. Have you "
                             "called BarrelTOFNeighborFinder::init(/* detector */) first?");

  auto geoManager = _detector->world().volume()->GetGeoManager();
  auto position   = this->cell2GlobalPosition(cell);
  auto node       = geoManager->FindNode(position.x(), position.y(), position.z());
  auto currMatrix = geoManager->GetCurrentMatrix();

  double g[3], l[3];
  position.GetCoordinates(g);
  currMatrix->MasterToLocal(g, l);
  position.SetCoordinates(l);
  return position;
}

std::vector<double> BarrelTOFNeighborFinder::cellDimension(const dd4hep::rec::CellID& hitCell) {
  if(!_converter) _initWithCell(hitCell);
  return _converter -> cellDimensions(hitCell);
}
