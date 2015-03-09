var threx = require('../build/Release/threx');
var Thread = threx.Thread;

Thread.prototype.spawn = threx.spawn;
Thread.prototype.join = threx.join;
Thread.prototype.enqueue = threx.enqueue;

module.exports = Thread;
