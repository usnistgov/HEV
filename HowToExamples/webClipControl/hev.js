var urlParams;

(window.onpopstate = function () {
  var match,
      pl     = /\+/g,  // Regex for replacing addition symbol with a space
      search = /([^&=]+)=?([^&]*)/g,
      decode = function (s) { return decodeURIComponent(s.replace(pl, " ")); },
      query  = window.location.search.substring(1);
  urlParams = {};
  while (match = search.exec(query)) urlParams[decode(match[1])] = decode(match[2]);
})();

$(function() {
  var socket;

  var currClip = "";
  var currStep = 0.01;
  var currX = 0, currY = 0, currZ = 0;
  var clipSCS = {
    xy: [ 0, 0, 0, 0, 90, 0 ],
    yz: [ 0, 0, 0, 90, 0, 0 ],
    zx: [ 0, 0, 0, 0, 0, 90 ]
  }

  setStatusString = function(statusString, fadeOutMS) {
    $("#status").text(statusString);
    if (arguments.length == 2) $("#status").fadeOut(fadeOutMS);
  }

  sendControlMessage = function(messageString) {
    if (typeof(socket) == "undefined") {
      console.log("control: ["+messageString+"]");
      return;
    }
    socket.emit("control", messageString);
  }

  updateSCS = function(n) {
    var t = clipSCS[n];
    sendControlMessage("SCS "+n+"SCS "+t[0]+" "+t[1]+" "+t[2]+" "+t[3]+" "+t[4]+" "+t[5]);
  }

  initClipPlane = function(clipName) {
    sendControlMessage("CLIPNODE "+clipName);
    updateSCS(clipName);
    sendControlMessage("ADDCHILD "+clipName+" "+clipName+"SCS");
    sendControlMessage("ADDCHILD "+clipName+"SCS world");
    sendControlMessage("CLIPNODE "+clipName+" OFF");
  }

  toggleClipPlane = function(clipName) {
    if (!(currClip === "")) {
      sendControlMessage("CLIPNODE "+currClip+" OFF");
      currClip = "";
    }

    if (clipName === "none") {
      $("#x").slider("disable");
      $("#y").slider("disable");
      $("#z").slider("disable");
      return;
    }

    sendControlMessage("CLIPNODE "+clipName+" ON");
    currClip = clipName;

    if (currClip === "xy") {
      $("#x").slider("disable");
      $("#y").slider("disable");
      $("#z").slider("enable");
    } else if (currClip === "yz") {
      $("#x").slider("enable");
      $("#y").slider("disable");
      $("#z").slider("disable");
    } else if (currClip === "zx") {
      $("#x").slider("disable");
      $("#y").slider("enable");
      $("#z").slider("disable");
    }
  }

  translateClipPlane = function(slider) {
    if (currClip === "") return;

    var axis = slider.attr("id").toLowerCase();
    if (axis === "x") currX = clipSCS[currClip][0] = parseFloat(slider.val());
    else if (axis === "y") currY = clipSCS[currClip][1] = parseFloat(slider.val());
    else if (axis === "z") currZ = clipSCS[currClip][2] = parseFloat(slider.val());

    updateSCS(currClip);
  }

  initSpinner = function(spinner, value) {
    spinner.attr("min", -1e6);
    spinner.attr("max", 1e6);
    spinner.attr("value", value);
  }

  initSlider = function(slider, minSlider, maxSlider, value) {
    var opts = {
      min: parseFloat($(minSlider).val()),
      max: parseFloat($(maxSlider).val()),
      value: value,
      step: currStep,
      tooltip: "hide",
      selection: "none",
      enabled: false
    };

    slider.slider(opts);
    slider.on("slide", function(event) { translateClipPlane($(this)); });

    minSlider.on("change", function(event) { updateSlider(slider, "min", parseFloat($(this).val())); });
    maxSlider.on("change", function(event) { updateSlider(slider, "max", parseFloat($(this).val())); });
  }

  updateSlider = function(slider, attribute, value) {
    slider.slider("setAttribute", attribute, value);

    var axis = slider.attr("id").toLowerCase();
    if (axis === "x") slider.slider("setValue", currX, false, false);
    else if (axis === "y") slider.slider("setValue", currY, false, false);
    else if (axis === "z") slider.slider("setValue", currZ, false, false);
  }


  if (!(typeof(io) === "undefined")) { socket = io.connect("http://localhost"); }
  //sendControlMessage("ECHO ON");

  initClipPlane("xy");
  initClipPlane("yz");
  initClipPlane("zx");

  $("form").on("submit", function(event) { event.preventDefault(); });
  $("input.clipPlanes").on("click", function(event) { toggleClipPlane($(this).val()); });

  var dataMin = [-2, -2, -2];
  var dataMax = [ 2,  2,  2];

  if ("dataMin" in urlParams) {
    dataMin.forEach(function(cv, i, a) { a[i] = parseFloat(urlParams["dataMin"]); });
  }
  if ("dataMax" in urlParams) {
    dataMax.forEach(function(cv, i, a) { a[i] = parseFloat(urlParams["dataMax"]); });
  }
  
  if ("XMin" in urlParams) dataMin[0] = parseFloat(urlParams["XMin"]);
  if ("XMax" in urlParams) dataMax[0] = parseFloat(urlParams["XMax"]);
  if ("YMin" in urlParams) dataMin[1] = parseFloat(urlParams["YMin"]);
  if ("YMax" in urlParams) dataMax[1] = parseFloat(urlParams["YMax"]);
  if ("ZMin" in urlParams) dataMin[2] = parseFloat(urlParams["ZMin"]);
  if ("ZMax" in urlParams) dataMax[2] = parseFloat(urlParams["ZMax"]);

  if ("step" in urlParams) currStep = parseFloat(urlParams["step"]);

  initSpinner($("#Xmin"), dataMin[0]);
  initSpinner($("#Xmax"), dataMax[0]);
  initSpinner($("#Ymin"), dataMin[1]);
  initSpinner($("#Ymax"), dataMax[1]);
  initSpinner($("#Zmin"), dataMin[2]);
  initSpinner($("#Zmax"), dataMax[2]);

  initSlider($("#x"), $("#Xmin"), $("#Xmax"), currX);
  initSlider($("#y"), $("#Ymin"), $("#Ymax"), currY);
  initSlider($("#z"), $("#Zmin"), $("#Zmax"), currZ);

  setStatusString("HEV ready...", 2000);
});

