#!/bin/bash

# Setup tool chain:
# ... ruby should be there already
# > sudo apt install asciidoctor
# > sudo gem install asciidoctor-pdf --pre
# > sudo gem install rouge


ADOC=Debian_Package_Erstellung

echo '*** Generating html ***' &&
python3 adoc_utils/scripts/adoc-image-prep.py html . &&
asciidoctor -a lang=en $ADOC.adoc &&

echo '*** Generating pdf ***' &&
python3 adoc_utils/scripts/adoc-image-prep.py pdf . &&
asciidoctor-pdf -a lang=en  -a pdf-theme=./adoc_utils/pdf-theme.yml -r ./adoc_utils/rouge_theme.rb -a pdf-fontsdir="./adoc_utils/fonts;GEM_FONTS_DIR" $ADOC.adoc &&

# restore html-type image files
python3 adoc_utils/scripts/adoc-image-prep.py html . &&

exit 0

#echo '*** Copying files to ../../docs directory' &&

#if [ ! -d "../../docs/$ADOC" ]; then
#	mkdir ../docs/$ADOC
#fi &&
#mv $ADOC.html ../docs/$ADOC/index.html 
#mv $ADOC.pdf ../docs &&

#imgFiles=(./images/*.png) 
#if [ ${#imgFiles[@]} -gt 0 ]; then
#	echo 'Copying '${#imgFiles[@]}' images to ../../docs/'$ADOC'/images' &&
#	cp -r ./images/*.png ../docs/$ADOC/images
#fi

