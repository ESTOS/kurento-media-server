if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWIN32_LEAN_AND_MEAN=1")
  set (WINSOCK_LIBRARIES libws2_32.a libmswsock.a librpcrt4.a libole32.a)
endif ()

set (WEBSOCKET_SOURCES
  WebSocketTransport.cpp
  WebSocketTransport.hpp
  WebSocketTransportFactory.cpp
  WebSocketTransportFactory.hpp
  WebSocketEventHandler.cpp
  WebSocketEventHandler.hpp
  WebSocketRegistrar.cpp
  WebSocketRegistrar.hpp
)

set(FLAGS "-D_WEBSOCKETPP_CPP11_STL_ -pthread")

set_source_files_properties(${WEBSOCKET_SOURCES}
  PROPERTIES
    COMPILE_FLAGS ${FLAGS}
)

add_library (websocketTransport STATIC
  ${WEBSOCKET_SOURCES}
)

target_link_libraries(websocketTransport
  ${GSTREAMER_LIBRARIES}
  ${KMSCORE_LIBRARIES}
  ${JSONRPC_LIBRARIES}
  ${OPENSSL_LIBRARIES}
  ${WINSOCK_LIBRARIES}
)

set_property (TARGET websocketTransport
  PROPERTY INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/server/transport
    ${CMAKE_SOURCE_DIR}/server/signalHandler
    ${JSONRPC_INCLUDE_DIRS}
    ${GSTREAMER_INCLUDE_DIRS}
    ${KMSCORE_INCLUDE_DIRS}
)
