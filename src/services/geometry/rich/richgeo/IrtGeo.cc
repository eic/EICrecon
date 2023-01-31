// Copyright 2022, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include "IrtGeo.h"

// constructor: creates IRT-DD4hep bindings using main `Detector` handle `*det_`
rich::IrtGeo::IrtGeo(std::string detName_, dd4hep::Detector *det_, bool verbose_) :
  m_detName(detName_), m_det(det_), m_log(Logger::Instance(verbose_))
{
  Bind();
}

// alternate constructor: use compact file for DD4hep geometry (backward compatibility)
rich::IrtGeo::IrtGeo(std::string detName_, std::string compactFile_, bool verbose_) :
  m_detName(detName_), m_log(Logger::Instance(verbose_))
{
  // compact file name; if it's not been specified, try to find the default one
  std::string compactFile;
  if(compactFile_=="") {
    std::string DETECTOR_PATH(getenv("DETECTOR_PATH"));
    std::string DETECTOR_CONFIG(getenv("DETECTOR_CONFIG"));
    if(DETECTOR_PATH.empty() || DETECTOR_CONFIG.empty()) {
      m_log.PrintError("cannot find default compact file, since env vars DETECTOR_PATH and DETECTOR_CONFIG are not set");
      return;
    }
    compactFile = DETECTOR_PATH + "/" + DETECTOR_CONFIG + ".xml";
  } else compactFile = compactFile_;

  // build DD4hep detector from compact file
  m_det = &dd4hep::Detector::getInstance();
  m_det->fromXML(compactFile);

  // set IRT and DD4hep geometry handles
  Bind();
}

// Bind() -----------------------------------------
// set IRT and DD4hep geometry handles
void rich::IrtGeo::Bind() {

  // DD4hep geometry handles
  m_detRich = m_det->detector(m_detName);
  m_posRich = m_detRich.placement().position();

  // IRT geometry handles
  m_irtDetectorCollection = new CherenkovDetectorCollection();
  m_irtDetector = m_irtDetectorCollection->AddNewDetector(m_detName.c_str());

  // cellID conversion
  m_cellid_converter = std::make_shared<const dd4hep::rec::CellIDPositionConverter>(*m_det);

  // define the `cell ID -> pixel position` converter, correcting the returned
  // `Position` to be at the sensor surface
  m_irtDetector->m_ReadoutIDToPixelPosition = [this] (auto cell_id) {
    auto pixel_volume_centroid    = (1/dd4hep::mm) * m_cellid_converter->position(cell_id);
    auto sensor_id                = m_cellid_converter->findContext(cell_id)->element.id();
    auto sensor_surface_offset_it = m_sensor_surface_offset.find(sensor_id);
    if(sensor_surface_offset_it == m_sensor_surface_offset.end()) {
      m_log.PrintError("WARNING: cannot find sensor ID {} in IrtGeo",sensor_id);
      // return the volume centroid instead
      return TVector3( pixel_volume_centroid.x(), pixel_volume_centroid.y(), pixel_volume_centroid.z());
    }
    auto pixel_surface_centroid = pixel_volume_centroid + sensor_surface_offset_it->second;
    return TVector3( pixel_surface_centroid.x(), pixel_surface_centroid.y(), pixel_surface_centroid.z());
  };

  // define the `cell ID -> sensor position` converter
  m_irtDetector->m_ReadoutIDToSensorPosition = [this] (auto cell_id) {
    // auto pixel_volume_centroid = m_cellid_converter->position(cell_id);
    // auto sensor_pos = m_cellid_converter->findContext(cell_id)->element.placement().position(); // FIXME: broken!
    // auto sensor_pos = m_cellid_converter->findDetElement(pixel_volume_centroid).placement().position(); // FIXME: broken!
    auto sensor_id = m_cellid_converter->findContext(cell_id)->element.id();
    auto sensor_centroid_it = m_sensor_centroid.find(sensor_id);
    if(sensor_centroid_it == m_sensor_centroid.end()) {
      m_log.PrintError("WARNING: cannot find sensor ID {} in IrtGeo",sensor_id);
      return TVector3(0, 0, 0);
    }
    auto sensor_pos = sensor_centroid_it->second;
    return TVector3( sensor_pos.x(), sensor_pos.y(), sensor_pos.z());
  };

}
// ------------------------------------------------

// fill table of refractive indices
void rich::IrtGeo::SetRefractiveIndexTable() {
  m_log.PrintLog("{:-^60}"," Refractive Index Tables ");
  for(auto rad_obj : m_irtDetector->Radiators()) {
    m_log.PrintLog("{}:", rad_obj.first);
    const auto rad = rad_obj.second;
    auto rindex_matrix = m_det->material(rad->GetAlternativeMaterialName()).property("RINDEX"); 
    for(unsigned row=0; row<rindex_matrix->GetRows(); row++) {
      auto energy = rindex_matrix->Get(row,0) / dd4hep::eV;
      auto rindex = rindex_matrix->Get(row,1);
      m_log.PrintLog("  {:>5} eV   {:<}", energy, rindex);
      rad->m_ri_lookup_table.push_back({energy,rindex});
    }
  }
}

// destructor
rich::IrtGeo::~IrtGeo() {
  if(m_irtDetector) delete m_irtDetector;
  if(m_irtDetectorCollection) delete m_irtDetectorCollection;
}
