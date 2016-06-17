within ;
model Part3
    Real x4=out_x4.value;
    Real x3=in_x3.value;
  annotation (uses(Modelica(version="3.0")), Icon(coordinateSystem(
          preserveAspectRatio=true, extent={{-100,-100},{100,100}}), graphics={
          Rectangle(extent={{-80,80},{80,-80}}, lineColor={0,0,255}), Text(
          extent={{-48,54},{56,-52}},
          lineColor={0,0,255},
          fillColor={255,0,255},
          fillPattern=FillPattern.Solid,
          textString="Part3")}),
    Diagram(coordinateSystem(preserveAspectRatio=true,  extent={{-100,-100},{
            100,100}}), graphics));
  conin in_x3 annotation (Placement(transformation(extent={{-100,-10},{-80,10}}),
        iconTransformation(extent={{-100,-10},{-80,10}})));
  conout out_x4 annotation (Placement(transformation(extent={{-50,-94},{-30,-74}}),
        iconTransformation(extent={{-50,-94},{-30,-74}})));
initial equation
  x4 = 0;
equation
  der(x4)  = 2 * x3;

end Part3;
