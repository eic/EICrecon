// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef EICRECON_JCHAINFACTORYT_H
#define EICRECON_JCHAINFACTORYT_H

#include <string>
#include <vector>

#include <JANA/JFactoryT.h>


template <typename OutT>
class JChainFactoryT : public JFactoryT<OutT> {

public:
    JChainFactoryT( std::vector<std::string> default_input_tags ):
            m_default_input_tags( std::move(default_input_tags) )
    {
    }

    std::vector<std::string>& GetDefaultInputTags() { return m_default_input_tags; }

private:
    std::vector<std::string> m_default_input_tags;
};

#endif //EICRECON_JCHAINFACTORYT_H
