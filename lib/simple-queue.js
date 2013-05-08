module.exports = SimpleQueue;

function SimpleQueue() {
  var self = this;
  
  self.fifo = [];
  self.executing = false;
}

SimpleQueue.prototype.push = function (fn) {
  var self = this;
  
  self.fifo.push(fn);
  
  self.maybeNext();
};

SimpleQueue.prototype.maybeNext = function () {
  var self = this;
  
  if (!self.executing) {
    self.next();
  }
};

SimpleQueue.prototype.next = function () {
  var self = this;
  
  if (self.fifo.length) {
    var fn = self.fifo.shift();
    
    self.executing = true;
    
    fn(function () {
      self.executing = false;
      
      self.maybeNext();
    });
  }
};