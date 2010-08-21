#! /bin/sh
$EXTRACTRC *.rc *.ui *.kcfg >> rc.cpp
$EXTRACTATTR --attr=constant,name scienceconstants.xml  >>rc.cpp ||exit 11
$XGETTEXT *.cpp -o $podir/kcalc.pot
rm -f rc.cpp
