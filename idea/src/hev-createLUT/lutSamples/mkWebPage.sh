#! /bin/sh

# Make a web page to display the sample Puts

# Usage: ./mkWebPage.sh

# June 1, 2009
# steve@nist.gov

out="samples.html"

cat > ${out} <<EOF
<html>
<title>LUT Samples</title>
<body bgcolor="#dddddd">

<b>
<center>
Sample Color Look Up Tables
</center>
</b>

<UL>

<p>
Some Sample Color Look Up Tables
created with <i>hev-createLUT</i>.
They are compatible with its related commands. See the <i>hev-createLUT</i> on-line manual
page for details.
<p>
These files are located at: \${HEVROOT}/savg/src/hev-createLUT/lutSamples



<hr>
<br><br><br>
EOF


FILES="*.dat"
#FILES=blackWhite.dat

for d in ${FILES}
do
b=`basename $d .dat`
echo $d $b > /dev/tty
hev-createLUT < $d > ${b}.lut

hev-LUT2png ${b}.lut ${b}.png

hev-LUT2plot ${b}.lut temp.png
convert -resize 50% temp.png ${b}_plot.png

echo "<p>" >> ${out}

head -1 $d >> ${out}

cat >> ${out} <<EOF
<br>
<img src="${b}.png">
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
<img src="${b}_plot.png">
<br>
<a href="${d}">${d}</a>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
<a href="${b}.lut">${b}.lut</a>

<br><br><hr><br><br>


EOF

rm -f temp.png
done

cat >> ${out}  <<EOF
</ul>
</body>
</html>
EOF

