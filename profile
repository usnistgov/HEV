# set up HEV environment
# usage is "source profile [-q | --quiet]"

hev_msg_prefix="${HEVROOT}/profile: "

hev_spew() {
    if [ -z "$hev_quiet" ]
    then (echo "$*" > /dev/tty) 2>/dev/null
    fi
}

# just toss old verbose option
if [ "$1" == "-q" -o "$1" == "--quiet" ] ; then
   hev_quiet=1
   shift
fi

if [ $# -eq 0 -o $# -eq 1 ]; then
   hev_spew "Setting up the IRIS environment"
   shift
   export HEV_IRIS_PROFILE=1
else # more than 1 argument or 1 argument and != savg or iris
     hev_spew "$hev_msg_prefix Unknown option(s), exiting"
     return 1 ;
fi

if [ "$HEVROOT" == "" ] ; then
   export HEVROOT=/usr/local/HEV
fi

if [ "$HEVROOT" != "/usr/local/HEV" ]
then hev_spew "HEV environment using non-standard root, $HEVROOT."
fi

if [ ! -d "$HEVROOT" ] ; then
   hev_spew "$hev_msg_prefix \$HEVROOT is not a directory, exiting"
   return 1 ;
fi

# removes $2 in $1.  $3 is seperator
# this'll break if the seperator is in $1
hev_strip_path() {
    if [ "$3" == "" ] ; then
	hev_spew "$hev_msg_prefix hev_strip_path called with no delimiter, exiting"
	hev_spew "$hev_msg_prefix params passed: $*"
	return 1
    fi

    if [ "$1" == "" -o "$1" == "$2" ] ; then
	return 0
    fi
    echo $1 | sed -e "s|$3$2$3|$3|g" -e "s|^$2$3||" -e "s|$3$2$||"
}

# put $2 in front of $1, using $3 as a seperator
# if $2 is in $1, it will be removed from $1 before prepending
hev_prepend_path() {
    strip=`hev_strip_path "$1" "$2" "$3"`
    if [ "$strip" == "" ] ; then
	echo $2
    else
	echo ${2}${3}${strip}
    fi
}

# put $2 at the end of $1, using $3 as a seperator
# if $2 is in $1, it will be removed from $1 before appending
hev_append_path() {
    strip=`hev_strip_path "$1" "$2" "$3"`
    if [ "$strip" == "" ] ; then
	echo $2
    else
	echo ${strip}${3}${2}
    fi
}

export HEV_PROFILE=1

# set up the iris environment
# pass the path of the external directory
source ${HEVROOT}/external/profile ${HEVROOT}/external

# pass the path of the iris directory
if [ ! -x ${HEV_IRIS_PROFILE} ]; then
    source ${HEVROOT}/iris/profile ${HEVROOT}/iris
fi

# pass the path of the idea directory
source ${HEVROOT}/idea/profile ${HEVROOT}/idea

# misc envars
export __GL_SYNC_TO_VBLANK=1

# misc. legacy kruft- can some of this be deleted?

unset hev_msg_prefix
unset hev_spew
unset hev_strip_path
unset hev_prepend_path
unset hev_append_path
unset hev_quiet
