#! /bin/bash


#echo fixperm is running id: `id`

if [ $(ypmatch diverse group | grep ${USER} | wc -l) -eq 1 ]
then
    if [ "$(/usr/bin/hostname -s)" == "dradis" ]
    then
	cd ${HEVROOT}/external
	#echo making changes
	chgrp -R diverse .
        chmod -R ug+w,a+r,o-w .
	find . -type d -exec chmod g+s {} \;
	find . -name "*.js" -exec chmod 664 {} \;
    fi
fi
