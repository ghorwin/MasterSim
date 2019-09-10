#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

const double RHO_W = 1000; // kg/m3


void pump(const double p_in, const double mdot_in, double & p_out, double & mdot_out) {
	// increases pressure based on performance curve

	static double mdot_last = -1000; // mimic state of FMU
	static double relaxFactor = 0.1;
	// for now, use linear relation ship

	const double MDOT_MAX = 1;
	const double P_MAX = 200000;

	p_out = 100000;
	double deltaP = p_out-p_in;
	bool clipped = false;
	if (deltaP > P_MAX) {
		deltaP = P_MAX-1e-8;
		clipped = true;
	}
	if (deltaP < 0)
		deltaP = 0;

	if (mdot_last == -1000.0)
		mdot_last = mdot_in;

	// prevent excessive clipping
	double mdot_new = (1-deltaP/P_MAX)*MDOT_MAX;
//	mdot_out = (0.7*mdot_new + 0.3*mdot_last);
	if (clipped)
		relaxFactor = 0.01;
	if (!clipped && relaxFactor == 0.01) {
		relaxFactor = 0.001;
	}
	mdot_out = (relaxFactor*mdot_new + (1-relaxFactor)*mdot_last);
	if (!clipped)
		relaxFactor = 0.1;
	mdot_last = mdot_out;
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
		double deltaP = mdot_in* mdot_in *valveValue*200000; // pressure drop modeled linear with valve closing value...

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


int main() {
	std::cout << "Pipe network Gauss-Seidel Test" << std::endl;

	std::vector<double> valveValue = {0,1,100,50,100,0,10};

	double p_in=0;
	double p_out;
	double mdot_in = 0, mdot_out = 0;

	for (unsigned int i=0; i<valveValue.size(); ++i) {
		double mdot_in_last = -100;
		unsigned int MAX_ITERS = 1000;
		unsigned int iterations = MAX_ITERS;
		while ( (std::fabs((mdot_in_last - mdot_in)/mdot_in) > 1e-5 && --iterations) || (MAX_ITERS - iterations < 2)) {
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
		}
		if (iterations == 0)
			std::cout << "Value setting = " << valveValue[i] << ", not converged!" << std::endl;
		else
			std::cout << "Value setting = " << valveValue[i] << ", converged after " << MAX_ITERS-iterations << " iters: mdot = " << mdot_out << std::endl;
	}

	return 0;
}
