add_executable(truk main.cpp)

target_link_libraries(truk PRIVATE matter)

target_compile_features(truk PRIVATE cxx_std_20)

