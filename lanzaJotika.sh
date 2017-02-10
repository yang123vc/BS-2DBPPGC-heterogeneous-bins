#!/bin/sh

#  lanzaJotika.sh
#  
#
#  Created by Marta Cabo Nodar on 10/06/16.
#
cd ./instancesJH

for f in *.dat
do
cp -v $f ../$f
done

#cp -v StockSizes_M.brd ../StockSizes_M.brd
cp -v StockSizes.brd ../StockSizes.brd

cd ..

for f in *.dat
do
echo "Running Instance: ${f%.dat}"

#path for laptop
/Users/Marta/Library/Developer/Xcode/DerivedData/BS_HetBins-gmegmxgmuhwkxlgqenejguyxfysy/Build/Products/Release/BS_HetBins StockSizes.brd "${f%.dat}"

rm $f
done

for i in *.tex; do /Library/TeX/texbin/pdflatex $i;done

for i in *.aux; do rm $i;done

for i in *.log; do rm $i;done

for i in *.pdf
do
mv $i ./instancesJH/Soluciones/
done

for i in *.tex
do
mv $i ./instancesJH/Soluciones/
done

#rm StockSizes_M.brd
rm StockSizes.brd

mv Results.txt ./instancesJH/