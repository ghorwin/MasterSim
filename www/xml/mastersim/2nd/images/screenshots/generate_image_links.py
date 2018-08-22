import os
from PIL import Image

# parse all png files in current directory
currentPath = os.getcwd()
pngFiles = [f for f in os.listdir(currentPath) if os.path.isfile(os.path.join(currentPath, f)) and f.endswith("png")]

# create thumbnails of all images
THUMBNAIL_WIDTH = 428
THUMBNAIL_MAX_HEIGHT = 400
IMG_PREFIX = "2nd/images/screenshots"
for pngFile in pngFiles:
	# skip thumbnail files
	if pngFile.startswith("th-"):
		continue
	img = Image.open(pngFile)
	pix = img.size
	aspect = float(pix[0])/pix[1]
	newSize = (THUMBNAIL_WIDTH, THUMBNAIL_WIDTH/aspect)
	# resize further if max height is exceeded
	if newSize[1] > THUMBNAIL_MAX_HEIGHT:
		newSize = (THUMBNAIL_MAX_HEIGHT*aspect, THUMBNAIL_MAX_HEIGHT)

	newSize = (int(newSize[0]), int(newSize[1]))
	img = img.resize(newSize, Image.ANTIALIAS)
	thPngFile = "th-" + pngFile
	img.save(thPngFile)
	
	line = "{}/{}|{}|{}||{}/{}|{}|{}|click".format(IMG_PREFIX, thPngFile, newSize[0], newSize[1],
	                                               IMG_PREFIX, pngFile, pix[0], pix[1])
	
	print("{{"+line+"}}")
	
