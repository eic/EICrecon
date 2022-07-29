
#include <string>

#include <TFile.h>

class EICRootReader{

public:

    void OpenFile( const std::string &filename );

protected:

    std::shared_ptr<TFile> m_file;

};


