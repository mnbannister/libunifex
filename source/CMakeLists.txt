# Copyright (c) 2019-present, Facebook, Inc.
#
# This source code is licensed under the license found in the
# LICENSE.txt file in the root directory of this source tree.

add_library(unifex "")
set_target_properties(unifex PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR})

target_sources(unifex
  PRIVATE
    async_mutex.cpp
    exception.cpp
    inplace_stop_token.cpp
    manual_event_loop.cpp
    static_thread_pool.cpp
    thread_unsafe_event_loop.cpp
    timed_single_thread_context.cpp
    trampoline_scheduler.cpp
    async_manual_reset_event.cpp)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_sources(unifex
    PRIVATE
      linux/mmap_region.cpp
      linux/monotonic_clock.cpp
      linux/safe_file_descriptor.cpp
      linux/io_epoll_context.cpp)

  target_link_libraries(unifex
    PUBLIC
      pthread)

if (NOT UNIFEX_NO_LIBURING)

  target_sources(unifex
    PRIVATE
      linux/io_uring_context.cpp
      linux/io_uring_syscall.cpp)

  target_include_directories(unifex
    PUBLIC
      ${UNIFEX_URING_INCLUDE_DIRS})

  target_link_libraries(unifex
    PRIVATE
      ${UNIFEX_URING_LIBRARY})

endif()

endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")

  target_sources(unifex
    PRIVATE
      win32/filetime_clock.cpp
      win32/safe_handle.cpp
      win32/windows_thread_pool.cpp
      win32/low_latency_iocp_context.cpp
      win32/ntapi.cpp)

endif()

configure_file(
  ../include/unifex/config.hpp.in
  "${PROJECT_BINARY_DIR}/include/unifex/config.hpp")

include(GNUInstallDirs)
target_include_directories(unifex
    PUBLIC
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/>
      $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include/>
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_compile_features(unifex PUBLIC cxx_std_17)

if(CXX_COROUTINES_HAVE_COROUTINES)
  target_link_libraries(unifex PUBLIC std::coroutines)
endif()

configure_file(unifex.pc.in unifex.pc @ONLY)

# install unifex with its configure file into the default paths

install(TARGETS unifex EXPORT unifexTargets)
install(FILES ${PROJECT_BINARY_DIR}/include/unifex/config.hpp
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/unifex/)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/
        TYPE INCLUDE
        PATTERN "*.in" EXCLUDE)
install(EXPORT unifexTargets
        FILE unifexConfig.cmake
        NAMESPACE unifex::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/unifex/)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/unifex.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
