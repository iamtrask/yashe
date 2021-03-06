/*
This software is governed by the CeCILL license under French law and
abiding by the rules of distribution of free software.  You can  use, 
modify and/ or redistribute the software under the terms of the CeCILL
license as circulated by CEA, CNRS and INRIA at the following URL
"http://www.cecill.info". 

As a counterpart to the access to the source code and  rights to copy,
modify and redistribute granted by the license, users are provided only
with a limited warranty  and the software's author,  the holder of the
economic rights,  and the successive licensors  have only  limited
liability. 

The fact that you are presently reading this means that you have had
knowledge of the CeCILL license and that you accept its terms.

*/

#include "CyclotomicwiseCRT.h"

using namespace std;
using namespace flint;

// --------------------- AUX FUNCTIONS
fmpz_mod_polyxx product_excluding_ith(const fmpzxx& t, const std::vector<flint::fmpz_mod_polyxx>& coprimes, unsigned int i){
	fmpz_mod_polyxx prod(t);
	prod.set_coeff(0, fmpzxx("1"));
	for (unsigned int k = 0; k < i; k++){
		prod *= coprimes[k];
	}

	for (unsigned int k = i+1; k < coprimes.size(); k++){
		prod *= coprimes[k];
	}

	return prod;
}
	
// --------------------- MAIN FUNCTIONS
CyclotomicwiseCRT::CyclotomicwiseCRT(const std::vector<flint::fmpz_mod_polyxx>& _coprimes) 
: t(_coprimes[0].modulus()), coprimes(_coprimes), poly_modulus(t) {
	
	// calculating the product of the cyclotomic polynomials
	poly_modulus.set_coeff(0, fmpzxx("1"));
	for (unsigned int i = 0; i < coprimes.size(); i++){
		poly_modulus *= coprimes[i];
	}

	for (unsigned int i = 0; i < coprimes.size(); i++){
		partial_modulus.push_back(product_excluding_ith(t, coprimes, i)); 
	}
	for (unsigned int i = 0; i < coprimes.size(); i++){
		fmpz_mod_polyxx tmp(partial_modulus[i].invmod(coprimes[i]));
		inverses.push_back(tmp); 
	}
}

flint::fmpz_mod_polyxx CyclotomicwiseCRT::n_th_cyclotomic_mod_t(unsigned int n, const fmpzxx t){
	fmpz_polyxx poly;
	arith_cyclotomic_polynomial(poly._data().inner, n);

	fmpz_mod_polyxx cyclotomic_mod_t(t);
	cyclotomic_mod_t = poly;
	return cyclotomic_mod_t;
}

std::vector<flint::fmpz_mod_polyxx> CyclotomicwiseCRT::factorize_cyclotomic(const fmpz_mod_polyxx& cyclotomic){
	fmpz_mod_poly_factorxx factors = fmpz_mod_poly_factorxx(factor_cantor_zassenhaus(cyclotomic));
	vector<fmpz_mod_polyxx> vec_factors;
    for (unsigned i=0; i < factors.size(); i++){
		cout << "factor " << i << ":   ";
		vec_factors.push_back(fmpz_mod_polyxx(factors.p(i)));
		print_pretty(vec_factors[i], "x");
		cout << endl;
	}
	return vec_factors;
}

const flint::fmpz_mod_polyxx& CyclotomicwiseCRT::get_modulus() const{
	return poly_modulus;
}

const std::vector<flint::fmpz_mod_polyxx>& CyclotomicwiseCRT::get_coprimes() const{
	return coprimes;
}

Plaintext CyclotomicwiseCRT::pack(const std::vector<Plaintext>& remainders){
	fmpz_mod_polyxx poly(t);
	for (unsigned int i = 0; i < coprimes.size(); i++)
		poly += remainders[i].polynomial() * partial_modulus[i] * inverses[i];
	poly %= poly_modulus;
	return Plaintext(poly, poly_modulus);
}

std::vector<Plaintext> CyclotomicwiseCRT::unpack(const Plaintext& plain){
	vector<Plaintext> remainders;
	fmpz_mod_polyxx poly = plain.polynomial();
	for (unsigned int i = 0; i < coprimes.size(); i++){
		fmpz_mod_polyxx tmp(poly % coprimes[i]);
		Plaintext p(tmp, coprimes[i]);
		remainders.push_back(p);
	}
	return remainders;
}
