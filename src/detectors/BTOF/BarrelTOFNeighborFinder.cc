// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Chun Yuen Tsang
//
// Find cells that are in the same sensor in BTOF
//
// Author: Chun Yuen Tsang
// Date: 18/07/2024

#include "BarrelTOFNeighborFinder.h"

// Include appropriate class headers. e.g.
#include "TGeoManager.h"
#include "TAxis.h"
#include "TCanvas.h"

#include <iostream>
#include <algorithm>

//-------------------------------------------
// InitWithGlobalRootLock
//-------------------------------------------
BarrelTOFNeighborFinder::BarrelTOFNeighborFinder(int cellNX, int cellNY,
                                                 double sensorWidth, double sensorLength) :
    _sensorWidth(sensorWidth), _sensorLength(sensorLength),
    _cellWidth(sensorWidth/cellNX), _cellLength(sensorLength/cellNY) {

}

void BarrelTOFNeighborFinder::init(const dd4hep::Detector* detector) {
    _detector = detector;
    _converter = std::make_unique<dd4hep::rec::CellIDPositionConverter>(*detector);
}

int BarrelTOFNeighborFinder::_findBin(double value, double minRange, double binSize) {
    int bin = static_cast<int>((value - minRange) / binSize + 1e-10);
    // Make sure bin is within range
    return std::max(0, bin);
}

double BarrelTOFNeighborFinder::_binCenter(int bin, double minRange, double binSize) {
    return binSize/2 + bin*binSize + minRange;
}

double BarrelTOFNeighborFinder::_binLowEdge(int bin, double minRange, double binSize) {
    return bin*binSize + minRange;
}

void BarrelTOFNeighborFinder::setLogger(const std::shared_ptr<spdlog::logger>& log) {
    _log = log;
}



// return cell bin (defined internally within DetPosProcessor) list for all neighbors in a sensor
void BarrelTOFNeighborFinder::_findAllNeighborsInSensor(int cellBinX, int cellBinY, 
                                                int sensorBinX, int sensorBinY, 
                                    std::vector<std::pair<int, int>>& ans, 
                                    std::vector<std::vector<bool>>& dp) {
    // use MST to find all neighbor within a sensor
    // I can probably write down the formula by hand, but why do things manually when computer do everything for you?
    const std::vector<std::pair<int, int>> searchDirs{{0, 1}, {0, -1}, {1, 0}, {-1, 0}};
    for(const auto& dir : searchDirs) {
        int testCellBinX = cellBinX + dir.first;
        int testCellBinY = cellBinY + dir.second;
        if(testCellBinX >= 0 && testCellBinX < _cellNX &&
           testCellBinY >= 0 && testCellBinY < _cellNY) {
                // only look for cells that have not been searched
            if(!dp[testCellBinX][testCellBinY]) {
                dp[testCellBinX][testCellBinY] = true;
                double testX = this -> _binCenter(testCellBinX, _staveXMin, _cellWidth);
                double testY = this -> _binCenter(testCellBinY, _staveYMin, _cellLength);
                int testSensorBinX = this -> _findBin(testX, _staveXMin, _sensorWidth);
                int testSensorBinY = this -> _findBin(testY, _staveYMin, _sensorLength);
                if(testSensorBinX == sensorBinX && testSensorBinY == sensorBinY) {
                    // inside the same sensor
                    ans.push_back({testCellBinX, testCellBinY});
                    this -> _findAllNeighborsInSensor(testCellBinX, testCellBinY,
                                                      sensorBinX, sensorBinY,
                                                      ans, dp);
                }
            }
        }
    }
}

