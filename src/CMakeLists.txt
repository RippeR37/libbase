add_library(libbase STATIC "")

target_compile_features(libbase PUBLIC cxx_std_17)
target_compile_options(libbase PRIVATE ${LIBBASE_COMPILE_FLAGS})
target_compile_definitions(libbase PUBLIC ${LIBBASE_DEFINES})
set_target_properties(libbase PROPERTIES
  CXX_EXTENSIONS OFF
  ${LIBBASE_OPT_CLANG_TIDY_PROPERTIES})

target_include_directories(libbase PUBLIC ${libbase_SOURCE_DIR}/src/)
target_link_libraries(libbase PUBLIC
  ${LIBBASE_LINK_FLAGS}
  Threads::Threads
  glog::glog)

target_sources(libbase
  PRIVATE
    base/auto_reset.h
    base/bind.h
    base/bind_internals.h
    base/callback.h
    base/callback_forward.h
    base/callback_helpers.cc
    base/callback_helpers.h
    base/callback_iface.h
    base/callback_internals.h
    base/logging.h
    base/memory/weak_ptr.h
    base/message_loop/message_loop.h
    base/message_loop/message_loop_impl.cc
    base/message_loop/message_loop_impl.h
    base/message_loop/message_pump.h
    base/message_loop/message_pump_impl.cc
    base/message_loop/message_pump_impl.h
    base/sequence_checker.cc
    base/sequence_checker.h
    base/sequence_id.cc
    base/sequence_id.h
    base/sequenced_task_runner.h
    base/sequenced_task_runner_helpers.cc
    base/sequenced_task_runner_helpers.h
    base/sequenced_task_runner_internals.h
    base/single_thread_task_runner.h
    base/source_location.h
    base/synchronization/auto_signaller.cc
    base/synchronization/auto_signaller.h
    base/synchronization/waitable_event.cc
    base/synchronization/waitable_event.h
    base/task_runner.cc
    base/task_runner.h
    base/task_runner_internals.h
    base/threading/sequenced_task_runner_handle.cc
    base/threading/sequenced_task_runner_handle.h
    base/threading/task_runner_impl.cc
    base/threading/task_runner_impl.h
    base/threading/thread.cc
    base/threading/thread.h
    base/threading/thread_pool.cc
    base/threading/thread_pool.h
    base/time/time.cc
    base/time/time.h
    base/time/time_delta.cc
    base/time/time_delta.h
    base/type_traits.h
    base/vlog_is_on.h
)
