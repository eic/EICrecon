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
  // static std::string print_shape(const std::vector<std::int64_t>& v) {
  //   std::stringstream ss("");
  //   for (std::size_t i = 0; i < v.size() - 1; i++) ss << v[i] << "x";
  //   ss << v[v.size() - 1];
  //   return ss.str();
  // }

  // template <typename T>
  // Ort::Value vec_to_tensor(std::vector<T>& data, const std::vector<std::int64_t>& shape) {
  //   Ort::MemoryInfo mem_info =
  //       Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault);
  //   auto tensor = Ort::Value::CreateTensor<T>(mem_info, data.data(), data.size(), shape.data(), shape.size());
  //   return tensor;
  // }

  void ChargeSharingDigitizationML::init() {

    std::cout << "ChargeSharingDigitizationML::init()" << std::endl;

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
    Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "inclusive-kinematics-ml");
    Ort::SessionOptions session_options;
    try {
      m_session = std::make_unique<Ort::Session>(env, m_cfg.modelPath.c_str(), session_options);

  //     // print name/shape of inputs
  //     Ort::AllocatorWithDefaultOptions allocator;
  //     debug("Input Node Name/Shape:");
  //     for (std::size_t i = 0; i < m_session.GetInputCount(); i++) {
  //       m_input_names.emplace_back(m_session.GetInputNameAllocated(i, allocator).get());
  //       m_input_shapes.emplace_back(m_session.GetInputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape());
  //       debug("\t{} : {}", m_input_names.at(i), print_shape(m_input_shapes.at(i)));
  //     }

  //     // print name/shape of outputs
  //     debug("Output Node Name/Shape:");
  //     for (std::size_t i = 0; i < m_session.GetOutputCount(); i++) {
  //       m_output_names.emplace_back(m_session.GetOutputNameAllocated(i, allocator).get());
  //       m_output_shapes.emplace_back(m_session.GetOutputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape());
  //       debug("\t{} : {}", m_output_names.at(i), print_shape(m_output_shapes.at(i)));
  //     }

  //     // convert names to char*
  //     m_input_names_char.resize(m_input_names.size(), nullptr);
  //     std::transform(std::begin(m_input_names), std::end(m_input_names), std::begin(m_input_names_char),
  //                    [&](const std::string& str) { return str.c_str(); });
  //     m_output_names_char.resize(m_output_names.size(), nullptr);
  //     std::transform(std::begin(m_output_names), std::end(m_output_names), std::begin(m_output_names_char),
  //                    [&](const std::string& str) { return str.c_str(); });

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

      auto subSubCellMomentum = transformation*subCellMomentum;   

      // Prepare input tensor
      input_tensor_values.push_back(subSubCellPos[0]);
      input_tensor_values.push_back(subSubCellPos[1]);
      input_tensor_values.push_back(subSubCellMomentum[0]);
      input_tensor_values.push_back(subSubCellMomentum[1]);   

      // Get x, y, sensor, layer, module, detector
      auto x        = m_id_dec->get(cellID, m_x_idx);
      auto y        = m_id_dec->get(cellID, m_y_idx);
      auto sensor   = m_id_dec->get(cellID, m_sensor_idx);
      auto lay      = m_id_dec->get(cellID, m_layer_idx);
      auto mod      = m_id_dec->get(cellID, m_module_idx);
      auto det      = m_id_dec->get(cellID, m_detector_idx);

    }

  //   // Assume model has 1 input nodes and 1 output node.
  //   if (m_input_names.size() != 1 || m_output_names.size() != 1) {
  //     debug("skipping because model has incorrect input and output size");
  //     return;
  //   }

  //   // Prepare input tensor
  //   std::vector<float> input_tensor_values;
  //   std::vector<Ort::Value> input_tensors;
  //   for (std::size_t i = 0; i < electron->size(); i++) {
  //     input_tensor_values.push_back(electron->at(i).getX());
  //   }
  //   input_tensors.emplace_back(vec_to_tensor<float>(input_tensor_values, m_input_shapes.front()));

  //   // Double-check the dimensions of the input tensor
  //   if (! input_tensors[0].IsTensor() || input_tensors[0].GetTensorTypeAndShapeInfo().GetShape() != m_input_shapes.front()) {
  //     debug("skipping because input tensor shape incorrect");
  //     return;
  //   }

  //   // Attempt inference
  //   try {
  //     auto output_tensors = m_session.Run(Ort::RunOptions{nullptr}, m_input_names_char.data(), input_tensors.data(),
  //                                         m_input_names_char.size(), m_output_names_char.data(), m_output_names_char.size());

  //     // Double-check the dimensions of the output tensors
  //     if (!output_tensors[0].IsTensor() || output_tensors.size() != m_output_names.size()) {
  //       debug("skipping because output tensor shape incorrect");
  //       return;
  //     }

  //     // Convert output tensor
  //     float* output_tensor_data = output_tensors[0].GetTensorMutableData<float>();
  //     auto x  = output_tensor_data[0];
  //     auto kin = ml->create();
  //     kin.setX(x);

  //   } catch (const Ort::Exception& exception) {
  //     error("error running model inference: {}", exception.what());
  //   }
  }

  Eigen::Vector2d ChargeSharingDigitizationML::getSubcellPosition(const edm4hep::Vector3d global_pos, uint64_t cellID) const {
      
    auto cellPos = m_seg.position(cellID);

    //Translate global hit position into local hit coordinates
    double global_hit_pos[3] = {global_pos.x/10, global_pos.y/10, global_pos.z/10};

    // Get detector element global translation and rotation
    auto detElementContext = m_cellid_converter->findContext(cellID);
    auto subCellPos        = detElementContext->worldToLocal(global_hit_pos) - cellPos;

    // Divide into segmentation of the cell
    auto cellSize = m_seg.cellDimensions(cellID);

    std::cout << "Cell position: " << subCellPos.X() << " " << subCellPos.Y() << " " << subCellPos.Z() << std::endl;

    double xPos = subCellPos.X()/cellSize[0];
    double yPos = subCellPos.Y()/cellSize[1];

    Eigen::Vector2d subCellPosCoords = {xPos,yPos};
    
    //print out the cell position
    std::cout << "Cell position: " << subCellPosCoords[0] << " " << subCellPosCoords[1] << std::endl << std::endl;

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
    
    //print out the cell position
    std::cout << "Momentum: " << unitMomXY[0] << " " << unitMomXY[1] << std::endl << std::endl;

    return unitMomXY;
  }

  // Fold the hit into a triangle and keep track of the transformation
  Eigen::Matrix2d ChargeSharingDigitizationML::getTransformationMatrix(const Eigen::Vector2d point) const {
    Eigen::Matrix2d transformation;
    transformation << 1, 0, 0, 1;
    std::cout << "Transformation matrix: \n" << transformation << std::endl;

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
    std::cout << "Transformation matrix: \n" << transformation << std::endl << std::endl;

    return transformation;
  }

} // namespace eicrecon
