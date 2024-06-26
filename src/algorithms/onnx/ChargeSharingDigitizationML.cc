// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Simon Gardner

#include <fmt/core.h>
#include <algorithm>
#include <cstddef>
#include <exception>
#include <gsl/pointers>
#include <iterator>
#include <ostream>
#include <JANA/JException.h>
#include <onnxruntime_c_api.h>
#include <onnxruntime_cxx_api.h>

#include "ChargeSharingDigitizationML.h"

namespace eicrecon {

    

  // Setup onnxruntime
  static std::string print_shape(const std::vector<std::int64_t>& v) {
    std::stringstream ss("");
    for (std::size_t i = 0; i < v.size() - 1; i++) ss << v[i] << "x";
    ss << v[v.size() - 1];
    return ss.str();
  }

  template <typename T>
  Ort::Value vec_to_tensor(std::vector<T>& data, const std::vector<std::int64_t>& shape) {
    Ort::MemoryInfo mem_info =
        Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault);
    auto tensor = Ort::Value::CreateTensor<T>(mem_info, data.data(), data.size(), shape.data(), shape.size());
    return tensor;
  }

  void ChargeSharingDigitizationML::init() {

    if (m_cfg.readout.empty()) {
      throw JException("Readout is empty");
    }
    try {
      m_seg    = m_detector->readout(m_cfg.readout).segmentation();
      m_id_dec = m_detector->readout(m_cfg.readout).idSpec().decoder();
      if (!m_cfg.x_field.empty()) {
        m_x_idx = m_id_dec->index(m_cfg.x_field);
        debug("Find x field {}, index = {}", m_cfg.x_field, m_x_idx);
      }
      if (!m_cfg.y_field.empty()) {
        m_y_idx = m_id_dec->index(m_cfg.y_field);
        debug("Find y field {}, index = {}", m_cfg.y_field, m_y_idx);
      }
      if (!m_cfg.sensor_field.empty()) {
        m_sensor_idx = m_id_dec->index(m_cfg.sensor_field);
        debug("Find sensor field {}, index = {}", m_cfg.sensor_field, m_sensor_idx);
      }
      if (!m_cfg.layer_field.empty()) {
        m_layer_idx = m_id_dec->index(m_cfg.layer_field);
        debug("Find layer field {}, index = {}", m_cfg.layer_field, m_layer_idx);
      }
      if (!m_cfg.module_field.empty()) {
        m_module_idx = m_id_dec->index(m_cfg.module_field);
        debug("Find module field {}, index = {}", m_cfg.module_field, m_module_idx);
      }
      if (!m_cfg.detector_field.empty()) {
        m_detector_idx = m_id_dec->index(m_cfg.detector_field);
        debug("Find detector field {}, index = {}", m_cfg.detector_field, m_detector_idx);
      }
    } catch (...) {
      error("Failed to load ID decoder for {}", m_cfg.readout);
      throw JException("Failed to load ID decoder"); //Probably needs removal/change to internal logger...
    }

    // onnxruntime setup
    Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "charge-sharing-digitization-ml");
    Ort::SessionOptions session_options;
    try {
      m_session = std::make_unique<Ort::Session>(env, m_cfg.modelPath.c_str(), session_options);

      // print name/shape of inputs
      Ort::AllocatorWithDefaultOptions allocator;
      debug("Input Node Name/Shape:");
      for (std::size_t i = 0; i < m_session->GetInputCount(); i++) {
        m_input_names.emplace_back(m_session->GetInputNameAllocated(i, allocator).get());
        m_input_shapes.emplace_back(m_session->GetInputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape());
        debug("\t{} : {}", m_input_names.at(i), print_shape(m_input_shapes.at(i)));
      }

      // print name/shape of outputs
      debug("Output Node Name/Shape:");
      for (std::size_t i = 0; i < m_session->GetOutputCount(); i++) {
        m_output_names.emplace_back(m_session->GetOutputNameAllocated(i, allocator).get());
        m_output_shapes.emplace_back(m_session->GetOutputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape());
        debug("\t{} : {}", m_output_names.at(i), print_shape(m_output_shapes.at(i)));
      }

      // convert names to char*
      m_input_names_char.resize(m_input_names.size(), nullptr);
      std::transform(std::begin(m_input_names), std::end(m_input_names), std::begin(m_input_names_char),
                     [&](const std::string& str) { return str.c_str(); });
      m_output_names_char.resize(m_output_names.size(), nullptr);
      std::transform(std::begin(m_output_names), std::end(m_output_names), std::begin(m_output_names_char),
                     [&](const std::string& str) { return str.c_str(); });

    } catch(std::exception& e) {
      error(e.what());
    }


  }

  void ChargeSharingDigitizationML::process(
      const ChargeSharingDigitizationML::Input& input,
      const ChargeSharingDigitizationML::Output& output) const {

    const auto [hits] = input;
    auto  [digihits, associations] = output;

    std::vector<float> input_tensor_values;
    std::vector<Ort::Value> input_tensors;

    //Vector of transformations
    std::vector<Eigen::Matrix2d> transformations;

    for(auto hit : *hits){
      // Convert CellID and x,y to local cell position
      // Get global cell position
      auto cellID  = hit.getCellID();
      edm4hep::Vector3d global_hit_pos_vec = hit.getPosition();
      edm4hep::Vector3f global_momentum_vec = hit.getMomentum();
      
      auto subCellPos         = getSubcellPosition(global_hit_pos_vec, cellID);
      auto subCellMomentum    = getSubcellMomentum(global_momentum_vec, cellID);

      // Get transformation matrix to fold the hit into a triangle
      // (0,0), (0,0.5), (0.5,0.5)
      auto transformation = getTransformationMatrix(subCellPos);

      // offset to between 0 and 1
      Eigen::Vector2d offset = {0.5,0.5};
      auto subSubCellPos = transformation*subCellPos + offset;
      // std::cout << "SubSubCellPos:" << std::endl;
      // std::cout << subSubCellPos << std::endl;


      auto subSubCellMomentum = transformation*subCellMomentum;   
      // std::cout << "SubSubCellMomentum:" << std::endl;
      // std::cout << subSubCellMomentum << std::endl;

      // Prepare input tensor
      input_tensor_values.push_back(subSubCellPos[0]);
      input_tensor_values.push_back(subSubCellPos[1]);
      input_tensor_values.push_back(subSubCellMomentum[0]);
      input_tensor_values.push_back(subSubCellMomentum[1]);   

      // Get x, y, sensor, layer, module, detector    
      transformations.push_back(transformation);
            
    }

    // m_input_shapes.front()[0] = input_tensor_values.size();
    std::int64_t batch_size = hits->size();
    if(batch_size==0){
      debug("No hits to digitize in this event.");
      return;
    }
    std::vector<std::int64_t> input_shape = {batch_size,m_input_shapes.front()[1]};
    debug("Input shape: {}", print_shape(input_shape));
    
    try{
      input_tensors.emplace_back(vec_to_tensor<float>(input_tensor_values, input_shape));
    } catch (const Ort::Exception& exception) {
      error("error creating tensor: {}", exception.what());
    }

    // Double-check the dimensions of the input tensor
    if (! input_tensors[0].IsTensor() || input_tensors[0].GetTensorTypeAndShapeInfo().GetShape() != input_shape) {
      debug("skipping because input tensor shape incorrect");
      return;
    }

    float* output_tensor_data;

    try {
      auto output_tensors = m_session->Run(Ort::RunOptions{nullptr}, m_input_names_char.data(), input_tensors.data(),
                                          m_input_names_char.size(), m_output_names_char.data(), m_output_names_char.size());

      // Double-check the dimensions of the output tensors
      if (!output_tensors[0].IsTensor() || output_tensors.size() != m_output_names.size()) {
        debug("skipping because output tensor shape incorrect");
        return;
      }

      auto& output_tensor = output_tensors.front();
      auto output_shape = output_tensor.GetTensorTypeAndShapeInfo().GetShape();
      debug("Output shape: {}", print_shape(output_shape));

      output_tensor_data = output_tensor.GetTensorMutableData<float>();
      
      // // Round the output to the nearest integer
      // for (std::size_t i = 0; i < output_shape[1]*batch_size; i++) {
      //   output_tensor_data[i] = std::round(output_tensor_data[i]);
      // }

      // //Print out the output tensor
      // std::cout << "Output tensor: ";
      // for (std::size_t i = 0; i < batch_size; i++) {
      //   std::cout << "Hit " << i << std::endl;
      //   for (std::size_t j = 0; j < 2; j++) {
      //     if(j==0){
      //       std::cout << "ToT: " << std::endl;
      //     } else {
      //       std::cout << "ToA: " << std::endl;
      //     }
      //     for (std::size_t k = 0; k < 6; k++) {
      //       for (std::size_t l = 0; l < 6; l++) {
      //         std::cout << output_tensor_data[i*72 + j*36 + k*6 + l] << "\t";
      //       }
      //       std::cout << std::endl;
      //     }
      //     std::cout << std::endl;
      //   }
      // }
      // std::cout << std::endl;

    } catch (const Ort::Exception& exception) {
      error("error running model inference: {}", exception.what());
    }

    // Transform the inference results back into ToT an
    for(std::size_t i=0; auto hit : *hits){
      
      auto cellID    = hit.getCellID();     
      auto hitTime   = hit.getTime();

      // Convert hit time into digitization time
      auto digiTime = std::round(hitTime/m_cfg.timeBinning);

      int xID        = m_id_dec->get(cellID, m_x_idx);
      int yID        = m_id_dec->get(cellID, m_y_idx);

      uint64_t newCellID = cellID;

      auto transformation = transformations[i];
      
      for (int k = 0; k < 6; k++) {
        for (int l = 0; l < 6; l++) {
          int32_t charge    = std::round(output_tensor_data[i*72 + k*6 + l]);
          if(charge<=0) continue;
          // std::cout << k << " " << l << std::endl;
          // std::cout << output_tensor_data[i*72 + k*6 + l] << " " << output_tensor_data[i*72 + 36 + k*6 + l] << std::endl;
          int32_t timeStamp = std::round(output_tensor_data[i*72 + 36 + k*6 + l]);
          // std::cout << charge << " " << timeStamp << std::endl;
          // std::cout << l-3 << " " << k-2 << std::endl;
          
          Eigen::Vector2d localCell = {l-3,k-2};
          // std::cout << localCell << std::endl;
          auto transformedCell = transformation*localCell;
          // std::cout  << transformedCell << std::endl;
          int newX = transformedCell[0]+xID;
          int newY = transformedCell[1]+yID;
          m_id_dec->set( newCellID, m_x_idx, newX  );
          m_id_dec->set( newCellID, m_y_idx, newY );

          // std::cout << newCellID << " " << newX << " " << newY << std::endl;

          // Add global time and convert back to ns
          auto outTime = (digiTime+timeStamp)*m_cfg.timeBinning;

          // Create new RawHit
          auto rawHit = digihits->create(newCellID,charge,outTime);

          // Create association
          auto hitAssoc = associations->create(1.0);
          hitAssoc.setRawHit(rawHit);
          hitAssoc.setSimHit(hit);

        }
      }

      // Create hits based on the x/y transformation from local inference to layer coordinates


      i++;
    }


  }

  //Transform the position into the local cell coordinates
  Eigen::Vector2d ChargeSharingDigitizationML::getSubcellPosition(const edm4hep::Vector3d global_pos, uint64_t cellID) const {
      
    auto cellPos = m_seg.position(cellID);

    //Translate global hit position into local hit coordinates
    double global_hit_pos[3] = {global_pos.x/10, global_pos.y/10, global_pos.z/10};

    // Get detector element global translation and rotation
    auto detElementContext = m_cellid_converter->findContext(cellID);
    auto subCellPos        = detElementContext->worldToLocal(global_hit_pos) - cellPos;

    // Divide into segmentation of the cell
    auto cellSize = m_seg.cellDimensions(cellID);

    // std::cout << "Cell position: " << subCellPos.X() << " " << subCellPos.Y() << " " << subCellPos.Z() << std::endl;

    double xPos = subCellPos.X()/cellSize[0];
    double yPos = subCellPos.Y()/cellSize[1];

    Eigen::Vector2d subCellPosCoords = {xPos,yPos};

    // Hack as algorithm doesn't quite work. Position sometimes apears outside of the cell
    if(subCellPosCoords[0]<-0.5 || subCellPosCoords[0]>0.5 || subCellPosCoords[1]<-0.5 || subCellPosCoords[1]>0.5){
      subCellPosCoords[0] = 0.0;
      subCellPosCoords[1] = 0.0;
    }
    
    //print out the cell position
    // std::cout << "Cell position: " << subCellPosCoords[0] << " " << subCellPosCoords[1] << std::endl << std::endl;

    return subCellPosCoords;

  }

  //Transform the momentum into the local cell coordinates
  Eigen::Vector2d ChargeSharingDigitizationML::getSubcellMomentum(const edm4hep::Vector3f global_mom, uint64_t cellID) const {
    
    //calculate the magnitude of the momentum
    double mom_mag = sqrt(global_mom.x*global_mom.x + global_mom.y*global_mom.y + global_mom.z*global_mom.z);

    Eigen::Vector3d global_mom_unit = {global_mom.x/mom_mag, global_mom.y/mom_mag, global_mom.z/mom_mag};
    
    // Get detector element global translation and rotation
    auto detElementContext  = m_cellid_converter->findContext(cellID);
    auto detElementRotation = detElementContext->toElement().GetRotationMatrix();

    Eigen::Matrix3d eigenRotation;
    eigenRotation << detElementRotation[0], detElementRotation[1], detElementRotation[2],
                          detElementRotation[3], detElementRotation[4], detElementRotation[5],
                          detElementRotation[6], detElementRotation[7], detElementRotation[8];

    // Transform the momentum into the local cell coordinates
    auto unitMom = eigenRotation*global_mom_unit;

    Eigen::Vector2d unitMomXY = {unitMom[0],unitMom[1]};
    
    // Hack to match range of trained model
    if(unitMom[2]>-0.9){
      unitMomXY[0] = 0.0;
      unitMomXY[1] = 0.0;
    }
    // print out the cell position
    // std::cout << "Momentum: " << unitMom[0] << " " << unitMom[1] << " " << unitMom[2] << std::endl << std::endl;
    // std::cout << "Momentum: " << unitMomXY[0] << " " << unitMomXY[1] << std::endl << std::endl;

    return unitMomXY;
  }

  // Fold the hit into a triangle and keep track of the transformation
  Eigen::Matrix2d ChargeSharingDigitizationML::getTransformationMatrix(const Eigen::Vector2d point) const {
    Eigen::Matrix2d transformation;
    transformation << 1, 0, 0, 1;
    // std::cout << "Transformation matrix: \n" << transformation << std::endl;

    if(point[0] > 0){
      transformation(0,0) = -1;
    }
    if(point[1] > 0){
      transformation(1,1) = -1;
    }
    if(point[0]*transformation(0,0)>point[1]*transformation(1,1)){
      transformation(1,0) = transformation(0,0);
      transformation(0,1) = transformation(1,1);
      transformation(0,0) = 0;
      transformation(1,1) = 0;
    }
    //print matrix
    // debug("Transformation matrix: \n {} \n \n", transformation)

    return transformation;
  }

} // namespace eicrecon
