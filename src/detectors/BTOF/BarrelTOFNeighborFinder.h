
#ifndef BARRELTOFNEIGHBORFINDER_H
#define BARRELTOFNEIGHBORFINDER_H

#include <DD4hep/Detector.h>
#include <DDRec/CellIDPositionConverter.h>
#include <DDRec/Surface.h>
#include <JANA/JEventProcessorSequentialRoot.h>
#include <algorithms/geo.h>
#include <services/log/Log_service.h>

#include "TGeoMatrix.h"
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

class BarrelTOFNeighborFinder {
public:
  void init(const dd4hep::Detector* detector);
  void useCache(bool value = true) { _useCache = value; };
  void setLogger(const std::shared_ptr<spdlog::logger>& log) { _log = log; };
  bool isDeadCell(const dd4hep::rec::CellID& hitCell);
  const std::shared_ptr<std::vector<dd4hep::rec::CellID>> 
       findAllNeighborInSensor(const dd4hep::rec::CellID& hitCell);

  // functions below are helper function. Isn't needed to find neighbors.
  dd4hep::Position    cell2GlobalPosition(const dd4hep::rec::CellID& cell);
  dd4hep::Position    cell2LocalPosition(const dd4hep::rec::CellID& cell);
  dd4hep::Position    global2Local(const dd4hep::Position& pos);
  dd4hep::Position    local2GlobalInStaveFromCell(const dd4hep::rec::CellID& cell,
                                                  const dd4hep::Position& pos);
  dd4hep::rec::CellID globalPosition2Cell(const dd4hep::Position& pos);
  std::vector<double> cellDimension(const dd4hep::rec::CellID& cell);

private:
  void _findAllNeighborsInSensor(dd4hep::rec::CellID hitCell,
                                 std::shared_ptr<std::vector<dd4hep::rec::CellID>>& ans,
                                 std::unordered_set<dd4hep::rec::CellID>& dp);
  int _getSensorID(const dd4hep::rec::CellID& hitCell);
  // need to initialize the class with a cell from Barrel TOF
  void _initWithCell(const dd4hep::rec::CellID& hitCell);



  bool _useCache                                        = true;
  const dd4hep::DDSegmentation::BitFieldCoder* _decoder = nullptr;
  const dd4hep::Detector* _detector                     = nullptr;

  std::unique_ptr<dd4hep::rec::CellIDPositionConverter>                _converter;
  std::shared_ptr<spdlog::logger>                                      _log;
  std::unordered_map<dd4hep::rec::CellID, 
                    std::shared_ptr<std::vector<dd4hep::rec::CellID>>> _cache;

};

#endif
