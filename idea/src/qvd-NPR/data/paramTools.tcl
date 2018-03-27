# GUI for controlling shader parameters
#
# procedures intended for end-user use:
#   paramHeader .path shaderNode
#   paramFloat .path label value min max
#   paramInt   .path label value min max
#   paramRGB   .path label red green blue
#   paramRGBA  .path label red green blue alpha
#
# .path is a tk widget path (usually in the form .x.y.z), where the
# final component is the name of a shader variable. Earlier components
# are the names of nested frames that lay out the GUI
#
# shaderNode is the name of the scenegraph node for reloading shaders
#
# label is a text label to display in the GUI
#
# value is the initial value for a float or integer parameter
# should be between min and max
#
# red, green, blue and alpha are the initial values for a color
# parameter. Should be between 0 and 1


# global variables
set tmp /tmp/$env(USER).[pid].iris;	# unique temp file name
set continuous 0;			# disable continuous updates

# add commands to remove temp file
if {[info exists env(QVTONPR_CLEANUP)]} {
    exec echo rm -f $tmp >> $env(QVTONPR_CLEANUP)
}

# create namespace for parameters
namespace eval p {}


# create header with toggle for slider mode and reloading shaders
# argument:
#   widget = tk name for header panel (e.g. .header)
proc paramHeader {widget shaderNode} {
    global continuous

    frame $widget

    # continuous update checkbox
    label $widget.label -text "Continuous slider updates?"
    checkbutton $widget.check -variable continuous

    # reload button
    button $widget.reload -text "Reload Shaders" \
	-command [list puts "GROUP shader
SHADERPROG shader V qvdNPR.vert
SHADERPROG shader F qvdNPR.frag
MERGESTATE shader ${shaderNode}_root
UNLOAD shader"]

    # build header
    grid $widget.label $widget.check $widget.reload
    grid $widget.reload -sticky e
    grid columnconfigure $widget 2 -weight 1

    return $widget
}


# send a float or vector shader variable change to the running program
proc shaderParam {param} {
    # get local name for parameter variable and tmp file
    upvar \#0 p::$param var
    global tmp

    exec echo UNIFORM world $param float [llength $var] $var > $tmp
    puts "LOAD $tmp"
}

# create a shader parameter slider for a single float or integer parameter
# horizontal layout for label, value & slider
# arguments:
#   widget = tcl widget path, last element = shader parameter name
#   text = text label for slider
#   val = initial value
#   min, max = range for slider
#   digits = digits needed for value display
#   res = slider resolution
proc paramScalar {widget text val min max digits res} {
    # strip parameter as final component of widget name
    regexp {[^.]+$} $widget param

    set ::p::$param $val;		# set default value

    # create label, value and slider
    frame $widget
    label $widget.label -text $text
    label $widget.value -width $digits
    scale $widget.scale -orient h -showvalue false -length 400 \
	-from $min -to $max -resolution $res -variable ::p::$param \
	-command [list scalarUpdate $widget $param $digits]
    # if not "continuous", update on mouse button release
    bind $widget.scale <ButtonRelease> [list shaderParam $param]

    # lay out with label left justified, then value and slider
    grid $widget.label $widget.value
    grid $widget.label -sticky w
    grid columnconfigure $widget 0 -weight 1
    grid $widget.scale -

    return $widget
}
proc scalarUpdate {widget param digits val} {
    global continuous;			# access global behavior toggle

    # print padded value
    $widget.value configure -text [format "%*s" $digits $val]

    if {$continuous} {shaderParam $param}
}

# compute the precision settings for a slider
# arguments:
#   min, max = range for slider
#   width = pixel width of slider
# return {digits resolution}
#   digits = text characters necessary to display
#   resolution = increment between slider values
proc computePrecision {min max width} {
    # compute resolution based on single pixel step 
    # rounded to nearest power of ten (1, .1, .01, etc.)
    set decimals [expr round(log10(double($max-$min)/$width))]
    if {$decimals > 0} {set decimals 0}
    set resolution [expr pow(10,$decimals)]

    # compute digits for integer portion of min or max, including sign
    set a [expr ceil($min>=1 ? log10($min) : $min<=-1 ? log10(-$min)+1 : 0)]
    set b [expr ceil($max>=1 ? log10($max) : $max<=-1 ? log10(-$max)+1 : 0)]
    set digits [expr int($a>$b ? $a : $b)]

    # add space for decimal point and fractional part if needed
    if {$decimals < 0} {
	set digits [expr int($digits-$decimals+1)]
    }

    return [list $digits $resolution]
}

# create a shader parameter slider for a single float parameter
# horizontal layout for label, value & slider
# arguments:
#   widget = tcl widget path, last element = shader parameter name
#   text = text label for slider
#   val = initial value
#   min, max = range for slider
proc paramFloat {widget text val min max} {
    # what width for value text & resolution for slider
    set res [computePrecision $min $max 400]

    return [paramScalar $widget $text $val $min $max \
		[lindex $res 0] [lindex $res 1]]
}

# create a shader parameter slider for an integer parameter
# horizontal layout for label, value & slider
# arguments:
#   widget = tcl widget path, last element = shader parameter name
#   text = text label for slider
#   val = initial value
#   min, max = range for slider
proc paramInt {widget text val min max} {
    # what width for value text & resolution for slider
    set res [computePrecision $min $max [expr $max-$min]]

    return [paramScalar $widget $text $val $min $max [lindex $res 0] 1]
}

# create a shader parameter slider for an RGB color
# horizontal layout for label, color swatch & slider
# arguments:
#   widget = tcl widget path, last element = shader parameter name
#   text = text label for slider
#   r,g,b,a = initial value
#   if initial value for a is less than 0, reverts to RGB slider
proc paramRGBA {widget text r g b a} {
    # strip parameter as final component of widget name
    regexp {[^.]+$} $widget param

    # get local name for parameter variable out of p:: namespace
    if {$a >= 0} {
	set ::p::$param [list $r $g $b $a];# default value (RGBA mode)
    } else {
	set ::p::$param [list $r $g $b];# default value (RGB mode)
    }

    # create label, value and slider
    frame $widget
    label $widget.label -text $text
    canvas $widget.value -width 30 -height 30

    scale $widget.r -orient h -showvalue false -width 3 -length 400 \
	-from 0 -to 1 -resolution -1 \
	-command [list colorUpdate $widget $param 0] -bg red
    $widget.r set $r
    bind $widget.r <ButtonRelease> [list shaderParam $param]

    scale $widget.g -orient h -showvalue false -width 3 -length 400 \
	-from 0 -to 1 -resolution -1 \
	-command [list colorUpdate $widget $param 1] -bg green
    $widget.g set $g
    bind $widget.g <ButtonRelease> [list shaderParam $param]

    scale $widget.b -orient h -showvalue false -width 3 -length 400 \
	-from 0 -to 1 -resolution -1 \
	-command [list colorUpdate $widget $param 2] -bg blue
    $widget.b set $b
    bind $widget.b <ButtonRelease> [list shaderParam $param]

    if {$a >= 0} {
	scale $widget.a -orient h -showvalue false -width 3 -length 400 \
	    -from 0 -to 1 -resolution -1 \
	    -command [list colorUpdate $widget $param 3] -bg white
	$widget.a set $a
	bind $widget.a <ButtonRelease> [list shaderParam $param]
    }

    # lay out with label left justified, then value and slider
    grid $widget.label $widget.value $widget.r
    grid ^             ^             $widget.g
    grid ^             ^             $widget.b
    if {$a >= 0} {grid ^ ^           $widget.a}
    grid $widget.label -sticky w
    grid columnconfigure $widget 0 -weight 1
    return $widget
}
proc colorUpdate {widget param index val} {
    global continuous;			# access global behavior toggle
    upvar \#0 p::$param var;		# get local name for parameter
    lset var $index $val;		# set value
					
    # update swatch color
    $widget.value configure -bg \
	[format "#%02x%02x%02x" \
	     [expr int([lindex $var 0]*255)] \
	     [expr int([lindex $var 1]*255)] \
	     [expr int([lindex $var 2]*255)]]

    if {$continuous} {shaderParam $param}
}

# create a shader parameter slider for an RGB color
# horizontal layout for label, color swatch & slider
# arguments:
#   widget = tcl widget path, last element = shader parameter name
#   text = text label for slider
#   r,g,b = initial value
proc paramRGB {widget text r g b} {
    return [paramRGBA $widget $text $r $g $b -1]
}

# create a frame selector graph
# horizontal layout for label, value & graph
# arguments:
#   widget = tcl widget path, last element = shader parameter name
#   text = text label for slider
#   val = initial value
#   max = max frame
#   graph = graph image file
#   left, right, top, bottom = graph's location on image
proc paramFrame {widget text framelist val max graph left right top bottom} {
    # strip parameter as final component of widget name
    regexp {[^.]+$} $widget param

    set ::p::$param -1;			# set default value

    if {$max==0} {set max 1};		# avoid division by 0

    # create label, value and canvas for graph
    frame $widget
    label $widget.value -width \
	[expr [string length $text] + \
	     [string length "current frame (click graph to change)"]]
    canvas $widget.canvas -width 400 -height 400

    # fill in canvas
    image create photo plot -file $graph
    $widget.canvas create image 200 200 -image plot -tags img

    # create and hide line
    $widget.canvas create rectangle \
	-2 $top 3 $bottom -width 4 -state hidden -tag handle -outline "\#88F"

    # event handlers
    #   jump to current position
    set jump [list frameJump $widget $param $text $framelist %x $max \
		  $left $right $top $bottom]
    #   increment/decrement if clicked to the right/left of the current value
    set incdec [list frameIncDec $widget $param $text $framelist %x $max \
		    $left $right $top $bottom]
    #   send parameter update
    set update [list shaderParam $param]

    # drag handle with button 1 or 2
    $widget.canvas bind handle <Button1-Motion> $jump
    $widget.canvas bind handle <Button2-Motion> $jump
    $widget.canvas bind handle <ButtonRelease-1> $update
    $widget.canvas bind handle <ButtonRelease-2> $update

    # button 1 outside handle increments or decrements
    $widget.canvas bind img <ButtonPress-1> $incdec

    # button 2 outside handle jumps to current location then tracks it
    $widget.canvas bind img <ButtonPress-2> $jump
    $widget.canvas bind img <Button2-Motion> $jump
    $widget.canvas bind img <ButtonRelease-2> $update



    # lay out with label left justified, then value and canvas
    grid $widget.value
    grid $widget.canvas

    # set real initial value
    graphUpdate $widget $param $text $framelist $val $max \
	$left $right $top $bottom

    return $widget
}
# event handler helper to jump to a specific frame
proc frameJump {widget param text framelist x max left right top bottom} {
    set val [expr $max*($x-$left)/($right-$left)]
    graphUpdate $widget $param $text $framelist $val $max \
	$left $right $top $bottom
}
# event handler helper to inc/dec if clicked right/left of the current frame
proc frameIncDec {widget param text framelist x max left right top bottom} {
    upvar \#0 p::$param var;		# get local name for parameter
    set click [expr $max*($x-$left)/($right-$left)]
    set val [expr $click < $var ? $var-1 : $click > $var ? $var+1 : $var]
    graphUpdate $widget $param $text $framelist $val $max \
	$left $right $top $bottom
    shaderParam $param
}
# update graph and associated parameter to new value
proc graphUpdate {widget param text framelist val max left right top bottom} {
    global continuous;			# access global behavior toggle
    upvar \#0 p::$param var;		# get local name for parameter

    # clamp to range of legal frames with a 5-frame buffer zone 
    # before switching to frame tracking mode (val = -1)
    set val [expr $val<-5 | $val>$max+5 ? -1 : \
		 $val<0 ? 0 : $val>$max ? $max : $val];

    # update text label
    if {$val<0} {
	$widget.value configure -text \
	    [format "%s current frame (click graph to change)" $text]
    } else {
	$widget.value configure -text \
	    [format "%s frame \#%s" $text [lindex $framelist $val]]
    }
					
    # convert value to graph location
    set gx [expr $left + $val*($right-$left)/$max]
    # update graph marker
    if {$val < 0} {			# remove handle if outside graph
	$widget.canvas itemconfigure handle -state hidden;
    } elseif {$var < 0} {		# make reappear if was hidden
	$widget.canvas itemconfigure handle -state normal
    }
    # move existing handle to new spot
    $widget.canvas coords handle [expr $gx-2] $top [expr $gx+3] $bottom

    # update value
    set var $val

    if {$continuous} {shaderParam $param}
}
