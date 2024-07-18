// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Chun Yuen Tsang
//
// Find cells that are in the same sensor in BTOF
//
// Author: Chun Yuen Tsang
// Date: 18/07/2024


#ifndef BARRELTOFNEIGHBORFINDER_H
#define BARRELTOFNEIGHBORFINDER_H

#include <JANA/JEventProcessorSequentialRoot.h>
#include <services/log/Log_service.h>
#include <DDRec/CellIDPositionConverter.h>
#include <DDRec/Surface.h>
#include <DD4hep/Detector.h>

#include "TGeoMatrix.h"
#include <utility>
#include <memory>
#include <vector>

class BarrelTOFNeighborFinder{
private:

    int _cellNX = 0, _cellNY = 0;
    int _sensorNX = 0, _sensorNY = 0;
    TGeoHMatrix *_currMatrix = nullptr;
    double _staveXMin = 1e5, _staveYMin = 1e5;
    double _staveXMax = -1e5, _staveYMax = -1e5;
    double _sensorWidth, _sensorLength;
    double _cellWidth, _cellLength;
    std::shared_ptr<spdlog::logger> _log;

    double _binCenter(int bin, double minRange,  double binSize);
    double _binLowEdge(int bin, double minRange, double binSize);
    int _findBin(double value, double minRange,  double binSize);
    void _findAllNeighborsInSensor(int cellBinX, int cellBinY, 
                                   int sensorBinX, int sensorBinY, 
                                   std::vector<std::pair<int, int>>& ans, 
                                   std::vector<std::vector<bool>>& dp);

    void _searchNOBins(const dd4hep::rec::CellID& cellID, bool length, bool width); // seed hit point to scan number of cells each side

    std::unique_ptr<dd4hep::rec::CellIDPositionConverter> _converter;
    const dd4hep::Detector *_detector = nullptr;

public:
    BarrelTOFNeighborFinder(int cellNX, int cellNY,
                            double sensorWidth, double sensorLength);



    void init(const dd4hep::Detector* detector);

    void                                     setLogger(const std::shared_ptr<spdlog::logger>& log);
    std::vector<dd4hep::rec::CellID>         findAllNeighborInSensor(const dd4hep::rec::CellID& hitCell);
    dd4hep::Position                         cell2GlobalPosition(const dd4hep::rec::CellID& cell);
    dd4hep::Position                         cell2LocalPosition(const dd4hep::rec::CellID& cell);
    dd4hep::rec::CellID                      globalPosition2Cell(const dd4hep::Position& pos);
    int counter = 0;
};

#endif
