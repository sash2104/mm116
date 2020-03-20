#!/bin/bash

problem=$1 # e.g., NumberCreator
outdir="../data"
if [ ! -d $outdir ]
then
    mkdir $outdir
fi

set -x
# for i in $(seq 1 1000)
for i in $(seq 1 10)
do
    outbase=${outdir}/$i
    java ${problem}Vis -exec "python ${problem}.py" -seed $i -novis 2> ${outbase}.in
    # java ${problem}Vis -exec "python ${problem}.py" -seed $i -novis 2> ${outbase}.ans
done
