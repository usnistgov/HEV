#!/usr/bin/perl -w

# GK uses this to make sure that there are no filename clashes between
# different libraries, a constraint introduced with the Windows port.
# Currently this is very simple, perhaps a more sophisticated thing
# will be written later.  Doubtful.
#
# Usage is:
#
# cd src
# ls -1 {air,hest,biff,ell,nrrd,unrrdu,dye,moss,gage,bane,limn,hoover,mite,ten,echo}/*.c | ./names.pl | sort | more

while (<>) {
    chomp;
    ($dir, $name) = split '/', $_, 2;
    print "$name $dir\n";
}
