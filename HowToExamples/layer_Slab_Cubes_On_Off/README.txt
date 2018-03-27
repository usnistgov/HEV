
June 27, 2014 - Steve Satterfield
June 18, 2015 - (fixed a few typo's)

This How To Example was motivated by a summer 2014 student (Danny
Catacora) project where he wanted to build a scene graph with a
moderate size array of cubes and use NODEMASK commands to select
alternate colors.

As a test and example, I buillt a 40x50 array of cubes. With all cubes
on we get relatively poor performance.

This example illustrates some techniques for building and manipulating
the scene graph using the HEV fifo commands.

To see how this example was created look at build.sh.
It builds all necessary files.


FPS                  1 cube on    All cubes on

Vail (Quadro 5000)   600          63

Tylium (FX5800)      320          18

Nickel (FX4800)      170          17



Build one large slab to get performance and provide the context.
Use individual nodes per cube in a layer to allow on off
of three red green blue cubes.
The color cubes are slightly larger to hide the original when on.
Get good performance when percent of cubes on is low.

These experiments seem to indicate drawing lots of nodes with
individual geometry is relatively slow while lots of triangles in one
larger geometry is very fast (almost free).


# Only need to do this to rebuild all the files
# but make your own copy first.
./build.sh



# Use cut&paste these commands into a bash shell

# run irisfly, is the 's' key to get FPS reported
env __GL_SYNC_TO_VBLANK=0 irisfly --ex wr.savg twolayers.iris oneslab.iris &

# one green cube on
echo NODEMASK g_0_0_0 ON >  $IRIS_CONTROL_FIFO

# one green cube off
echo NODEMASK g_0_0_0 OFF >  $IRIS_CONTROL_FIFO

# 10 blue cubes on
for i in 0 1 2 3 4 5 6 7 8 9
do
	echo NODEMASK b_${i}_${i}_0 ON >  $IRIS_CONTROL_FIFO
done

# 10 green cubes on
for i in 0 1 2 3 4 5 6 7 8 9
do
	echo NODEMASK b_${i}_${i}_0 OFF >  $IRIS_CONTROL_FIFO
	echo NODEMASK g_${i}_${i}_0 ON  >  $IRIS_CONTROL_FIFO
done

# turn them off
for i in 0 1 2 3 4 5 6 7 8 9
do
	echo NODEMASK g_${i}_${i}_0 OFF >  $IRIS_CONTROL_FIFO
done


# display the other slab and layer
echo DCS slab 0 0 3 >  $IRIS_CONTROL_FIFO
echo NODEMASK layer0 OFF>  $IRIS_CONTROL_FIFO
echo NODEMASK layer3 ON>  $IRIS_CONTROL_FIFO


# 10 green on
for i in 0 1 2 3 4 5 6 7 8 9
do
	echo NODEMASK g_${i}_${i}_3 ON >  $IRIS_CONTROL_FIFO
done


# now stress test by turning all on, watch fps change
for ((i=0; i<40; i++))
do
   for ((j=0; j<50; j++))
   do
	echo NODEMASK g_${i}_${j}_3 ON >  $IRIS_CONTROL_FIFO
   done
done

# all off
for ((i=0; i<40; i++))
do
   for ((j=0; j<50; j++))
   do
	echo NODEMASK g_${i}_${j}_3 OFF >  $IRIS_CONTROL_FIFO
   done
done

