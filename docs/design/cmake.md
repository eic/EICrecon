# plugins CMake API

There is a copy/paste CMake file that should automatically create plugin out of sources.

- plugin name is taken from a directory name
- there should be /</plugin name/>/.cc file with `void InitPlugin(JApplication *app)` function


### Create a plugin:

E.g. if you want to create a plugin named `my_plugin`

- Create a directory `my_plugin`
- Create a file `my_plugin.cc` which will have `InitPlugin` function
- Create `CMakeLists.txt` with the content below
- Add all others files (cmake GLOB is used)

## Recommended cmake:

Recommended CMake for a plugin:

```cmake
cmake_minimum_required(VERSION 3.16)

get_filename_component(PLUGIN_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)

# Function creates ${PLUGIN_NAME}_plugin and ${PLUGIN_NAME}_library targets
# Setting default includes, libraries and installation paths
plugin_add(${PLUGIN_NAME} WITH_STATIC_LIBRARY)

# The macro grabs sources as *.cc *.cpp *.c and headers as *.h *.hh *.hpp
# Then correctly sets sources for ${_name}_plugin and ${_name}_library targets
# Adds headers to the correct installation directory
plugin_glob_all(${PLUGIN_NAME})

# Find dependencies
# Uncomment below as needed:
# plugin_add_dd4hep(${PLUGIN_NAME})
# plugin_add_acts(${PLUGIN_NAME})
# plugin_add_cern_root(${PLUGIN_NAME})
plugin_add_event_model(${PLUGIN_NAME})

# Add include directories (works same as target_include_directories)
# plugin_include_directories(${PLUGIN_NAME} SYSTEM PUBLIC ... )

# Add libraries (works similar target_include_directories but for plugin targets)
# plugin_link_libraries(${PLUGIN_NAME} ... )


```

## CMake macros:

There are `plugin_...` macros that are slim wrappers trying to minimize an amount of boilerplate
code of each plugin cmake scripts. Macros mimic CMake functions like `target_link_libraries` => `plugin_link_libraries`.


#### plugin_add

```cmake
# Function creates ${PLUGIN_NAME}_plugin target
# Sets default includes, libraries and installation paths
plugin_add(my_plugin)
```

It is possible to also automatically crate a static library from a plugin
sources in addition to the plugin itself. Adding `WITH_STATIC_LIBRARY` to
`plugin_add`. All other `plugin_xxx` functions will know about the second target then.

```cmake
# Now function will create 2 targets and ${PLUGIN_NAME}_plugin ${PLUGIN_NAME}_library
# one can add WITH_STATIC_LIBRARY flag to also create a static library with plugin sources
plugin_add(my_plugin WITH_STATIC_LIBRARY)
```

If `WITH_STATIC_LIBRARY` flag is given, all `plugin_...` macros will work on both targets:
a plugin and a static library.

#### plugin_glob_all

The macro grabs sources as *.cc *.cpp *.c and headers as *.h *.hh *.hpp
Then correctly sets sources for ${plugin_name}_plugin and ${plugin_name}_library
targets (if library is enabled).
Adds headers to the correct installation directory

```cmake
plugin_glob_all(my_plugin)
```

#### plugin_sources

Same as target_sources both for library (if enabled) and a plugin.
If library creation is enabled, the function automatically removes
`/</plugin-name/>/.cc` file from library sources

```cmake
plugin_sources(my_plugin File1.cc File2.cc)
```

### plugin_include_directories

Runs target_include_directories for both a plugin and a library (if enabled)

```cmake
#example
plugin_include_directories(${PLUGIN_NAME} SYSTEM PUBLIC  ${ROOT_INCLUDE_DIRS})
```

### plugin_link_libraries
Runs target_link_libraries for both a plugin and a library (if enabled)

```cmake
# example
plugin_link_libraries(${PLUGIN_NAME} ${JANA_LIB})
```

### plugin_add_|PACKAGE|

The next snippets combine boiler code for common libraries.
They also try to use packages that are already found.
Consider using them instead of find_packge(...)+plugin_link_libraries+plugin_include_directories


- plugin_add_event_model - podio, edm4hep, edm4eic
- plugin_add_acts - ACTS with version check
- plugin_add_cern_root - CERN ROOT
- plugin_add_dd4hep - dd4hep + Geant4


```cmake
# podio, edm4hep, edm4eic
plugin_add_event_model(${PLUGIN_NAME})

# acts with common components
plugin_add_acts(${PLUGIN_NAME})

# common cern ROOT libraries & include dirs
plugin_add_cern_root(${PLUGIN_NAME})

# dd4hep
plugin_add_dd4hep(${PLUGIN_NAME})
```
