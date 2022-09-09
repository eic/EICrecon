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

void printJANAHeaderIMG() {
    std::cout << "     ____      _     ___      ___       _               \n"
                 "     `MM'     dM.    `MM\\     `M'      dM.              \n"
                 "      MM     ,MMb     MMM\\     M      ,MMb              \n"
                 "      MM     d'YM.    M\\MM\\    M      d'YM.      ____   \n"
                 "      MM    ,P `Mb    M \\MM\\   M     ,P `Mb     6MMMMb  \n"
                 "      MM    d'  YM.   M  \\MM\\  M     d'  YM.   MM'  `Mb \n"
                 "      MM   ,P   `Mb   M   \\MM\\ M    ,P   `Mb        ,MM \n"
                 "      MM   d'    YM.  M    \\MM\\M    d'    YM.      ,MM' \n"
                 "(8)   MM  ,MMMMMMMMb  M     \\MMM   ,MMMMMMMMb    ,M'    \n"
                 "((   ,M9  d'      YM. M      \\MM   d'      YM. ,M'      \n"
                 " YMMMM9 _dM_     _dMM_M_      \\M _dM_     _dMM_MMMMMMMM " << std::endl << std::endl;
}

#endif //EICRECON_PRINT_INFO_H
