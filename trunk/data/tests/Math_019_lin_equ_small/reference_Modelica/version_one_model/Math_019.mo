// CP: 65001
// SimulationX Version: 3.7.2.40674
within ;
model Math_019 "Math_019.mo"
	Real x1;
	Real x2;
	Real x3;
	initial equation
		x1 = 0;
	equation
		1 = x1 + x3;
		1 = x1 + x2 + x3;
		0 = x3 - x2;
end Math_019;
