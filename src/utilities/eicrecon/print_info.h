//
// Created by xmei on 9/8/22.
//

#ifndef EICRECON_PRINT_INFO_H
#define EICRECON_PRINT_INFO_H

#include <iostream>
#include <iomanip>
#include <JANA/Utils/JTablePrinter.h>

void printFactoryTable(JComponentSummary const& cs) {
    JTablePrinter factory_table;
    factory_table.AddColumn("Plugin");
    factory_table.AddColumn("Object name");
    factory_table.AddColumn("Tag");
    for (const auto& factory : cs.factories) {
        factory_table | factory.plugin_name | factory.object_name | factory.factory_tag;
    }

    std::ostringstream ss;
    factory_table.Render(ss);
    std::cout << ss.str() << std::endl;
}

#endif //EICRECON_PRINT_INFO_H
