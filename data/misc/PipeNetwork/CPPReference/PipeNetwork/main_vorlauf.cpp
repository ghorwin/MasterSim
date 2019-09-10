#include <iostream>
#include <vector>
#include <cmath>

const double RHO_W = 1000; // kg/m3


void pump(const double p_ref, const double p_out, double & p_in, double & mdot_out) {
	// increases pressure based on performance curve

	// p_out - Druck vom Verbraucher, gegen den die Pumpe arbeitet
	// p_in - konstanter Referenzdruck

	// for now, use linear relation ship

	const double MDOT_MAX = 1;
	const double DELTAP_MAX = 2000000;
	const double P_OUT = 10000;

	double deltaP = p_out-p_ref;
	if (deltaP >= DELTAP_MAX) {
		mdot_out = 1e-5;
	}
	else {
		mdot_out = std::max((1-deltaP/DELTAP_MAX)*MDOT_MAX, 1e-05);
	}
	p_in = p_ref;
}


void valve(const double valveValue, const double p_out, const double mdot_in, double & p_in, double & mdot_out) {
	// valveValue = 0 - open, no resistance
	// valveValue = 100 - completely closed
	if (mdot_in <= 0 || valveValue == 0.0) {
		// special case, mass flux = 0 -> outlet pressure equal to inlet pressure
		// same when valve is completely open (= 0)
		p_in = p_out;
	}
	else {
		// now compute the pressure drop
		double deltaP = mdot_in* mdot_in *valveValue*10000000; // pressure drop modeled linear with valve closing value...
		p_in = p_out + deltaP;
	}

	mdot_out = mdot_in;
}


void pipe(const double p_out, const double mdot_in, double & p_in, double & mdot_out) {
	double d = 0.05; // m
	double A = 3.15*d*d; // m2
	double L = 1000; // m
	double lambda = 0.4;

	double xeta = lambda * L / d;

	if (mdot_in <= 0) {
		// special case, mass flux = 0 -> outlet pressure equal to inlet pressure
		p_in = p_out;
	}
	else {
		const double RHO_W = 1000; // density of water, kg/m3
		double v = mdot_in/(RHO_W*A); // flow velocity = mass flow rate / cross section and density: kg/s / ( m2 * kg/m3) = m/s
		// now compute the pressure drop
		double deltaP = xeta * RHO_W * v*v / 2;
		p_in = p_out + deltaP;
	}
	mdot_out = mdot_in;
}


int main() {
	std::cout << "Pipe network Gauss-Seidel Test" << std::endl;

	std::vector<double> valveValue = {0,1,100,50,100,0,10};

	double p_ref=10000;
	double p_out=10000;
	double mdot_in = 0.2; // kg/s
	double mdot_out = mdot_in;

	for (unsigned int i=0; i<valveValue.size(); ++i) {
		// always start calculation with supply loop

		double mdot_in_last = -100;
		double p_in = p_ref;
		unsigned int iterations = 100;
		std::cout << "Value setting = " << valveValue[i] << " mdot_initial = " << mdot_in << std::endl;
/*		while ( std::fabs(mdot_in_last - mdot_in) > 0.000001 && --iterations) {
			mdot_in_last = mdot_in;

			pump(p_ref, p_out, p_in, mdot_out);
			mdot_in = mdot_out;
			p_out = p_in;

			valve(valveValue[i], p_out, mdot_in, p_in, mdot_out);
			mdot_in = mdot_out;
			p_out = p_in;

			pipe(p_out, mdot_in, p_in, mdot_out);
			mdot_in = mdot_out;
			p_out = p_in;
		}*/
		while ( std::fabs(mdot_in_last - mdot_in) > 0.000001 && --iterations) {
			mdot_in_last = mdot_in;

			pipe(p_out, mdot_in, p_in, mdot_out);
			mdot_in = mdot_out;
			p_out = p_in;

			valve(valveValue[i], p_out, mdot_in, p_in, mdot_out);
			mdot_in = mdot_out;
			p_out = p_in;

			pump(p_ref, p_out, p_in, mdot_out);
			mdot_in = mdot_out;
			p_out = p_in;

		}
		std::cout << "    converged after " << 100-iterations << " iters: mdot = " << mdot_out << std::endl;
	}

	return 0;
}
