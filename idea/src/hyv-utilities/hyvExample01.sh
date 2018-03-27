#! /bin/sh
#
# hyvExample.sh
#
#    This is a specific example of using hyv-utilities written as a
#    shell script.
#
#    Copy this file to an empty directory.
#    Carefully review and make appropriate changes.
#    Execute each line manually or if you feel lucky try: sh ./hyExample01.sh
#
#    Be careful to notice the cd command included in this script so you
#    are working in the proper directory.
#
# Original: July 25, 2008
# Revision: August 7, 2008
# steve@nist.gov

# Make an empty directory
mkdir C3Ssmall


# Pick 10 time steps for a small data set
# In this example, the interesting stuff happens after step 60

# Select and copy the img files of interest
hyv-selection 61 124 7 ~steve/hydratiCAvis/2007Data/C3Ssmall_20071109 C3Ssmall

# Set N to the number of processors to use
N=2

# Identify and create 4 phases
hyv-img2phase --parallel $N 1 '$c[2]'       C3Ssmall
hyv-img2phase --parallel $N 2 '$c[3]'       C3Ssmall
hyv-img2phase --parallel $N 3 '$c[4]+$c[5]' C3Ssmall
hyv-img2phase --parallel $N 4 '$c[9]'       C3Ssmall


# Convert phase files to vtk format
hyv-phase2vtk  --parallel $N  25 25 25 C3Ssmall/

# Create the necessary *.iso files
hyv-phase2iso  --parallel $N C3Ssmall/
# To create a specific iso value, use the alternate. See the man page.
# For example:
# hyv-phase2iso --iso .5  --parallel $N C3Ssmall/




# Create the isosurface files *.ivu
# Use some interesting RGB values
hyv-vtk2ivu --parallel $N  1 1.0      0.498039 0.941176 C3Ssmall
hyv-vtk2ivu --parallel $N  2 1.0      0.756863 0.498039 C3Ssmall
hyv-vtk2ivu --parallel $N  3 0.866667 1.0      0.498039 C3Ssmall
hyv-vtk2ivu --parallel $N  4 0.498039 0.741176 1.0      C3Ssmall


# Note: We now use other HEV commands, so we cd into the data directory
cd C3Ssmall


# Optimize the isosurfaces and create *.ive files
hev-osgOptimize --parallel $N ive *.ivu

# Make sequence files for each phase
hev-makeSequenceNode phase1 .1  *_1.ive > phase1.sge
hev-makeSequenceNode phase2 .1  *_2.ive > phase2.sge
hev-makeSequenceNode phase3 .1  *_3.ive > phase3.sge
hev-makeSequenceNode phase4 .1  *_4.ive > phase4.sge

# Give them more application specific name
ln -s phase1.sge C3S.sge
ln -s phase2.sge CaOH2.sge
ln -s phase3.sge CSH.sge
ln -s phase4.sge CSHm.sge

# Create a wire frame bounding cube
qvd-wireBox --noclip 25 25 25 .5 .5 .5 temp.ive
dosg-transform --translate 12.5 12.5 12.5 temp.ive Box.ive

#
# At this point we have everything that selectionAppBuilder will use.
# before moving on,let's create the full volume fraction plot.
# Eventually, I'll add the instructions/commands for including the
# plot with the application. For now the plot can be manually displayed.
#

# Need to do this in the original parent directory
cd ..

# Create the *.vf (volume fraction) file we'll use later for the plot
hyv-phase2vf  --parallel $N C3Ssmall/

# Generate but don't display the full volume fraction plot

echo "1 1.0   0.498 0.941  C3S"      > config.prgb
echo "2 1.0   0.757 0.498  Ca(OH)2" >> config.prgb
echo "3 0.867 1.0   0.498  CSH"     >> config.prgb
echo "4 0.498 0.741 1.0    CSH(m)"  >> config.prgb

hyv-vfplot --nodisplay config.prgb C3Ssmall

# To display use
# (normally, you should run this in the forground, but for the purposes
#  of this scrip, I use the &)
hyv-vfplot C3Ssmall &


# Generate the legend image, but we need to tweak the color a bit
# so we can't use the came config file.
echo "1 1.0   0.494 0.937  C3S"      > legend.prgb
echo "2 0.584 0.451 0.310  Ca(OH)2" >> legend.prgb
echo "3 0.376 0.628 0.235  CSH"     >> legend.prgb
echo "4 0.494 0.737 1.0    CSH(m)"  >> legend.prgb


hyv-legend --nodisplay legend.prgb   legend.png

# To display use
hyv-legend legend.png &





################################################################
# Create the final application with selectionAppBuilder        #
# To make the selectionAppBuilder manageable, set options into #
# environment variable                                         #
################################################################

cd C3Ssmall

# For use on the RAVE, include some RAVE specific options
OPT="-pointer -probe -clip"

# Or for desktop only they are not needed
OPT=""

# Specify some options that are useful/needed for both environments
BOTH=" -timewarp -dataRange 0 0 0 25 25 25 -status"

# Specify the files
FILES=" Box.ive C3S.sge  CaOH2.sge CSH.sge CSHm.sge"

# Now actually build the application
selectionAppBuilder $OPT $BOTH $FILES

# If it is necessary to re-run selectionAppBuilder,
# execute
#
#    ./SAB_cleanup.sh
#
# to remove all files created by selectionAppBuilder


echo "To execute the application: ./runapp"



###############################################################
#                                                             #
# For debugging purpose, it is possible to use the            #
# hyv-phase2glyph to generate a glyph representation of       #
# the phase files. At each voxel, a cube glyph will be placed #
# is the voxel value is grater than or equal to the iso value.#
#                                                             #
# Due to the added overhead of lots of small cubes, this      #
# command is generally best used on small img files.          #
#                                                             #
# 
###############################################################


