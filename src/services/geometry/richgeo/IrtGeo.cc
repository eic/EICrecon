// Copyright (C) 2022, 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "IrtGeo.h"

// constructor: creates IRT-DD4hep bindings using main `Detector` handle `*det_`
richgeo::IrtGeo::IrtGeo(std::string detName_, dd4hep::Detector *det_, std::shared_ptr<spdlog::logger> log_) :
  m_detName(detName_), m_det(det_), m_log(log_)
{
  Bind();
}

// alternate constructor: use compact file for DD4hep geometry (backward compatibility)
richgeo::IrtGeo::IrtGeo(std::string detName_, std::string compactFile_, std::shared_ptr<spdlog::logger> log_) :
  m_detName(detName_), m_log(log_)
{
  // build DD4hep detector from compact file
  m_det = &dd4hep::Detector::getInstance();
  m_det->fromXML(compactFile_);

  // set IRT and DD4hep geometry handles
  Bind();
}

// Bind() -----------------------------------------
// set IRT and DD4hep geometry handles
void richgeo::IrtGeo::Bind() {
  // DD4hep geometry handles
  m_detRich = m_det->detector(m_detName);
  m_posRich = m_detRich.placement().position();

  // IRT geometry handles
  m_irtDetectorCollection = new CherenkovDetectorCollection();
  m_irtDetector = m_irtDetectorCollection->AddNewDetector(m_detName.c_str());

  // cellID conversion
  m_cellid_converter = std::make_shared<const dd4hep::rec::CellIDPositionConverter>(*m_det);
}
// ------------------------------------------------

// define the `cell ID -> pixel position` converter, correcting to sensor surface
void richgeo::IrtGeo::SetReadoutIDToPositionLambda() {

  m_irtDetector->m_ReadoutIDToPosition = [
    &m_log = this->m_log, // capture logger by reference
    // capture instance members by value, so those owned by `this` are not mutable here
    cell_mask        = this->m_irtDetector->GetReadoutCellMask(),
    cellid_converter = this->m_cellid_converter,
    sensor_info      = this->m_sensor_info
  ] (auto cell_id) {
    // decode cell ID to get the sensor ID and pixel volume centroid
    auto sensor_id = cell_id & cell_mask;
    auto pixel_volume_centroid = (1/dd4hep::mm) * cellid_converter->position(cell_id);
    // get sensor info
    auto sensor_info_it = sensor_info.find(sensor_id);
    if(sensor_info_it == sensor_info.end()) {
      m_log->warn("cannot find sensor ID {} in IrtGeo; using pixel volume centroid instead",sensor_id);
      return TVector3( pixel_volume_centroid.x(), pixel_volume_centroid.y(), pixel_volume_centroid.z());
    }
    auto sensor_obj = sensor_info_it->second;
    // get pixel surface centroid, given sensor surface offset w.r.t centroid
    auto pixel_surface_centroid  = pixel_volume_centroid + sensor_obj.surface_offset;
	printf("------>>>>> %lf %lf %lf\n",sensor_obj.surface_offset.X(),sensor_obj.surface_offset.Y(),sensor_obj.surface_offset.Z());
    // cross check: make sure pixel and sensor surface centroids are close enough
    auto dist = sqrt((pixel_surface_centroid - sensor_obj.surface_centroid).Mag2());
    if( dist > sensor_obj.size / sqrt(2) )
      m_log->warn("dist(pixel,sensor) is too large: {} mm",dist);
    return TVector3( pixel_surface_centroid.x(), pixel_surface_centroid.y(), pixel_surface_centroid.z());
  };

}
// ------------------------------------------------

// fill table of refractive indices
void richgeo::IrtGeo::SetRefractiveIndexTable() {
  m_log->debug("{:-^60}"," Refractive Index Tables ");
  for(auto rad_obj : m_irtDetector->Radiators()) {
    m_log->debug("{}:", rad_obj.first);
    const auto rad = rad_obj.second;
    auto rindex_matrix = m_det->material(rad->GetAlternativeMaterialName()).property("RINDEX");
    for(unsigned row=0; row<rindex_matrix->GetRows(); row++) {
      auto energy = rindex_matrix->Get(row,0) / dd4hep::eV;
      auto rindex = rindex_matrix->Get(row,1);
      m_log->debug("  {:>5} eV   {:<}", energy, rindex);
      rad->m_ri_lookup_table.push_back({energy,rindex});
    }
  }
}

// destructor
richgeo::IrtGeo::~IrtGeo() {
  delete m_irtDetector;
  delete m_irtDetectorCollection;
}
