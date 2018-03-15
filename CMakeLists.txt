cmake_minimum_required(VERSION 2.6)
project(joj-re8)

AUX_SOURCE_DIRECTORY(src SRC)
add_executable(re8 ${SRC})
target_link_libraries(re8 fftw3f stdc++fs)

set(CMAKE_CXX_FLAGS_DEBUG  "-g -D_GLIBCXX_DEBUG") 

enable_testing()
add_test(worldIsMine re8 ../ust/wim.ust ../voicelibraries/teto output.wav)
