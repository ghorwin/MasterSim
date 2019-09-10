#include <iostream>
#include <vector>
#include <cmath>

const double RHO_W = 1000; // kg/m3


void pump(const double p_in, const double mdot_in, double & p_out, double & mdot_out) {
	// increases pressure based on performance curve

	// for now, use linear relation ship

	const double MDOT_MAX = 1;
	const double P_MAX = 20000;

	if (mdot_in >= MDOT_MAX) {
		p_out = p_in;
	}
	else {
		double deltaP = (1-mdot_in/MDOT_MAX)*P_MAX;
		p_out = p_in + deltaP;
	}

	mdot_out = mdot_in;
}


void valve(const double valveValue, const double p_in, const double mdot_in, double & p_out, double & mdot_out) {
	// valveValue = 0 - open, no resistance
	// valveValue = 100 - completely closed
	if (mdot_in <= 0 || valveValue == 0.0) {
		// special case, mass flux = 0 -> outlet pressure equal to inlet pressure
		// same when valve is completely open (= 0)
		p_out = p_in;
	}
	else {
		// now compute the pressure drop
		double deltaP = mdot_in* mdot_in *valveValue*10000000; // pressure drop modeled linear with valve closing value...
		p_out = p_in - deltaP;
	}

	mdot_out = mdot_in;
}


void pipe(const double p_in, const double mdot_in, double & p_out, double & mdot_out) {
	double d = 0.05; // m
	double A = 3.15*d*d; // m2
	double L = 1000; // m
	double lambda = 0.4;

	double xeta = lambda * L / d;

	if (mdot_in <= 0) {
		// special case, mass flux = 0 -> outlet pressure equal to inlet pressure
		p_out = p_in;
	}
	else {
		const double RHO_W = 1000; // density of water, kg/m3
		double v = mdot_in/(RHO_W*A); // flow velocity = mass flow rate / cross section and density: kg/s / ( m2 * kg/m3) = m/s
		// now compute the pressure drop
		double deltaP = xeta * RHO_W * v*v / 2;
		p_out = p_in - deltaP;
	}
	mdot_out = mdot_in;
}


void loop_breaker(const double p_in, const double mdot_in, double & p_out, double & mdot_out) {
	// adjusts mdot based on difference between p_out and p_in -> difference approaches zero when converged
	// p_out is always fixed
	const double P_FIX = 1000;

	double deltaP = P_FIX - p_in;
	double delta_mdot = -deltaP*0.00001;

	mdot_out = mdot_in + delta_mdot;
	// prevent back-flow
	if (mdot_out < 1e-5)
		mdot_out = 1e-5;
	if (mdot_out > 1)
		mdot_out = 1;

	p_out = P_FIX;
}


int main() {
	std::cout << "Pipe network Gauss-Seidel Test" << std::endl;

	std::vector<double> valveValue = {0,1,100,50,100,0,10};

	double p_in=1000;
	double p_out;
	double mdot_in = 0.2; // kg/s
	double mdot_out = mdot_in;

	for (unsigned int i=0; i<valveValue.size(); ++i) {
		// always start calculation with supply loop

		double mdot_in_last = -100;
		unsigned int iterations = 100;
		std::cout << "Value setting = " << valveValue[i] << " mdot_initial = " << mdot_in << std::endl;
		while ( std::fabs(mdot_in_last - mdot_in) > 0.000001 && --iterations) {
			mdot_in_last = mdot_in;

			pump(p_in, mdot_in, p_out, mdot_out);
			mdot_in = mdot_out;
			p_in = p_out;

			valve(valveValue[i], p_in, mdot_in, p_out, mdot_out);
			mdot_in = mdot_out;
			p_in = p_out;

			pipe(p_in, mdot_in, p_out, mdot_out);
			mdot_in = mdot_out;
			p_in = p_out;

//			loop_breaker(p_in, mdot_in, p_out, mdot_out);
//			mdot_in = mdot_out;
//			p_in = p_out;
		}
		std::cout << "    converged after " << 100-iterations << " iters: mdot = " << mdot_out << std::endl;
	}

	return 0;
}
