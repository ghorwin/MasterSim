within ;
model Part3
   output Real x4;
   input Real x3;
 annotation (Placement(transformation(extent={{-50,-94},{-30,-74}}),
        iconTransformation(extent={{-50,-94},{-30,-74}})));
equation
  der(x4)  = 2 * x3;

  annotation (uses(Modelica(version="3.2.1")),
                                             Icon(coordinateSystem(
          preserveAspectRatio=true, extent={{-100,-100},{100,100}})),
    Diagram(coordinateSystem(preserveAspectRatio=true,  extent={{-100,-100},{
            100,100}}), graphics));
end Part3;
