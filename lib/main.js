var buffer = new Buffer();
var threx = require('../build/Release/threx');
var Thread = threx.Thread;

Thread.prototype.spawn = threx.spawn;
Thread.prototype.join = threx.join;
Thread.prototype.enqueue = threx.enqueue;

Thread.prototype.enqueue = function enqueue(obj, data) {
  if (data instanceof buffer &&
      data.parent !== undefined)
    throw new Error('Buffer should not be from a slice');
  return threx.enqueue.call(this, obj, data);
};

module.exports = Thread;
