#! /usr/bin/perl

# experiment with using split for prefix:filename
#
# This might be a better way to implement moo paramater
#
#  label:filename
#
# and perhaps g further
#
#  label:r,g,b:filename


use Getopt::Long;

$searator="";

GetOptions("debug"         => \$debug,
	   "separator:s"   => \$separator
);



$n=@ARGV;

print "n= $n\n";
print "ARGV= @ARGV\n";
print "separator= $separator\n"; 


for ($i=0; $i<$n; $i++) {
    @fields=split /$separator/, $ARGV[$i];

    print "$i <> $ARGV[$i] <> @fields\n";

    



}
