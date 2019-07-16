#!/bin/bash

export PATH=~/Qt/5.11.3/gcc_64/bin:$PATH
lupdate ../../projects/Qt/MasterSimulatorUI.pro

linguist MasterSimulatorUI_de.ts

