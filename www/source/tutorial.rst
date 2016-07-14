.. highlight:: rst

.. toctree::
   :maxdepth: 0


Tutorial
========

This tutorial illustrates usage of the MasterSimulator user interface
to setup a co-simulation scenario.

First step is to **create a new project**. From the welcome page 
choose *Create project...* or from the file menu *New...*.

.. image:: /images/tutorial/Tutorial_01.png


Defining Simulators/Importing FMUs
----------------------------------

Once the project has been created it should be saved to 
some location. For this tutorial simply use the directory
from the examples with the Math003 files and create a new
project file inside this directory.

Now add simulators by selecting FMU files. The slave name 
is automatically generated from the FMU file name, but can
be later edited in the table.

.. image:: /images/tutorial/Tutorial_02.png

You may even add an FMU file several times, in which case
the FMU is instantiated once for each slave.

.. image:: /images/tutorial/Tutorial_03.png

Once all FMUs have been imported, the connections between
the slaves can be made. Before setting connections you may want
to set the cycle index for each FMU. FMUs with the same cycle index
are solved iteratively, if an iteration method is selected.

Analyzing FMUs
--------------

Click on "Analyze FMUs" to parse model description.

.. image:: /images/tutorial/Tutorial_04.png

A window will show basic information (to be extended in future) about
read FMUs.

.. image:: /images/tutorial/Tutorial_05.png

Now all input and output variables appear in the "create connection"
tables.

.. image:: /images/tutorial/Tutorial_06.png

Connecting Simulation Slaves
----------------------------

To connect slaves select input and corresponding output in both tables.
You can double-click on a variable to create a connection or use the
+ button.

.. image:: /images/tutorial/Tutorial_07.png

Slave inputs that are connected disappear from the list of 
available inputs.

.. image:: /images/tutorial/Tutorial_08.png

Once all connections are made, the input variable table should be empty.
However, you can always choose to leave some variables unconnected,
in which case only the start/default value will be set at these inputs.

Specifying Simulation Parameters
--------------------------------

Finally, you need to set the master algorithm parameters and start
the simulation.

.. image:: /images/tutorial/Tutorial_09.png


*End of the MasterSimulator mini-tutorial.*

Back to index: :doc:`index`






