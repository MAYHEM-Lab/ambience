add_library(cmsis_core INTERFACE)
target_include_directories(cmsis_core SYSTEM INTERFACE include)
add_library(cmsis::core ALIAS cmsis_core)