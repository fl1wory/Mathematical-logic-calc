# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/LogicSimplifier_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/LogicSimplifier_autogen.dir/ParseCache.txt"
  "LogicSimplifier_autogen"
  )
endif()
