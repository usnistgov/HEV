#! /bin/bash



i=.66666666

case $1 in

front)
	# Create frontBackground.png
	#createimg 1600x1200 $i 0 0 tmp1.png
	#composite -geometry +285+100 hpcvg.png tmp1.png  frontBackGround.png
	./createimg 1600x1200 $i  0  0 frontBackGround.png
	;;

frontAlt)
	./createimg 1600x1200  0 $i $i frontBackGroundAlt.png
	;;

left)
	# Create leftBackground.png
	./createimg 1600x1200  0 $i  0 leftBackGround.png
	;;

leftAlt)
	./createimg 1600x1200 $i  0 $i leftBackGroundAlt.png
	;;


floor)
	# Create floorBackground.png
	./createimg 1600x1200 0 0 $i tmp1.png
	convert tmp1.png  cautionTapeFront.png -geometry +1444+156 -composite  tmp2.png
	convert tmp2.png cautionTapeLeft.png  -geometry +0+156 -composite  tmp3.png
	convert  tmp3.png  -pointsize 30 -fill magenta  -annotate 90x90+5+775 'Check Shoes Before Entering' cautionText.png

	convert ShoeOutline.png  -monochrome bwshoes.png

	convert bwshoes.png  -rotate 25  -monochrome rotshoes.png

	convert rotshoes.png  -transparent white shoes.png

	convert cautionText.png shoes.png -geometry 300x200+600+700 -composite floorBackGround.png

	rm -f tmp*.png cautionText.png *shoes*.png
	;;

floorAlt)
	./createimg 1600x1200 $i $i 0 floorBackGroundAlt.png
	;;

*)
	echo "Errror - wrong parameter in buildBackGrounds.sh" `pwd`
	;;
esac

