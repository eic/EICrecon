//
// Created by xmei on 9/8/22.
//

#pragma once

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

void printPluginNames(std::vector<std::string> const& plugin_names) {
  JTablePrinter plugin_table;
  plugin_table.AddColumn("Plugin name");
  for (const auto& plugin_name : plugin_names) {
    plugin_table | plugin_name;
  }

  std::ostringstream ss;
  plugin_table.Render(ss);
  std::cout << ss.str() << std::endl;
}
