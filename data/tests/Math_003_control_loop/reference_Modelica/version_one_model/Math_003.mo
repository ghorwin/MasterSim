model Math_003 "Math_003.mo"
	Real x1;
	Real x2;
	Real x3;
	Real x4;

initial equation
	x4 = 0;

equation
	x1 = if time < 1 then 0 
	else if time < 2 then 1 
	else if time < 5 then 0 
	else 1;
	x2 = if time < 3 then 0 
	else if time < 4 then 1 
	else if time < 6 then 0 
	else 1;
	x3 = if noEvent(x1 > 0 and x2 <= 0.01 and x4 < 2.5) then 3 else
	     if noEvent(x1 <= 0.001 and x2 > 0 and x4 > -2.5) then -3 else 0;
	der(x4) = 2*x3;

annotation(
	x1(flags=2),
	x2(flags=2),
	x3(flags=2),
	x4(flags=2),
	experiment(
		StopTime=10,
		StartTime=0));
end Math_003;

