#!/bin/bash

submit=$1

if [ -z $submit ]; then
    echo "[i] Set it to submit = 0 for local test"
    submit=0
fi

##################################################
### Local test ###
##################################################
if [ $submit -eq 0 ]; then
    echo "[i] Running locally"
    root -l -q 'ana_d0.C("file.list","test.root")'
    exit
fi

###############################################################
### Batch prodcution for real data using file list          ###
###############################################################

if [ $submit -eq 1 ]; then
    #configs=(recofiles)
    configs=(disreco)

    for config in "${configs[@]}"; do
	echo "[i] Running config = $config"

	filelist=/home/dongwi/devcode/frameworks/eicdir/standalone/displacedvertex/filelist/file.${config}.list
	rm $filelist
	filedir=/home/dongwi/devcode/frameworks/eicdir/standalone/displacedvertex
	find /home/dongwi/devcode/frameworks/eicdir/standalone/displacedvertex/${config}/pythia8_D0_18x275_run*.root > $filelist
	output=DIS.D0.${config}.root
	root -l -q /home/dongwi/devcode/frameworks/eicdir/standalone/displacedvertex/vertexing/anasecvertex.C\(\"${filelist}\",\"${output}\"\)
    done
fi

