#ifndef INCLUDE_THREX_H_
#define INCLUDE_THREX_H_

#include "../deps/fuq/fuq.h"
#include <uv.h>
#include <v8.h>

#include <assert.h>  /* assert */
#include <stdlib.h>  /* malloc */

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

typedef void (*thread_work_cb)(thread_resource_t* tr, void* data);

typedef struct {
  thread_work_cb cb;
  void* data;
} queue_work_t;

static inline v8::Local<v8::External> export_work(v8::Isolate* isolate,
                                                  thread_work_cb cb) {
  v8::EscapableHandleScope scope(isolate);
  return scope.Escape(v8::External::New(isolate, reinterpret_cast<void*>(cb)));
}

static inline void enqueue_work(thread_resource_t* tr,
                                thread_work_cb cb,
                                void* data) {
  queue_work_t* qi;

  qi = static_cast<queue_work_t*>(malloc(sizeof(*qi)));
  assert(NULL != qi);
  qi->cb = cb;
  qi->data = data;

  fuq_enqueue(&tr->queue, qi);
  uv_sem_post(&tr->sem);
}

}  // namespace threx
#endif  // INCLUDE_THREX_H_
