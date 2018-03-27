#!/bin/sh
echo "making $@"
echo "<html>" > $@
echo "<!-- This file $@ was auto generated !-->" >> $@
echo "<head><title>DTK installation</title></head>" >> $@
echo "<body>" >> $@
echo "<h1>DIVERSE Toolkit Installation</h1>" >> $@
echo "<pre>" >> $@
cat  @DTK_SOURCE_DIR@/README >> $@
echo "</pre>" >> $@
cat footer.html >> $@

