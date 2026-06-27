set(OTP_DEVICE_TYPE "simulate")

set(FLASH_DEVICE_TYPE "simulate")

# include path
include_directories(${CMAKE_SOURCE_DIR}/inc)
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/src/service)

# src files
aux_source_directory(${CMAKE_SOURCE_DIR}/src SRC_FILES)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/component SRC_FILES)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/service SRC_FILES)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/service/hash SRC_FILES)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/service/pke SRC_FILES)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/service/ske SRC_FILES)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/service/aead SRC_FILES)

aux_source_directory(${CMAKE_SOURCE_DIR}/src/driver SRC_FILES)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/schedule SRC_FILES)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/driver/otp/${OTP_DEVICE_TYPE}
                     SRC_FILES)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/driver/flash/${FLASH_DEVICE_TYPE}
                     SRC_FILES)

# crypto_lib files
set(CRYPTO_LIB_DIR ${CMAKE_SOURCE_DIR}/libs/crypto_lib/osr_crypto)
include_directories(${CRYPTO_LIB_DIR}/crypto_include)

if(NOT EXISTS ${CMAKE_SOURCE_DIR}/libs/lib${CRYPTOLIB}.a)
  aux_source_directory(${CRYPTO_LIB_DIR}/crypto_common CRYPTO_SRC_FILES)
  aux_source_directory(${CRYPTO_LIB_DIR}/crypto_hal CRYPTO_SRC_FILES)
  aux_source_directory(${CRYPTO_LIB_DIR}/crypto_lib/hash_hmac CRYPTO_SRC_FILES)
  aux_source_directory(${CRYPTO_LIB_DIR}/crypto_lib/pke CRYPTO_SRC_FILES)
  aux_source_directory(${CRYPTO_LIB_DIR}/crypto_lib/ske CRYPTO_SRC_FILES)
  aux_source_directory(${CRYPTO_LIB_DIR}/crypto_lib/trng CRYPTO_SRC_FILES)
  aux_source_directory(${CRYPTO_LIB_DIR}/crypto_lib CRYPTO_SRC_FILES)
  add_library(${CRYPTOLIB} ${CRYPTO_SRC_FILES})
endif()
