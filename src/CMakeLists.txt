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

find_package(Threads REQUIRED)
if(NOT TARGET glog::glog)
  find_package(glog CONFIG REQUIRED)
endif()

add_library(libbase STATIC "")
add_library(libbase::libbase ALIAS libbase)

libbase_install_target(libbase)
libbase_configure_library_target(libbase)

target_compile_definitions(libbase
  PUBLIC
    ${LIBBASE_FEATURE_DEFINES}
)

set_target_properties(libbase PROPERTIES
  OUTPUT_NAME "${LIBBASE_OUTPUT_NAME}"
)

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
  if(NOT TARGET CURL::libcurl)
    find_package(CURL CONFIG REQUIRED)
  endif()

  add_library(libbase_net STATIC "")
  add_library(libbase::libbase_net ALIAS libbase_net)

  libbase_install_target(libbase_net)
  libbase_configure_library_target(libbase_net)

  target_compile_definitions(libbase_net
    PUBLIC
      ${LIBBASE_FEATURE_DEFINES}
    INTERFACE
      LIBBASE_MODULE_NET
  )

  set_target_properties(libbase_net
    PROPERTIES
      OUTPUT_NAME "${LIBBASE_OUTPUT_NAME}_net"
  )

  target_link_libraries(libbase_net
    PUBLIC
      libbase
    PRIVATE
      CURL::libcurl
  )

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
      base/net/url_request.cc
      base/net/url_request.h
  )
endif()


#
# Library integration module - win (WinApi)
#

if(LIBBASE_BUILD_MODULE_WIN)
  add_library(libbase_win STATIC "")
  add_library(libbase::libbase_win ALIAS libbase_win)

  libbase_install_target(libbase_win)
  libbase_configure_library_target(libbase_win)

  target_compile_definitions(libbase_win
    PUBLIC
      ${LIBBASE_FEATURE_DEFINES}
    INTERFACE
      LIBBASE_MODULE_WIN
  )

  set_target_properties(libbase_win
    PROPERTIES
      OUTPUT_NAME "${LIBBASE_OUTPUT_NAME}_win"
  )

  target_link_libraries(libbase_win
    PUBLIC
      libbase
  )

  target_sources(libbase_win
    PRIVATE
      base/message_loop/win/win_message_loop_attachment.cc
      base/message_loop/win/win_message_loop_attachment.h
  )
endif()


#
# Library integration module - wx (wxWidgets)
#

if(LIBBASE_BUILD_MODULE_WX)
  if(NOT TARGET wx::core OR NOT TARGET wx::base)
    find_package(wxWidgets CONFIG REQUIRED)
  endif()

  add_library(libbase_wx STATIC "")
  add_library(libbase::libbase_wx ALIAS libbase_wx)

  libbase_install_target(libbase_wx)
  libbase_configure_library_target(libbase_wx)

  target_compile_definitions(libbase_wx
    PUBLIC
      ${LIBBASE_FEATURE_DEFINES}
    INTERFACE
      LIBBASE_MODULE_WX
  )

  set_target_properties(libbase_wx
    PROPERTIES
      OUTPUT_NAME "${LIBBASE_OUTPUT_NAME}_wx"
  )

  target_link_libraries(libbase_wx
    PUBLIC
      libbase
      wx::core
      wx::base
  )

  target_sources(libbase_wx
    PRIVATE
      base/message_loop/wx/wx_message_loop_attachment.cc
      base/message_loop/wx/wx_message_loop_attachment.h
  )
endif()
