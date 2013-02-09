/*
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PROBLEM_HPP
#define PROBLEM_HPP

#include <iostream>
#include <vector>

/*
	Class: problem

	Representation of an instance of a UFLP/CFLP problem.
*/
struct problem
{
	/*
		Constructor: problem

		Parameters:
			num_objectives - Number of objectives
			capacitated - true for CFLP, false for UFLP
			single_source - true for SSCFLP, false for MSCFLP
	*/
	problem( int num_objectives, bool capacitated = false, bool single_source = true );

	/*
		Constructor: problem

		Initialize for bi-objective.

		Parameters:
			capacitated - true for CFLP, false for UFLP
			single_source - true for SSCFLP, false for MSCFLP
	*/
	problem( bool capacitated = false, bool single_source = true );

	// UFLP information

	int num_objectives,   // Number of objectives
	    num_customers,    // Number of customers
	    num_facilities;   // Number of facilities

	std::vector< std::vector< std::vector<double> > > c; // Cost of assignments
	std::vector< std::vector<double> > f;                // Cost of opening

	// CFLP information

	bool capacitated;        // true if CFLP, false otherwise
	bool single_source;      // true if SSCFLP, false if MSCFLP
	std::vector<double> d;   // Demand of customers
	std::vector<double> q;   // Capacity of facilities
	double D;                // Total demand
	double Q;                // Total capacity
};

std::istream & operator >> ( std::istream & is, problem & instance );
std::ostream & operator << ( std::ostream & os, const problem & instance );

#endif
