#! /usr/bin/perl

# readIso.pl - sample program to read a single value

sub getIsoValue {
    # Make these local
    local($isoNameTemp,$isoValueTemp);
    #Get the passed parameter
    $isoNameTemp=$_[0];
    open(ISO,"<$isoNameTemp") || die("Can not open file: $isoNameTemp\n");

#    if (($isoValueTemp=<ISO>) == undef) {
#	die("$isoNameTemp contains bad data\n");
#    }

    $isoValueTemp=<ISO>;
    print STDERR "iso= $isoValueTemp\n";
    # is it a valid number
    if ( ! ($isoValueTemp =~ /^([+-]?)(?=\d|\.\d)\d*(\.\d*)?([Ee]([+-]?\d+))?$/ ) ) {
	die("$isoNameTemp contains bad data\n");
    }

    chop($isoValueTemp);
    return($isoValueTemp);
}

$isoName="demo.iso";
$isoValue=getIsoValue($isoName,"hello","there");

printf "isoValue= %g\n",$isoValue;
