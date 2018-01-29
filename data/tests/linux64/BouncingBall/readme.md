# Steps to generate FMU

First open OMShell, then type:

    >> loadFile("/home/ghorwin/svn/mastersim-code/data/tests/linux64/BouncingBall/BouncingBall.mo")
    >> translateModelFMU(BouncingBall, fmuType="cs", generatedFileName="/home/ghorwin/svn/mastersim-code/data/tests/linux64/BouncingBall/BouncingBall.fmu")
    "/tmp/OpenModelica/BouncingBall.fmu"

Now copy /tmp/OpenModelica/BouncingBall.fmu to wherever it is needed.


