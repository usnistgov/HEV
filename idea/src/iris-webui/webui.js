function Queue() {
  var queue = [],
      offset = 0;

  this.getLength = function() { return queue.length - offset; };
  this.isEmpty = function() { return (queue.length == 0); };
  this.peek = function() { return (queue.length > 0) ? queue[offset] : undefined; }
  this.enqueue = function(item) { queue.push(item); }
  this.dequeue = function() {
    if (queue.length == 0) return undefined;
    var item = queue[offset];
    if (++offset * 2 >= queue.length) {
      queue = queue.slice(offset);
      offset = 0;
    }
    return item;
  };
}

function hex2rgb(hex) {
  var result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(
    hex.replace(/^#?([a-f\d])([a-f\d])([a-f\d])$/i, function(m,r,g,b) { return r+r+g+g+b+b; }));

  return result ? {
    r: parseInt(result[1], 16)/255.0,
    g: parseInt(result[2], 16)/255.0,
    b: parseInt(result[3], 16)/255.0
  } : null;
}

function clean(s) {
  return s.trim().replace(/\s+/g, " ");
}

function SocketFactory(socketFactory) {
  var socket = socketFactory();
  socket.forward('notify');
  socket.forward('response');
  return socket;
}

function HevScenegraph() {
  var width = 400,
      height = 500;

  return {
    restrict: 'E',
    link: function(scope, element, attrs) {
      var tree = d3.layout.tree().size([height, width-160]),
          diagonal = d3.svg.diagonal().projection(function(d) { return [d.y,d.x]; }),
          vis = d3.select(element[0]).append('svg').attr('width', width).attr('height', height)
            .append('g').attr('transform', "translate(40,0)");

      scope.$on('scenegraph:update', function() {
        vis.selectAll('*').remove(); // clear all elements inside directive

        var nodes = tree.nodes(scope.scene),
            links = tree.links(nodes),
            link = vis.selectAll('path.link').data(links).enter()
              .append('path').attr('class', "link").attr('d', diagonal),
            node = vis.selectAll('g.node').data(nodes).enter()
              .append('g').attr('class', "node")
              .attr('transform', function(d) { return "translate(" + d.y + "," + d.x + ")"; });

        node.append('circle').attr('r', 4.5);
        node.append('text')
          .attr('dx', function(d) { return d.children ? -8 : 8; })
          .attr('dy', 3)
          .attr('text-anchor', function(d) { return d.children ? "end" : "start"; })
          .text(function(d) { return d.name; });
      });
    }
  }
}

function IrisController($scope, socket) {
  $scope.sendControl = function(message) { socket.emit('control', message); };

  $scope.notify = "";
  $scope.response = "";
  $scope.scene = { name: "world", children: [] };
  $scope.echo = false;
  $scope.background = "#000000";

  $scope.$on('socket:notify', function(ev, data) {
    $scope.$apply(function() { $scope.notify = data; });
  });

  $scope.$on('socket:response', function(ev, data) {
    var split = clean(data).split(" ");
    if (split[0].toUpperCase() == "CHILDREN") {
      var parentName = clean(split[1]).replace(/'/g, "");
      split = split.slice(3, split.length);

      split.forEach(function(str, ind, arr) {
        str = clean(str).replace(/'/g, "");
        if (str === "") return;
        var childNode = { name: str, children: [] };

        $scope.$apply(function() {
          if (parentName === "world") {
            if (!angular.isArray($scope.scene.children)) $scope.scene.children = [];
            $scope.scene.children.push(childNode);
            $scope.sendControl("QUERY CHILDREN " + childNode.name);
          } else {
            var nodesQueue = new Queue();
            $scope.scene.children.forEach(function(child, ind, arr) { nodesQueue.enqueue(child); });

            while (!nodesQueue.isEmpty()) {
              var node = nodesQueue.dequeue();
              if (!node) break;

              if (parentName === node.name) {
                if (!angular.isArray(node.children)) node.children = [];
                node.children.push(childNode);
                $scope.sendControl("QUERY CHILDREN " + childNode.name);
              } else {
                if (angular.isArray(node.children)) {
                  node.children.forEach(function(child, ind, arr) { nodesQueue.enqueue(child); });
                }
              }
            }
          }
        });

        $scope.$broadcast('scenegraph:update');
      });
    } else {
      $scope.$apply(function() { $scope.response = data; });
    }
  });

  $scope.updateEcho = function(newEcho) {
    $scope.sendControl("ECHO " + (newEcho ? "ON" : "OFF"));
  };

  $scope.updateBackground = function(newBackground) {
    var bg = hex2rgb(newBackground);
    $scope.sendControl("BACKGROUND " + bg.r + " " + bg.g + " " + bg.b);
  };

  $scope.sendControl("QUERY CHILDREN world");
}

angular
  .module('hev-webui', ['btford.socket-io','ui.bootstrap'])
  .factory('socket', SocketFactory)
  .controller('IrisController', IrisController)
  .directive('hevScenegraph', HevScenegraph);
