#ifndef INCLUDE_THREX_H_
#define INCLUDE_THREX_H_

#include "../deps/fuq/fuq.h"
#include <uv.h>
#include <v8.h>

namespace threx {

typedef struct {
  /* private */
  // uv_async_t must be first
  uv_async_t keep_alive;  // keep event loop alive while thread is running.
  uv_thread_t thread;
  uv_sem_t sem;
  fuq_queue_t queue;
  bool active;

  /* public */
  void* data;
} thread_resource_t;

typedef void (*thread_work_cb)(thread_resource_t* tr);

static inline v8::Local<v8::External> export_work(v8::Isolate* isolate,
                                                  thread_work_cb cb) {
  v8::EscapableHandleScope scope(isolate);
  return scope.Escape(v8::External::New(isolate, reinterpret_cast<void*>(cb)));
}

static inline void enqueue_work(thread_resource_t* tr, thread_work_cb cb) {
  fuq_enqueue(&tr->queue, reinterpret_cast<void*>(cb));
  uv_sem_post(&tr->sem);
}

}  // namespace threx
#endif  // INCLUDE_THREX_H_
