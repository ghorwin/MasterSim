// CP: 65001
// SimulationX Version: 3.7.2.40674
within ;
model Part2 "Part2"
	conin in_x1 annotation(Placement(
		transformation(extent={{76,52},{96,72}}),
		iconTransformation(
			origin={-100,0},
			extent={{-10,-10},{10,10}})));
	conout out_x3 annotation(Placement(
		transformation(extent={{72,-10},{92,10}}),
		iconTransformation(extent={{90,-10},{110,10}})));
	Real x2;
	equation
		1 = in_x1.value + x2 + out_x3.value;
		0 = x2 - out_x3.value;
	annotation(
		Line(
			points={{78,-64},{36,-64},{36,-52},{78,-52},{78,-64}},
			color={0,0,255},
			smooth=Smooth.None),
		Icon(graphics={
			Rectangle(
				lineColor={0,0,255},
				extent={{-100,100},{100,-100}}),
			Text(
				textString="Part2",
				lineColor={0,0,255},
				fillColor={255,0,255},
				fillPattern=FillPattern.Solid,
				extent={{-50,56},{54,-50}})}),
		Diagram(coordinateSystem(preserveAspectRatio=false)));
end Part2;
