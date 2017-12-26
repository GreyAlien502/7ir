cmake_minimum_required(VERSION 2.6)
project(joj-re8)

AUX_SOURCE_DIRECTORY(src SRC)
add_executable(re8 ${SRC})
target_link_libraries(re8 fftw3f stdc++fs)

enable_testing()
add_test(worldIsMine re8 ../ust.ust ../voicelibraries/teto output.wav)
