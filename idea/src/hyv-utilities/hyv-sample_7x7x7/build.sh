#! /bin/sh
#
# build.sh modified version of hyvExample
#
# hyvExample.sh
#
#    This is a specific example of using hyv-utilities written as a
#    shell script.
#
#    Copy this file to an empty directory.
#    Carefully review and make appropriate changes.
#    Execute each line manually or if you feel lucky try: sh ./build.sh
#
#    Be careful to notice the cd command included in this script so you
#    are working in the proper directory.
#
#    Once all steps here are performed, use ./rundemo to run the
#    created visualization
#
#
# July 25, 2008
# September 19, 2008
# September 24, 2008
# October 2, 2008
# steve@nist.gov

echo Script started at `date`


#####################################
# Setup some parameters
#

# name of sub-directory to create
vis="vis"

# size of the computation system
#systemSize="100 100 100"
systemSize="7 7 7"

# name of source directory with the img file
SRC=c3swithcl02.data

# Selection is nstart nStop nStep
#imgSelection="100 200 10"
imgSelection="1 64 1"


# Set N to the number of processors to use
# Use a trick to determine the number of processors in the system
#N=`grep -c processor /proc/cpuinfo`
# A more precise method
N=`getconf _NPROCESSORS_ONLN`

# Name of phases used for labels
nPhase2='C3S'
nPhase4='CSHii'

# Name of phases used for file names, no special characters such as ()
filePhase2=`echo $nPhase2 | tr -d '()'`
filePhase4=`echo $nPhase4 | tr -d '()'`

#
#####################################


#
# Now get to work
#

# Make an empty directory
if [ ! -d $vis ]
then
  mkdir $vis


  # Pick small number time steps for a small data set

  # Select and copy the img files of interest
  hyv-selection $imgSelection $SRC $vis

  echo Date copy finished at `date`
fi


# Identify and create 2 phases
hyv-img2phase --parallel $N 2 '$c[2]'       $vis
hyv-img2phase --parallel $N 4 '$c[3]'       $vis


# Convert phase files to vtk format
hyv-phase2vtk  --parallel $N  $systemSize $vis

# Create the necessary *.iso files
# Create variable iso values
hyv-phase2iso  --parallel $N $vis
# Or specific iso value for each phase
#hyv-phase2iso  --parallel $N --iso .5    2 $vis
#hyv-phase2iso  --parallel $N --iso .005  4 $vis


# Create the isosurface files *.ivu
# Use some interesting RGB color values
hyv-vtk2ivu --parallel $N  2 1.0      0.498039 0.941176 $vis
hyv-vtk2ivu --parallel $N  4 1.0      0.756863 0.498039 $vis


# Note: We now use other HEV commands, so we cd into the data directory
cd $vis


# Optimize the isosurfaces and create *.ive files
hev-osgOptimize --parallel $N ive *.ivu

# Make sequence files for each phase
hev-makeSequenceNode phase2 .1  *_2.ive > phase2.sge
hev-makeSequenceNode phase4 .1  *_4.ive > phase4.sge

# Give them more application specific name
ln -s phase2.sge ${filePhase2}.sge
ln -s phase4.sge ${filePhase4}.sge


# Create a wire frame bounding cube
qvd-wireBox --noclip $systemSize .5 .5 .5 temp.ive

trans=`echo $systemSize | awk '{print $1/2,$2/2,$3/2}'`
dosg-transform --translate $trans temp.ive Box.ive
rm -f temp.ive

#
# At this point we have everything that selectionAppBuilder will use.
# before moving on,let's create the full volume fraction plot.
# Eventually, I'll add the instructions/commands for including the
# plot with the application. For now the plot can be manually displayed.
#

# Need to do this in the original parent directory
cd ..

# Create the *.vf (volume fraction) file we'll use later for the plot
hyv-phase2vf  --parallel $N $vis

# Generate but don't display the full volume fraction plot

echo "2 1.0   0.498 0.941  ${nPhase2}"  > config.prgb
echo "4 1.0   0.757 0.498  ${nPhase4}" >> config.prgb

hyv-vfplot --nodisplay config.prgb $vis

# To display:
# (normally, you should run this in the foreground, but for the purposes
#  of this script, I run it in the background)
hyv-vfplot $vis &


# Generate the legend image, but we need to tweak the color a bit
# so we can't use the came config file.
echo "1 1.0   0.494 0.937  ${nPhase2}"    > $vis/legend.prgb
echo "2 0.584 0.451 0.310  ${nPhase4}" >> $vis/legend.prgb

hyv-legend --nodisplay ${vis}/legend.prgb   ${vis}/legend.png

# To display:
hyv-legend ${vis}/legend.png &

#
# You might also want a plot of the actual isovalues used.
# The *.iso values are already geenrated
hyv-isoplot --nodisplay config.prgb  vis

# To display:
hyv-isoplot $vis &



###################################################################
# Create glyph representation of the two phases.                  #
#                                                                 #
# This is typically not necessary, but is useful for debugging    #
# and understanding the isosurfaces                               #
###################################################################

hyv-phase2glyph --parallel $N --size .25 2   $systemSize  1.0 0.498039 0.941176 vis
hyv-phase2glyph --parallel $N --size .25 4   $systemSize  1.0 0.756863 0.498039 vis

cd $vis
hev-osgOptimize --parallel $N ive *2_g.savg
hev-osgOptimize --parallel $N ive *4_g.savg

hev-makeSequenceNode phaseGlyph2 .1  *_2_g.ive > phaseGlyph2.sge
hev-makeSequenceNode phaseGlyph4 .1  *_4_g.ive > phaseGlyph4.sge

# Give them more application specific name
ln -s phaseGlyph2.sge ${filePhase2}Glyph.sge
ln -s phaseGlyph4.sge ${filePhase4}Glyph.sge

GLYPHFILES="${filePhase2}Glyph.sge ${filePhase4}Glyph.sge"

cd ..



################################################################
# Create the final application with selectionAppBuilder        #
# To make the selectionAppBuilder manageable, set options into #
# environment variable                                         #
################################################################

cd $vis

# Build a frame counter
nFrames=`ls  -1 *.img | wc -l`
rm -rf counterFiles
ivcounterSeq ${nFrames} .1 counterFiles .5 .5 .5 > counter.seq
dosg-transform --translate .4 1 .4 counter.seq counter.osg
hev-osgOptimize --osgLightingOff counter.osg counter.ive

# For use on the RAVE, include some RAVE specific options
OPT="-pointer -probe -clip -timewarp"

# Or for desktop only they are not needed
OPT="-timewarp"

# Specify some options that are useful/needed for both environments
BOTH=" -timewarp -dataRange 0 0 0 $systemSize -status"

# Specify the files
FILES=" Box.ive ${filePhase2}.sge  ${filePhase4}.sge $GLYPHFILES"

# Now actually build the application
selectionAppBuilder $OPT $BOTH $FILES

# Include the sequence counter 
# This is a bit ugly, but necessary until selectionAppBuilder can be modified
cat runapp | sed "/^savgfly/s/\$/ --scene counter.ive/" > runapp.new
mv runapp.new runapp
chmod +x runapp


# If it is necessary to re-run selectionAppBuilder,
# execute
#
#    ./SAB_cleanup.sh
#
# to remove all files created by selectionAppBuilder


echo "To execute the application: ./runapp"



echo Script finished at `date`

