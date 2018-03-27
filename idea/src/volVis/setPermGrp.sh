
# source this file in the installed directory 
# to resolve all perm and group issues

chgrp -R diverse * .
chmod -R ug+w,a+r,o-w * .

