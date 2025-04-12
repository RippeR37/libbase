#
# Features
#

option(LIBBASE_FEATURE_TRACING "Enable tracing (trace event macros)." ON)

set(LIBBASE_FEATURE_DEFINES "")
if (LIBBASE_FEATURE_TRACING)
  string(APPEND LIBBASE_FEATURE_DEFINES "LIBBASE_ENABLE_TRACING")
endif()

#
# Library
#

add_library(libbase STATIC "")

target_compile_features(libbase PUBLIC cxx_std_17)
target_compile_options(libbase PRIVATE ${LIBBASE_COMPILE_FLAGS})
target_compile_definitions(libbase PUBLIC
  ${LIBBASE_DEFINES}
  ${LIBBASE_FEATURE_DEFINES})
set_target_properties(libbase PROPERTIES
  CXX_EXTENSIONS ON
  ${LIBBASE_OPT_CLANG_TIDY_PROPERTIES})

target_include_directories(libbase PUBLIC
    $<BUILD_INTERFACE:${libbase_SOURCE_DIR}/src/>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/libbase>
)

target_link_libraries(libbase PUBLIC
  ${LIBBASE_LINK_FLAGS}
  Threads::Threads
  glog::glog)

target_sources(libbase
  PRIVATE
    base/auto_reset.h
    base/barrier_callback.h
    base/barrier_closure.cc
    base/barrier_closure.h
    base/bind_internals.h
    base/bind_post_task.h
    base/bind.h
    base/callback_forward.h
    base/callback_helpers.cc
    base/callback_helpers.h
    base/callback_iface.h
    base/callback_internals.h
    base/callback.h
    base/init.cc
    base/init.h
    base/logging.cc
    base/logging.h
    base/memory/weak_ptr.h
    base/message_loop/message_loop_impl.cc
    base/message_loop/message_loop_impl.h
    base/message_loop/message_loop.h
    base/message_loop/message_pump_impl.cc
    base/message_loop/message_pump_impl.h
    base/message_loop/message_pump.h
    base/message_loop/run_loop.cc
    base/message_loop/run_loop.h
    base/sequence_checker.cc
    base/sequence_checker.h
    base/sequence_id.cc
    base/sequence_id.h
    base/sequenced_task_runner_helpers.cc
    base/sequenced_task_runner_helpers.h
    base/sequenced_task_runner_internals.h
    base/sequenced_task_runner.h
    base/single_thread_task_runner.h
    base/source_location.h
    base/synchronization/auto_signaller.cc
    base/synchronization/auto_signaller.h
    base/synchronization/waitable_event.cc
    base/synchronization/waitable_event.h
    base/task_runner_internals.h
    base/task_runner.cc
    base/task_runner.h
    base/threading/delayed_task_manager_shared_instance.cc
    base/threading/delayed_task_manager_shared_instance.h
    base/threading/delayed_task_manager.cc
    base/threading/delayed_task_manager.h
    base/threading/sequenced_task_runner_handle.cc
    base/threading/sequenced_task_runner_handle.h
    base/threading/task_runner_impl.cc
    base/threading/task_runner_impl.h
    base/threading/thread_pool.cc
    base/threading/thread_pool.h
    base/threading/thread.cc
    base/threading/thread.h
    base/time/time_delta.cc
    base/time/time_delta.h
    base/time/time_ticks.cc
    base/time/time_ticks.h
    base/time/time.cc
    base/time/time.h
    base/timer/elapsed_timer.cc
    base/timer/elapsed_timer.h
    base/trace_event/trace_argument_packer.h
    base/trace_event/trace_async.h
    base/trace_event/trace_complete.h
    base/trace_event/trace_counter.h
    base/trace_event/trace_event_register.h
    base/trace_event/trace_event.h
    base/trace_event/trace_events.cc
    base/trace_event/trace_events.h
    base/trace_event/trace_flow.h
    base/trace_event/trace_flush.h
    base/trace_event/trace_json_writer.h
    base/trace_event/trace_metadata.h
    base/trace_event/trace_platform.cc
    base/trace_event/trace_platform.h
    base/trace_event/trace_signal.h
    base/type_traits.h
    base/vlog_is_on.h
)

#
# Library module - net
#

if(LIBBASE_BUILD_MODULE_NET)
  find_package(CURL CONFIG REQUIRED)

  add_library(libbase_net STATIC "")

  target_compile_features(libbase_net PUBLIC cxx_std_17)
  target_compile_options(libbase_net PRIVATE ${LIBBASE_COMPILE_FLAGS})
  target_compile_definitions(libbase_net PUBLIC
    ${LIBBASE_DEFINES}
    ${LIBBASE_FEATURE_DEFINES})
  target_compile_definitions(libbase_net
      INTERFACE LIBBASE_MODULE_NET
  )
  set_target_properties(libbase_net PROPERTIES
    CXX_EXTENSIONS ON
    ${LIBBASE_OPT_CLANG_TIDY_PROPERTIES})

  target_include_directories(libbase_net PUBLIC
      $<BUILD_INTERFACE:${libbase_SOURCE_DIR}/src/>
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/libbase>
  )

  target_link_libraries(libbase_net PUBLIC
    ${LIBBASE_LINK_FLAGS}
    libbase
    Threads::Threads
    glog::glog
    CURL::libcurl)

  target_sources(libbase_net
    PRIVATE
      base/net/impl/net_thread_impl.cc
      base/net/impl/net_thread_impl.h
      base/net/impl/net_thread.cc
      base/net/impl/net_thread.h
      base/net/init.cc
      base/net/init.h
      base/net/request_cancellation_token.cc
      base/net/request_cancellation_token.h
      base/net/resource_request.h
      base/net/resource_response.h
      base/net/result.h
      base/net/simple_url_loader.cc
      base/net/simple_url_loader.h
  )
endif()


#
# Library integration module - wx (wxWidgets)
#

if(LIBBASE_BUILD_MODULE_WX)
  find_package(wxWidgets CONFIG REQUIRED)

  add_library(libbase_wx STATIC "")

  target_compile_features(libbase_wx PUBLIC cxx_std_17)
  target_compile_options(libbase_wx PRIVATE ${LIBBASE_COMPILE_FLAGS})
  target_compile_definitions(libbase_wx PUBLIC
    ${LIBBASE_DEFINES}
    ${LIBBASE_FEATURE_DEFINES})
  target_compile_definitions(libbase_wx
      INTERFACE LIBBASE_MODULE_WX
  )
  set_target_properties(libbase_wx PROPERTIES
    CXX_EXTENSIONS ON
    ${LIBBASE_OPT_CLANG_TIDY_PROPERTIES})

  target_include_directories(libbase_wx PUBLIC
      $<BUILD_INTERFACE:${libbase_SOURCE_DIR}/src/>
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/libbase>
  )

  target_link_libraries(libbase_wx PUBLIC
    ${LIBBASE_LINK_FLAGS}
    libbase
    Threads::Threads
    glog::glog
    wx::core
    wx::base)

  target_sources(libbase_wx
    PRIVATE
      base/message_loop/wx/wx_message_loop_attachment.cc
      base/message_loop/wx/wx_message_loop_attachment.h
  )
endif()
