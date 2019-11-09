# MasterSim Manual

Written in 'AsciiDoctor'


## Install (Linux)

sudo apt install asciidoctor 

Install ruby, and then:

sudo gem install asciidoctor-pdf --pre
sudo gem install rouge


## Generating Documentation

```bash

# html export
> asciidoctor MasterSim_manual.adoc

# pdf export
> asciidoctor-pdf MasterSim_manual.adoc
```

## Writing notes

Use Editor: AsciidocFX _https://asciidocfx.com_

Mind:

- Pictures file names must only have one .




## TODO - Tool chain

PDFs need high-res images, HTML needs images
properly resized to 1000 pixel widths (depending on style sheet)

-> after html export, adjust png image file names to reference
   scaled versions -> need python script



