#! /bin/bash

os_str=$(uname)
echo "os_str: $os_str"

cmd=$(readlink "$0")
echo "cmd: $cmd"
scriptdir=$(dirname "$0")

photoflow=photoflow
align=align_image_stack

if [ x"$os_str" = "xDarwin" ]; then
	photoflow="/Applications/photoflow.app/Contents/MacOS/photoflow"
	align="/Applications/Hugin/Hugin.app/Contents/MacOS/align_image_stack"
fi

if [ x"$PHF_PATH" != "x" ]; then
	photoflow="$PHF_PATH/photoflow"
fi

rawpreset="$scriptdir/raw.pfp"
#shift

nimg=$#

rm -f exposures.txt images.txt
for i in $(seq 1 $nimg); do
	img="$1"
	aperture=$(exiftool -T -aperture "$img")
	#aperture=$("$exiv2" -pv "$img" grep Photo | grep FNumber | tr -s " " | cut -d" " -f6)
        #aperture=$("$exiv2" -pv "$img" grep Photo | grep ExposureTime | tr -s " " | cut -d" " -f6)
	if [ x"${aperture}"x = "x-x" ]; then aperture=8; fi

	speed=$(exiftool -T -SonyExposureTime2 "$img")
        #speed=$("$exiv2" -pv "$img" grep Photo | grep SonyExposureTime2 | tr -s " " | cut -d" " -f6)
	echo "SonyExposureTime2: $speed"
	if [ x"${speed}"x = "x-x" ]; then
		speed=$(exiftool -T -shutterspeed "$img")
	        #speed=$("$exiv2" -pv "$img" grep Photo | grep ExposureTime | tr -s " " | cut -d" " -f6)
	fi

	speeds=$(echo "$speed" | bc -l)
	exposure=$(echo "scale=20; $aperture * $aperture / $speeds" | bc -l)
	nc=$(echo "$exposure" | wc -c)
	exposure2=""
	for j in $(seq $nc 100); do
		exposure2="0$exposure2"
	done
	exposure2="$exposure2$exposure"
	echo "aperture=$aperture  speed=$speed  speeds=$speeds  exposure=$exposure"
	echo "$exposure2 $i" >> exposures.txt
	echo "$img" >> images.txt
	shift
done

#exit

sort -r -n exposures.txt > tmp.txt
rm exposures.txt
mv tmp.txt exposures.txt
cat exposures.txt

tifflist=""

for i in $(seq 1 $nimg); do
	imgid=$(head -n $i exposures.txt | tail -n 1 | cut -d" " -f 2)
	img=$(cat images.txt | sed -n ${imgid}p)
	echo -n "Converting $img..."
	echo "\"$photoflow\" --batch --config=\"$rawpreset\" --export-opt=tiff_depth=32,profile_type=no_change \"$img\" \"$img.tif\""
	"$photoflow" --batch --config="$rawpreset" --export-opt=tiff_depth=32,profile_type=no_change "$img" "$img.tif" >& "$img.log"
	echo " done."
	tifflist="$tifflist $img.tif"
done

test=$("$align" -h 2>&1 | grep use-given-order)
order_opt=""
if [ x"$test" != "x" ]; then
	order_opt="--use-given-order"
fi

echo "\"$align\" -v $order_opt -s 0 -a aligned $tifflist"
rm -f aligned*.tif
"$align" -v $order_opt -s 0 -a aligned $tifflist >& align.log
ls -1 aligned*.tif > aligned.txt

for i in $(seq 1 $nimg); do
	imgid=$(head -n $i exposures.txt | tail -n 1 | cut -d" " -f 2)
	img=$(cat images.txt | sed -n ${imgid}p)
	alimg=$(cat aligned.txt | sed -n ${i}p)
	echo -n "Copying EXIF tags from $img..."
	exiftool -tagsFromFile "${img}.tif" -Orientation#=1 "$alimg"
	echo " done."
done

#exit

refexp=$(head -n 1 exposures.txt | cut -d" " -f 1)

img=$(cat aligned.txt | sed -n 1p)
cat "$scriptdir/base.pfi" | sed "s|%image%|$img|g" > blend1.pfi

plist=""
for i in $(seq 2 $nimg); do
	exp=$(head -n $i exposures.txt | tail -n 1 | cut -d" " -f 1)
	imgid=$(head -n $i exposures.txt | tail -n 1 | cut -d" " -f 2)
	echo "imgid: $imgid"
	#img=$(cat images.txt | sed -n ${imgid}p)
	img=$(cat aligned.txt | sed -n ${i}p)
	r=$(echo "scale=20; $exp / $refexp" | bc -l)
	echo "r: $r"
	cat "$scriptdir/blend.pfp" | sed "s/<property name=\"exposure\" value=\"1\">/<property name=\"exposure\" value=\"$r\">/g" > blend${i}.pfp
	#sed -i.bak "s|%image%|$(pwd)/$img|g" blend${i}.pfp
	sed -i.bak "s|%image%|$img|g" blend${i}.pfp
	sed -i.bak "s|%imageid%|$i|g" blend${i}.pfp

	plist="$plist --config=blend${i}.pfp"

done

echo "\"$photoflow\" --batch $plist --config=\"$scriptdir/wb.pfp\" --config=\"$scriptdir/colorspace_conversion.pfp\" blend1.pfi blend.pfi"
"$photoflow" --batch $plist --config="$scriptdir/wb.pfp" --config="$scriptdir/colorspace_conversion.pfp" blend1.pfi blend.pfi >& blend.pfi.log

#rm $plist aligned.txt exposures.txt images.txt
