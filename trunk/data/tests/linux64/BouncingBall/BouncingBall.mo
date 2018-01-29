model BouncingBall "The 'classic' bouncing ball model"
  type Height=Real(unit="m");
  type Velocity=Real(unit="m/s");
  parameter Real e=0.8 "Coefficient of restitution";
  parameter Height h0=1.0 "Initial height";
  output Height h "Height";
  output Velocity v(start=0.0, fixed=true) "Velocity";
  output Integer bounceCounter(start=0);
  output Boolean falling;
initial equation
  h = h0;
equation
  v = der(h);
  der(v) = -9.81;
  if v < 0 then
    falling = true;
  else
    falling = false;
  end if;
  when h<0 then
    reinit(v, -e*pre(v));
    bounceCounter = pre(bounceCounter) + 1;
  end when;
annotation(
    experiment(StartTime = 0, StopTime = 5, Tolerance = 1e-6, Interval = 0.01));end BouncingBall;
