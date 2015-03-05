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
$ npm instal threx
```

To install using io.js do:

```
$ iojs $(which npm) install threx --nodedir=<path_to_source>
```


### API

The API is still pretty scarce, but this is what we've got:

#### JS

`Thread()`: Constructor to create a new thread

`Thread#spawn()`: Spawn the thread. Only run this _once_.

`Thread#join()`: Join the thread and cleanup resources. Blocking call.

`Thread#enqueue(obj)`: Pass a native method to be run on the spawned thread.
This is meant to be used in conjunction with the native API.

#### C++

`thread_resource_t`: `struct` of the thread resource attached to the `Thread()`
instance.

`thread_work_cb`: `typedef` of the type of callback that is called from the
spawned thread.

`export_work()`: Pass it a callback of type `thread_work_cb` and it will return
a `Local<External>` that you can return to JS and pass to `Thread#enqueue()`.

`enqueue_work()`: Pass it a `thread_resource_t*` and a `thread_work_cb` to have
it queued to be run from the spawned thread.


### Examples

Coming soon... Examples of how to use both the JS and native API to do awesome
stuff!
