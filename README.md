## threx

Spawn threads and run tasks on those threads from JS. Simple example that does
nothing:

```javascript
var Thread = require('threx');

var t = new Thread();
t.spawn();
t.join();
```
