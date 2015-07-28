#ifndef INCLUDE_THREX_H_
#define INCLUDE_THREX_H_

#if _MSC_VER < 1600 //MSVC version < 8
     #include "../src/nullptr/_nullptr.h"
#endif
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
  fuq_queue_t async_queue;
  bool active;

  /* public */
  void* data;
} thread_resource_t;

typedef void (*thread_work_cb)(thread_resource_t* tr, void* data, size_t size);

typedef struct {
  thread_work_cb cb;
  void* data;
  size_t size;
} queue_work_t;

static inline v8::Local<v8::External> export_work(v8::Isolate* isolate,
                                                  thread_work_cb cb) {
  v8::EscapableHandleScope scope(isolate);
  return scope.Escape(v8::External::New(isolate, reinterpret_cast<void*>(cb)));
}

static inline void enqueue_work(thread_resource_t* tr,
                                thread_work_cb cb,
                                void* data,
                                size_t size = 0) {
  queue_work_t* qi;

  qi = static_cast<queue_work_t*>(malloc(sizeof(*qi)));
  assert(NULL != qi);
  qi->cb = cb;
  qi->data = data;
  qi->size = size;

  fuq_enqueue(&tr->queue, qi);
  uv_sem_post(&tr->sem);
}

static inline void enqueue_cb(thread_resource_t* tr,
                              thread_work_cb cb,
                              void* data,
                              size_t size = 0) {
  bool is_empty = fuq_empty(&tr->async_queue);
  queue_work_t* qi;

  qi = static_cast<queue_work_t*>(malloc(sizeof(*qi)));
  assert(NULL != qi);
  qi->cb = cb;
  qi->data = data;
  qi->size = size;

  fuq_enqueue(&tr->async_queue, qi);
  if (is_empty)
    uv_async_send(&tr->keep_alive);
}

static inline thread_resource_t* get_thread_resource(v8::Local<v8::Object> o) {
  void* ptr;
  assert(0 < o->InternalFieldCount());
  ptr = o->GetAlignedPointerFromInternalField(0);
  assert(nullptr != ptr);
  return static_cast<thread_resource_t*>(ptr);
}

}  // namespace threx
#endif  // INCLUDE_THREX_H_
