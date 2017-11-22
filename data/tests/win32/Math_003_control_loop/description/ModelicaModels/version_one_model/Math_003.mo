within ;
model Math_003 "Math_003.mo"
	Real x1;
	Real x2;
	Real x3;
	Real x4;
	initial equation
		x4 = 0;
	equation
		x1 =	 if ((time < 1) or (time >= 2 and time < 5)) then 0 
			 else 1;
		x2 = if ((time < 3) or (time >= 4 and time < 6)) then 0
			 else 1;
		x3 = noEvent(
				if (x1 > 0 and x2 < 0.01 and x4 < 2.5) then 3 
				elseif (x1 < 0.001 and x2 > 0 and x4 > -2.5) then -3 
				else 0
			);
		der(x4) = 2*x3;
	annotation(
		experiment(
			StopTime=10,
			StartTime=0,
			Interval= 0.01, Tolerance = 1e-06));
end Math_003;