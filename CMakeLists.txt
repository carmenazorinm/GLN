cmake_minimum_required(VERSION 3.15)
project(GLN LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(GLN_USE_GMP   "Use GMP as BigInt backend" OFF)
option(GLN_USE_BOOST "Use Boost.Multiprecision as BigInt backend" ON)

include_directories(${PROJECT_SOURCE_DIR}/include)

# --- SOLO FUENTES EXISTENTES ---
set(SOURCES
    src/bigint_boost.cpp
    src/nt_utils.cpp
    src/params.cpp
    src/keygen.cpp
    src/encrypt.cpp
    src/decrypt.cpp
    src/kem.cpp
    src/attack.cpp
    experiments/experiment_g_conditions.cpp
    experiments/experiment_trios_attack.cpp 
)

add_library(gln_core ${SOURCES})
target_include_directories(gln_core PRIVATE ${PROJECT_SOURCE_DIR}/extern/boost_1_89_0)

if (GLN_USE_GMP)
  find_package(GMP REQUIRED)
  target_compile_definitions(gln_core PRIVATE GLN_USE_GMP=1)
  target_link_libraries(gln_core PRIVATE GMP::GMP GMP::GMPXX)
elseif(GLN_USE_BOOST)
  target_compile_definitions(gln_core PRIVATE GLN_USE_GMP=0)
  # Si tus headers Boost no están en /usr/include, añade:
  # target_include_directories(gln_core PRIVATE /ruta/a/boost/include)
else()
  message(FATAL_ERROR "Selecciona backend: -DGLN_USE_GMP=ON o -DGLN_USE_BOOST=ON")
endif()

# --- UN SOLO BINARIO MINIMO ---
add_executable(gln_keygen apps/gln_keygen.cpp)
target_link_libraries(gln_keygen gln_core)

# --- TESTS ---
enable_testing()  # Añade esto para habilitar el soporte de testing

# Define los ejecutables de prueba
add_executable(test_gcd tests/test_gcd.cpp)
add_executable(test_bigint tests/test_bigint.cpp)
add_executable(test_treea tests/test_treea.cpp)
add_executable(test_invmod tests/test_invmod.cpp)
add_executable(test_params tests/test_params.cpp)
add_executable(test_keygen tests/test_keygen.cpp)
add_executable(test_encrypt tests/test_encrypt.cpp)
add_executable(test_decrypt tests/test_decrypt.cpp)
add_executable(test_kem tests/test_kem.cpp)
add_executable(test_attack tests/test_attack.cpp)

# Enlaza las librerías necesarias
target_link_libraries(test_gcd gln_core)
target_link_libraries(test_bigint gln_core)
target_link_libraries(test_treea gln_core)
target_link_libraries(test_invmod gln_core)
target_link_libraries(test_params gln_core)
target_link_libraries(test_keygen gln_core)
target_link_libraries(test_encrypt gln_core)
target_link_libraries(test_decrypt gln_core)
target_link_libraries(test_kem gln_core)
target_link_libraries(test_attack gln_core)

# Añade las pruebas DESPUÉS de definir los ejecutables
add_test(NAME test_gcd COMMAND test_gcd)
add_test(NAME test_bigint COMMAND test_bigint)
add_test(NAME test_treea COMMAND test_treea)
add_test(NAME test_invmod COMMAND test_invmod)
add_test(NAME test_params COMMAND test_params)
add_test(NAME test_keygen COMMAND test_keygen)
add_test(NAME test_encrypt COMMAND test_encrypt)
add_test(NAME test_decrypt COMMAND test_decrypt)
add_test(NAME test_kem COMMAND test_kem)
add_test(NAME test_attack COMMAND test_attack)

# Example
add_executable(demo_pke examples/demo_pke.cpp)
target_include_directories(demo_pke PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(demo_pke PRIVATE gln_core)

add_executable(demo_kem examples/demo_kem.cpp)
target_include_directories(demo_kem PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(demo_kem PRIVATE gln_core)

add_executable(experiment_g_conditions experiments/experiment_g_conditions.cpp)
target_include_directories(experiment_g_conditions PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(experiment_g_conditions PRIVATE gln_core)

add_executable(experiment_trios_attack experiments/experiment_trios_attack.cpp)
target_include_directories(experiment_trios_attack PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(experiment_trios_attack PRIVATE gln_core)
