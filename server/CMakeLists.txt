if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWIN32_LEAN_AND_MEAN=1 -DWIN32=1")
else ()
  set (LIBRARIES dl)
endif ()

SET(KMSC_SOURCE
  main.cpp
  version.cpp
  version.hpp
  ServerMethods.cpp
  ServerMethods.hpp
  ResourceManager.cpp
  ResourceManager.hpp
  RequestCache.cpp
  RequestCache.hpp
  CacheEntry.cpp
  CacheEntry.hpp
  logging.cpp
  logging.hpp
  modules.cpp
  modules.hpp
  loadConfig.cpp
  loadConfig.hpp
  death_handler.cpp
  death_handler.hpp
)

add_definitions(-DBOOST_LOG_DYN_LINK)

add_executable(kurento-media-server ${KMSC_SOURCE})

add_dependencies(kurento-media-server transport)

target_link_libraries (kurento-media-server
  ${Boost_LIBRARIES}
  transport
  pthread
  ${LIBRARIES}
)

set_property (TARGET kurento-media-server
  PROPERTY INCLUDE_DIRECTORIES
    ${CMAKE_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/gen-cpp
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/transport
    ${KMSCORE_INCLUDE_DIRS}
)

install(TARGETS kurento-media-server RUNTIME DESTINATION bin)

add_subdirectory(transport)
