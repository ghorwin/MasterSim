// CP: 65001
// SimulationX Version: 3.7.2.40674
within ;
model Math_019 "Math_019.mo"
	Real x1;
	Real x2;
	Real x3;
	equation
		1 = x1 + x3;
		1 = x1 + x2 + x3;
		0 = x3 - x2;
	annotation(
		x1(flags=2),
		x2(flags=2),
		x3(flags=2),
		experiment(
			StopTime=1,
			StartTime=0,
			Interval=0.002,
			MaxInterval="0.001"));
end Math_019;
