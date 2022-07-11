
#include <iostream>
#include <vector>
#include <string>

#include <JANA/JApplication.h>
#include <JANA/Services/JServiceLocator.h>

#include "DD4hep/Detector.h"
#include "DDRec/CellIDPositionConverter.h"
#include "DDRec/SurfaceManager.h"
#include "DDRec/Surface.h"
#include "DD4hep/DD4hepUnits.h"


class JDD4hep_service : public JService
{
public:
    JDD4hep_service( JApplication *app );


private:
    JDD4hep_service(){}

    dd4hep::Detector* m_dd4hepGeo = nullptr;
	std::shared_ptr<const dd4hep::rec::CellIDPositionConverter> m_cellid_converter = nullptr;
    std::vector<std::string> m_xmlFileNames;
};
