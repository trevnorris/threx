#include "../include/threx.h"
#include "../deps/fuq/fuq.h"

#include <v8.h>
#include <node.h>
#include <uv.h>

#include <assert.h>  /* assert */
#include <stdlib.h>  /* free, malloc */

#if __cplusplus <= 199711L
#define nullptr NULL
#endif

namespace threx {

using v8::Context;
using v8::External;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Handle;
using v8::HandleScope;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::String;
using v8::Value;


static void thread_routine(void* d) {
  thread_resource_t* tr = static_cast<thread_resource_t*>(d);

  for (;;) {
    while (!fuq_empty(&tr->queue)) {
      void* arg = fuq_dequeue(&tr->queue);
      if (nullptr == arg)
        continue;
      queue_work_t* qi = static_cast<queue_work_t*>(arg);
      qi->cb(tr, qi->data);
      free(arg);
    }
    if (!tr->active)
      break;
    uv_sem_wait(&tr->sem);
  }
}


// Thread constructor

void Thread(const FunctionCallbackInfo<Value>& args) {
  assert(args.IsConstructCall());
  args.Holder()->SetAlignedPointerInInternalField(0, nullptr);
}


void Spawn(const FunctionCallbackInfo<Value>& args) {
  Local<Object> self;
  thread_resource_t* tr;

  self = args.Holder();
  assert(0 < self->InternalFieldCount());
  assert(nullptr == self->GetAlignedPointerFromInternalField(0));

  tr = static_cast<thread_resource_t*>(malloc(sizeof(*tr)));
  assert(nullptr != tr);

  fuq_init(&tr->queue);
  assert(uv_sem_init(&tr->sem, 1) == 0);
  assert(uv_async_init(uv_default_loop(), &tr->keep_alive, nullptr) == 0);
  assert(uv_thread_create(&tr->thread, thread_routine, tr) == 0);
  tr->active = true;

  self->SetAlignedPointerInInternalField(0, tr);
  args.GetReturnValue().Set(self);
}


static void bring_down_thread(thread_resource_t* tr, void* data) {
  tr->active = false;
}


static void close_async(uv_handle_t* handle) {
  free(handle);
}


void Join(const FunctionCallbackInfo<Value>& args) {
  Local<Object> self;
  thread_resource_t* tr;
  void* tr_p;

  self = args.Holder();
  assert(0 < self->InternalFieldCount());

  tr_p = self->GetAlignedPointerFromInternalField(0);
  assert(nullptr != tr_p);
  tr = static_cast<thread_resource_t*>(tr_p);

  self->SetAlignedPointerInInternalField(0, nullptr);

  enqueue_work(tr, bring_down_thread, nullptr);

  assert(uv_thread_join(&tr->thread) == 0);
  uv_sem_destroy(&tr->sem);
  fuq_dispose(&tr->queue);
  uv_close(reinterpret_cast<uv_handle_t*>(&tr->keep_alive), close_async);
  args.GetReturnValue().Set(self);
}


void Enqueue(const FunctionCallbackInfo<Value>& args) {
  Local<Object> self;
  thread_resource_t* tr;
  void* tr_p;
  void* ext;

  self = args.Holder();
  assert(0 < self->InternalFieldCount());

  tr_p = self->GetAlignedPointerFromInternalField(0);
  assert(nullptr != tr_p);
  tr = static_cast<thread_resource_t*>(tr_p);

  assert(args[0]->IsExternal());
  ext = args[0].As<External>()->Value();
  assert(nullptr != ext);
  enqueue_work(tr, reinterpret_cast<thread_work_cb>(ext), nullptr);
}


void Init(Handle<Object> target) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);
  Local<FunctionTemplate> thread_t = FunctionTemplate::New(isolate, Thread);
  thread_t->SetClassName(String::NewFromUtf8(isolate, "Thread"));
  thread_t->InstanceTemplate()->SetInternalFieldCount(1);

  NODE_SET_PROTOTYPE_METHOD(thread_t, "spawn", Spawn);
  NODE_SET_PROTOTYPE_METHOD(thread_t, "join", Join);
  NODE_SET_PROTOTYPE_METHOD(thread_t, "enqueue", Enqueue);

  target->Set(String::NewFromUtf8(isolate, "Thread"),
              thread_t->GetFunction());
}

}  // namespace threx

NODE_MODULE(threx, threx::Init)
