#pragma once

namespace eicrecon {

  struct TorchScriptInterfaceConfig {

    std::string ml_model_registry{"./"};
    std::vector<std::string> default_ml_models{"RP_model_x_1.0.pt", "RP_model_y_1.0.pt", "RP_model_z_1.0.pt"};

  };

}
