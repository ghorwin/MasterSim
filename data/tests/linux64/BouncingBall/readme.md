# Steps to generate FMU

First open OMShell, then type:

    >> loadFile("/home/ghorwin/svn/mastersim-code/data/tests/linux64/BouncingBall/reference_Modelica/BouncingBall.mo")
    >> translateModelFMU(BouncingBall, fmuType="cs")
    "/tmp/OpenModelica/BouncingBall.fmu"

Now copy /tmp/OpenModelica/BouncingBall.fmu to BouncingBall/fmus.


