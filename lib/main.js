var threx = require('../build/Release/threx');

module.exports = threx.Thread;
module.exports.print_include = function() {
  console.log(require('path').relative('.', __dirname) + '../include');
};
