## threx

Spawn threads and run tasks on those threads from JS. Simple example that does
nothing:

```javascript
var Thread = require('threx');

var t = new Thread();
t.spawn();
t.join();
```


### Install

Only works with Node.js v0.12.x or io.js. To install using Node do:

```
$ npm install threx
```

To install using io.js do:

```
$ iojs $(which npm) install threx --nodedir=<path_to_source>
```

To use in your project include this in your `binding.gyp` under `"targets"`:

```
"include_dirs": [
  "node_modules/threx/include"
]
```

And then put `#include <threx.h>` at the top of your C++ file.


### API

The API is still pretty scarce, but this is what we've got:

#### JS

`Thread()`: Constructor to create a new thread

`Thread#spawn()`: Spawn the thread. Only run this _once_.

`Thread#join()`: Join the thread and cleanup resources. Blocking call.

`Thread#enqueue(obj)`: Pass a native method to be run on the spawned thread.
This is meant to be used in conjunction with the native API.

#### C++

Everything is namespaced under `threx`.

`thread_resource_t`: `struct` of the thread resource attached to the `Thread()`
instance.

`thread_work_cb`: `typedef` of the type of callback that is called from the
spawned thread.

`export_work(v8::Isolate*, thread_work_cb)`: Pass it a callback of type
`thread_work_cb` and it will return a `Local<External>` that you can return to
JS and pass to `Thread#enqueue()`.

`enqueue_work(thread_resource_t*, thread_work_cb, void* data)`: Pass it a
`thread_resource_t*` and a `thread_work_cb` to have it queued to be run from
the spawned thread.


### Examples

Here's a multi-threaded "hello world!"

```cpp
// main.cc
#include <v8.h>
#include <node.h>
#include <threx.h>

using v8::FunctionCallbackInfo;
using v8::Handle;
using v8::Isolate;
using v8::Object;
using v8::Value;

using threx::thread_resource_t;
using threx::export_work;


static void test_cb(thread_resource_t* tr) {
  fprintf(stderr, "hello world\n");
}


void RunMe(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  args.GetReturnValue().Set(export_work(isolate, test_cb));
}


void init(Handle<Object> exports) {
  NODE_SET_METHOD(exports, "runMe", RunMe);
}

NODE_MODULE(addon, init)
```

```javascript
// run.js
var Thread = require('threx');
var runMe = require('./build/Release/addon').runMe;

var t = new Thread();
t.spawn();
t.enqueue(runMe());
t.join();
```

```gyp
# binding.gyp
{
  "targets": [{
    "target_name": "addon",
    "sources": [ "main.cc" ],
    "include_dirs": [
      "node_modules/threx/include"
    ]
  }]
}
```
