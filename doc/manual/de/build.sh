#!/bin/bash

echo '*** Generating html ***'
asciidoctor MasterSim_manual.adoc
echo '*** Generating pdf ***'
asciidoctor-pdf -a pdf-theme=mastersim-manual-pdf-theme.yml -r ./rouge_theme.rb MasterSim_manual.adoc

mv MasterSim_manual.html ../MasterSim_manual_de.html
mv MasterSim_manual.pdf ../MasterSim_manual_de.pdf
cp images/* ../images

