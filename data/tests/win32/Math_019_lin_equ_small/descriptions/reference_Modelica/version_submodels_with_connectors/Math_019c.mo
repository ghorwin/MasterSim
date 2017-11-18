// CP: 65001
// SimulationX Version: 3.7.2.40674
within ;
model Math_019c "Math_019c"
	Real x1=part1.out_x1.value;
	Real x3=part2.out_x3.value;
	Part1 part1 annotation(Placement(transformation(extent={{-30,30},{-10,50}})));
	Part2 part2 annotation(Placement(transformation(extent={{-30,-5},{-10,15}})));
	equation
		connect(part1.out_x1,part2.in_x1) annotation(
			Line(
				points={{-30,40},{-45,40},{-45,5},{-30,5}},
				color={0,0,255},
				thickness=0.0625),
			AutoRoute=false);
		connect(part1.in_x3,part2.out_x3) annotation(
			Line(
				points={{-10,40},{5,40},{5,5},{-10,5}},
				color={0,0,255},
				thickness=0.0625),
			AutoRoute=false);
	annotation(
		x1(flags=2),
		x3(flags=2),
		part2(x2(flags=2)),
		experiment(
			StopTime=1,
			StartTime=0,
			Interval=0.002,
			MaxInterval="0.001"));
end Math_019c;
