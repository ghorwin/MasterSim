#!/bin/bash

echo '*** Generating html ***'
asciidoctor MasterSim_manual.adoc
echo '*** Generating pdf ***'
asciidoctor-pdf -a pdf-theme=mastersim-manual-pdf-theme.yml MasterSim_manual.adoc
