#! /bin/bash -v

wd=$(pwd)

rm -rf icons
cd icons-inverted
pnglist=$(find . -name "*.png")
nf=$(echo "$pnglist" | wc -l)
echo "number of icons: $nf"

i=1
while [ $i -le $nf ]; do

    f=$(echo "$pnglist" | head -n $i | tail -n 1)
    echo "inverting \"$f\""
    dname=$(dirname "$f")
    fname=$(basename "$f")

    mkdir -p "../icons/$dname"
    echo "convert \"$f\" -negate \"../icons/$dname/$fname\""
    convert "$f" -negate "../icons/$dname/$fname"

    i=$((i+1))

done

cp ../icons/highlights_clip_warning.png ../icons/shadows_clip_warning.png_
cp ../icons/shadows_clip_warning.png ../icons/highlights_clip_warning.png
cp ../icons/shadows_clip_warning.png_ ../icons/shadows_clip_warning.png
rm -f ../icons/shadows_clip_warning.png_
