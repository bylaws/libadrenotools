add_library(main_hook SHARED main_hook.cpp)

target_compile_options(main_hook PRIVATE -Wall -Wextra)
target_link_options(main_hook PRIVATE -z global)
target_link_libraries(main_hook linkernsbypass log)
target_include_directories(main_hook PRIVATE ../../include)
set_target_properties(main_hook PROPERTIES CXX_VISIBILITY_PRESET hidden)

add_library(file_redirect_hook SHARED file_redirect_hook.cpp)

target_compile_options(file_redirect_hook PRIVATE -Wall -Wextra)
target_link_options(file_redirect_hook PRIVATE -z global)
target_link_libraries(file_redirect_hook linkernsbypass dl log)
target_include_directories(file_redirect_hook PRIVATE ../../include)
set_target_properties(file_redirect_hook PROPERTIES CXX_VISIBILITY_PRESET hidden)
