#! /bin/bash
#
# build demo data
#


# Create a wire frame cube
s=20
savg-cube | savg-scale $s $s $s | savg-wireframe > box.savg



# Create 12 distinct colors
c=.7   # "color"
w=.85
i=.5
j=.3

cat > colors.dat <<EOF
    0  0 $c 1
   $i $i $c 1
    0 $c  0 1
   $i $c $i 1
    0 $w $w 1
    0 $i $i 1
   $c  0  0 1
   $c $j $j 1
   $c  0 $c 1
   $i  0 $i 1
   $w $w  0 1
   $i $i  0 1
EOF


for a in 0 1 2 3 4 5 6 7 8 9
do
cat colors.dat | awk -v seed=$RANDOM '
BEGIN  {
    srand(seed);
    print "POINTS";
    m=100
}
{
    red=$1; green=$2; blue=$3;

    for (i=0; i< m; i++) {
        # radius .1 to 1
	radius=.1+(rand()*.9);

        # x y z -10 to + 10
        x=(rand()-.5)*20;
        y=(rand()-.5)*20;
        z=(rand()-.5)*20;

        print x,y,z, red,green,blue, radius;

    }
}' > demo$a.savg

done
