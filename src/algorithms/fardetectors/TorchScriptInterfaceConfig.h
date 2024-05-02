#pragma once

#include <string>

using string = std::string;

namespace eicrecon {

  struct TorchScriptInterfaceConfig {
    string model_x_file_path = "RP_model_x_1.0.pt";
    string model_y_file_path = "RP_model_y_1.0.pt";
    string model_z_file_path = "RP_model_y_1.0.pt";
  };

}
