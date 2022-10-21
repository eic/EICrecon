cmake_minimum_required(VERSION 3.16)

print_header(">>>>   D I R E C T O R Y :   algorithms    <<<<")       # Fancy printing

add_subdirectory( calorimetry )
add_subdirectory( tracking )
add_subdirectory( digi )
add_subdirectory( reco )
