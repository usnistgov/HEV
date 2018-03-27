#! /bin/bash

function usage {

echo "Usage: ./build --tim | --iris "
exit
}

case $# in
1)

  case $1 in
  -t*|--t*)
	  type="--tim"
	  ;;
  -i*|--i*)
	  type="--iris"
	  ;;
   *)
	  usage
	  ;;
    esac
    ;;
*)
   usage
   ;;
esac

	   

rocksDir=savg.66
rocksTemplate='StBonn-new_1584.struct-sf10.0.poly.%06d.savg'
#qvisdat=data.qvisdat
qvisdat=data3.qvisdat    # 3 frame test data

v="--verbose"
box="--box 30 30 30"

if [ -x ../qvd-shearingRocksAppBuilder ]
then
    d="../"
else
    d=""
fi


${d}qvd-shearingRocksAppBuilder $v $type $box $rocksDir $rocksTemplate $qvisdat






