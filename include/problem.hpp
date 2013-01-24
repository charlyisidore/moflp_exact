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
			single_sourcing - true for SSCFLP, false for MSCFLP
	*/
	problem( int num_objectives, bool capacitated, bool single_sourcing );

	/*
		Constructor: problem

		Initialize for bi-objective.

		Parameters:
			capacitated - true for CFLP, false for UFLP
			single_sourcing - true for SSCFLP, false for MSCFLP
	*/
	problem( bool capacitated, bool single_sourcing );

	// UFLP information

	int num_objectives,   // Number of objectives
	    num_customers,    // Number of customers
	    num_facilities;   // Number of facilities

	std::vector< std::vector< std::vector<double> > > c; // Cost of assignments
	std::vector< std::vector<double> > f;                // Cost of opening

	// CFLP information

	bool capacitated;        // true if CFLP, false otherwise
	bool single_sourcing;    // true if SSCFLP, false otherwise
	std::vector<double> d;   // Demand of customers
	std::vector<double> Q;   // Capacity of facilities
	double D;                // Total demand
};

std::istream & operator >> ( std::istream & is, problem & instance );
std::ostream & operator << ( std::ostream & os, const problem & instance );

////////////////////////////////////////////////////////////////////////////////

inline problem::problem( int num_objectives, bool capacitated, bool single_sourcing ) :
	num_objectives( num_objectives ),
	capacitated( capacitated ),
	single_sourcing( single_sourcing )
{
}

inline problem::problem( bool capacitated, bool single_sourcing ) :
	num_objectives( 2 ),
	capacitated( capacitated ),
	single_sourcing( single_sourcing )
{
}

#endif