void BarrelTOFNeighborFinder::_searchNOBins(const dd4hep::rec::CellID& cellID, bool length, bool width) {
    // find the readout limits of the staves
    // It's too unreliable to ask users for it
    auto localPos = this -> cell2LocalPosition(cellID);
    auto currentMatrix = _currMatrix;
    std::vector<std::pair<int, int>> searchDirs;
    if(length) {
        searchDirs.push_back({0, 1});
        searchDirs.push_back({0, -1});        
    }
    if(width) {
        searchDirs.push_back({1, 0});
        searchDirs.push_back({-1, 0});        
    }

    double l[3], g[3];
    localPos.GetCoordinates(l);

    _staveXMin = std::min(_staveXMin, l[0] - _cellWidth/2);
    _staveXMax = std::max(_staveXMax, l[0] + _cellWidth/2);
    _staveYMin = std::min(_staveYMin, l[1] - _cellLength/2);
    _staveYMax = std::max(_staveYMax, l[1] + _cellLength/2);
    _cellNX = std::max(_cellNX, static_cast<int>((_staveXMax - _staveXMin)/_cellWidth));
    _cellNY = std::max(_cellNY, static_cast<int>((_staveYMax - _staveYMin)/_cellLength));
    _sensorNX = std::max(_sensorNX, static_cast<int>((_staveXMax - _staveXMin)/_sensorWidth));
    _sensorNY = std::max(_sensorNY, static_cast<int>((_staveYMax - _staveYMin)/_sensorLength));

    for(const auto& dir : searchDirs) {
        localPos.GetCoordinates(l);
        l[0] += dir.first*_cellWidth;
        l[1] += dir.second*_cellLength;
        if(l[0] < _staveXMin || l[0] > _staveXMax || l[1] < _staveYMin || l[1] > _staveYMax) {
            currentMatrix -> LocalToMaster(l, g);
            try {
                auto testCellID = this -> globalPosition2Cell(dd4hep::Position(g[0], g[1], g[2]));
                if(testCellID != cellID) {
                        // cell ID change means there is more cells beyond the current edges
                        this -> _searchNOBins(testCellID, length, width);
                }
            } catch(...) {
                // position not in stave. Will not proceed there
            }
        }
    }
    // find previous cell
}


dd4hep::Position BarrelTOFNeighborFinder::cell2GlobalPosition(const dd4hep::rec::CellID& cell) {   
    return _converter -> position(cell);
}

dd4hep::rec::CellID BarrelTOFNeighborFinder::globalPosition2Cell(const dd4hep::Position& pos) {
    return _converter -> cellID(pos);
} 



dd4hep::Position BarrelTOFNeighborFinder::cell2LocalPosition(const dd4hep::rec::CellID& cell) {
    auto geoManager = _detector->world().volume() -> GetGeoManager();
    auto position = this -> cell2GlobalPosition(cell);
    auto node = geoManager -> FindNode(position.x(), position.y(), position.z());
    _currMatrix = geoManager -> GetCurrentMatrix();

    double g[3], l[3];
    position.GetCoordinates(g);
    _currMatrix -> MasterToLocal(g, l);
    position.SetCoordinates(l);
    return position;
}

std::vector<dd4hep::rec::CellID> BarrelTOFNeighborFinder::findAllNeighborInSensor(const dd4hep::rec::CellID& hitCell) {
    // need to find readout grid boundaries the first time this function is called
    if(_staveYMin >= _staveYMax || _staveXMin >= _staveXMax)  {
        if(_log) _log -> info("Searching for all the active cells in a stave. Will stamble upon empty volume a few times. Don't worry about a few empty volume below.");
        this -> _searchNOBins(hitCell, true, false);
        this -> _searchNOBins(hitCell, false, true);
	if(_log) {
            _log -> info("BarrelTOFNeighborFinder finds {} cells and {} sensors along the long direction.", _cellNX, _sensorNX);
            _log -> info("Finds {} cells and {} sensors along the short direction.", _cellNY, _sensorNY);
            _log -> info("Dimension for the active area of a stave is {} < length < {} cm and {} < width < {} cm.", _staveYMin, _staveYMax, _staveXMin, _staveXMax);
            _log -> info("The search for cells starts at cellID = {}, with steps along the long direction = {} cm, short direction = {} cm", hitCell, _cellLength, _cellWidth);
	}
    }

    auto localPos = this -> cell2LocalPosition(hitCell); // this set the _currMatrix and current position
    double g[3], l[3];
    localPos.GetCoordinates(l);
    std::vector<dd4hep::rec::CellID> neighbors;

    // find cell bin with respect to the upper left hand corner
    int cellBinX = this -> _findBin(l[0], _staveXMin, _cellWidth);
    int cellBinY = this -> _findBin(l[1], _staveYMin, _cellLength);

    int sensorBinX = this -> _findBin(l[0], _staveXMin, _sensorWidth);
    int sensorBinY = this -> _findBin(l[1], _staveYMin, _sensorLength);

    // find neighboring bin
    std::vector<std::pair<int, int>> neighborBins;
    std::vector<std::vector<bool>> dp(_cellNX, std::vector<bool>(_cellNY, false));
    this -> _findAllNeighborsInSensor(cellBinX, cellBinY,
                                      sensorBinX, sensorBinY,
                                      neighborBins, dp);
    for(const auto& bin : neighborBins) {
        // convert bin to global position
        l[0] = this -> _binCenter(bin.first, _staveXMin, _cellWidth);
        l[1] = this -> _binCenter(bin.second, _staveYMin, _cellLength);

        _currMatrix -> LocalToMaster(l, g);

        // find cellID
        neighbors.push_back(_converter -> cellID(dd4hep::Position(g[0], g[1], g[2])));
    }

    return std::move(neighbors);
}

