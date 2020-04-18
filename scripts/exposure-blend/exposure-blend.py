import os
import sys
import errno
from subprocess import call
from subprocess import Popen, PIPE
from decimal import *
from fractions import Fraction

# hugindir = "c:\\Program Files\\Hugin"
hugindir = os.getenv('HUGIN_PATH', '/usr/bin')
phfdir = os.getenv('PHF_PATH', '/usr/bin')
exiftooldir = os.getenv('EXIFTOOL_PATH', '/usr/bin')

# converts from a Fraction object to Decimal
def fracToDec(f):
    return Decimal(f.numerator)/Decimal(f.denominator)

# encapsulation of a raw file
class Exposure:
    def __init__(self, id, name, aperture, speed):
        self.id = id
        self.name = name
        self.aperture = aperture
        self.speed = speed
        self.exposure = aperture*aperture/speed
        self.basename = os.path.splitext(self.name)[0]

    def ext(self,extension):
        return self.basename+extension

    def tif(self):
        return self.ext(".tif")

    def log(self):
        return self.ext(".log")
        
    def aligned(self):
        return "aligned" + str(self.id).zfill(4) + ".tif"
        
    def expstr(self):
        with localcontext() as ctx:
            ctx.prec=4 # at most 4 decimal places, but usually less
            return str(fracToDec(self.exposure))

# search & replace by a dictionary
def filesr(dict,srcfile,dstfile):
    # Read in the file
    filedata = None
    with open(srcfile, 'r') as file :
      filedata = file.read()

    # Replace the target string
    for s, r in dict.items():
        filedata = filedata.replace(s, r)

    # Write the file out again
    with open(dstfile, 'w') as file:
      file.write(filedata)

# returns a property tag with the given value      
def ptag(value):
    return "<property name=\"exposure\" value=\"" + str(value) + "\">"


# === script runs from here  ===
    
# verify that we have sufficient arguments and that the files exist
argCount = len(sys.argv)
if (argCount<3):
    print("Syntax: " + sys.argv[0] + " <present name> <raw file 1> <raw file 2> ... <raw file n>")
    sys.exit(1)

for i in range(1,argCount):
    if not os.path.exists(sys.argv[i]):
        print("File not found: " + sys.argv[i])
        sys.exit(1)
        
# save the script directory and the raw conversion preset
scriptdir=sys.path[0]
#rawpreset=sys.argv[1]
rawpreset=os.path.join(scriptdir,"raw.pfp")
blendname="blend.pfp"

blendtemp=os.path.join(scriptdir,blendname)
#pfbatch=scriptdir+"/../../../bin/pfbatch"
photoflow=os.path.join(phfdir,"photoflow")
align_image_stack=os.path.join(hugindir,"align_image_stack")
exiftool=os.path.join(exiftooldir,"exiftool")

wbpreset="--config=" + os.path.join(scriptdir,"wb.pfp")
ccpreset="--config=" + os.path.join(scriptdir,"colorspace_conversion.pfp")

# verify that all required tools exist
for file in [blendtemp,photoflow,align_image_stack,exiftool]:
    if not (os.path.exists(file) or os.path.exists(file+".exe")):
        print("File not found: " + file)
        sys.exit(1)

# array for holding the properties of exposures from the bracketed set
exposures = []
exposures_unsorted = []
    
# go over all given raw files 
for i in range(1,argCount):

    # the current file
    img = sys.argv[i]

    # get the aperture 
    p = Popen([exiftool, "-T", "-aperture", img], stdout=PIPE)
    aperture = p.communicate()[0].decode('ascii').strip()
    if (aperture=="-"): aperture = "8"

    # get the shutter speed
    p = Popen([exiftool, "-T", "-SonyExposureTime2", img], stdout=PIPE)
    speed = p.communicate()[0].decode('ascii').strip()
    if (speed=="-"):
        p = Popen([exiftool, "-T", "-shutterspeed", img], stdout=PIPE)
        speed = p.communicate()[0].decode('ascii').strip()

    # create an exposure instance and store it
    exposure = Exposure(i-1,img,Fraction(aperture),Fraction(speed))
    exposures.append(exposure)
    exposures_unsorted.append(exposure)
    
    # print a message
    print ("img="+img+" aperture="+aperture+" speed="+speed+" exposure="+exposure.expstr()+" id="+str(exposure.id))
        

# sort the bracketed set according to the exposure value in a descending order    
exposures.sort(key=lambda x: x.exposure, reverse=True)

# convert each raw file to tiff
for exposure in exposures:
    sys.stdout.write("Converting " + exposure.name + " to " + exposure.tif() + "...")
    sys.stdout.flush()
    with open(exposure.ext(".log"), "w") as f:
        call([photoflow, "--batch", "--config="+rawpreset, "--export-opt=tiff_depth=32,profile_type=no_change", exposure.name, exposure.tif()], stdout=f, stderr=f)
    print(" done.")

# align the images
alignarray = [align_image_stack, "-v", "--use-given-order", "-s", "0", "-a", "aligned"] + [exposure.tif() for exposure in exposures_unsorted]
print (" ".join(alignarray))
with open("align.log", "w") as f:
    call(alignarray, stdout=f, stderr=f)

# prepare blending files        
blendarray = []
print (exposures[0].name + "(" + str(exposures[0].id) + "): ratio=1")
for i in range(1,len(exposures)):
    exposure = exposures[i]
    call([exiftool, "-overwrite_original", "-tagsFromFile", exposure.name, exposure.aligned()])
    blendfile = blendname.replace(".",str(exposure.id)+".");
    ratio = fracToDec(exposure.exposure / exposures[0].exposure)
    dict = {ptag(1) : ptag(ratio), "%image%" : exposure.aligned()}
    filesr(dict,blendtemp,blendfile)
    blendarray.append("--config="+blendfile)
    print (exposures[i].name + "(" + str(exposure.id) + "): ratio="+str(ratio))

# blend
#"$photoflow" --batch --config="$scriptdir/colorspace_conversion.pfp" $plist --config="$scriptdir/wb.pfp" $(ls -1 aligned*.tif | head -n 1)   blend.pfi #>& blend.pfi.log
blendarray = [photoflow, "--batch", exposures[0].aligned()] + blendarray + [wbpreset] + [ccpreset] + ["blend.pfi"]
print (" ".join(blendarray))
with open("blend.pfi.log", "w") as f:
    call(blendarray, stdout=f, stderr=f)
    
# cleanup
files = os.listdir(".")
for exposure in exposures:
    if os.path.exists(exposure.tif()):
        os.remove(exposure.tif())
    if os.path.exists(exposure.log()):
        os.remove(exposure.log())
