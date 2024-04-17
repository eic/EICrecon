cmake_minimum_required(VERSION 3.16)

<<<<<<< HEAD
add_subdirectory( interfaces )
add_subdirectory( calorimetry )
add_subdirectory( tracking )
add_subdirectory( pid )
add_subdirectory( digi )
add_subdirectory( reco )
add_subdirectory( fardetectors )
add_subdirectory( postburn )
=======
add_subdirectory(interfaces)
add_subdirectory(calorimetry)
add_subdirectory(tracking)
add_subdirectory(pid)
add_subdirectory(digi)
add_subdirectory(reco)
add_subdirectory(fardetectors)
<<<<<<< HEAD
>>>>>>> 5f2afbb9 (feat(ci): run cmake-format in ci (#1372))
=======

if(USE_ONNX)
  add_subdirectory(onnx)
endif()
>>>>>>> 850dd437 (feat: example of how to use ONNX Runtime (Ort) in algorithms (#1358))
