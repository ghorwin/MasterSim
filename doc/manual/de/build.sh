#!/bin/bash

ADOC=MasterSim_manual

echo '*** Generating html ***' &&
python3 ../adoc_utils/adoc-image-prep.py html . &&
asciidoctor -a lang=de $ADOC.adoc &&

echo &&
echo '*** Generating pdf ***' &&
python3 ../adoc_utils/adoc-image-prep.py pdf . &&
asciidoctor-pdf -a lang=de  -a pdf-theme=mastersim-manual-pdf-theme.yml  -r ./rouge_theme.rb -a pdf-fontsdir="../adoc_utils/fonts;GEM_FONTS_DIR" $ADOC.adoc &&

# restore html-type image files
echo &&
echo '*** Restoring html ***' &&
python3 ../adoc_utils/adoc-image-prep.py html . &&

echo &&
echo '*** Copying files target directory ***' &&

mv MasterSim_manual.html ../html/MasterSim_manual_de.html &&
mv MasterSim_manual.pdf ../MasterSim_manual_de.pdf && 

echo '*** Finished successfully ***'


