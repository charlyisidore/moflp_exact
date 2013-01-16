#include "problem.hpp"
#include <limits>

std::istream & operator >> ( std::istream & is, problem & instance )
{
	// Reset all
	instance.c.clear();
	instance.f.clear();
	instance.d.clear();
	instance.Q.clear();

	// Number of customers and facilities
	is >> instance.num_customers;
	is >> instance.num_facilities;

	// Resize assignment costs
	instance.c.resize( instance.num_objectives,
		std::vector< std::vector<double> >( instance.num_customers,
		std::vector<double>( instance.num_facilities, 0 ) ) );

	// Resize opening costs
	instance.f.resize( instance.num_objectives, std::vector<double>( instance.num_facilities, 0 ) );

	// Resize demands and capacities
	instance.d.resize( instance.num_customers, 0 );
	instance.Q.resize( instance.num_facilities, std::numeric_limits<double>::infinity() );

	// Read assignment costs
	for ( int k = 0; k < instance.num_objectives; ++k )
	{
		for ( int i = 0; i < instance.num_customers; ++i )
		{
			for ( int j = 0; j < instance.num_facilities; ++j )
			{
				is >> instance.c[k][i][j];
			}
		}
	}

	// Read opening costs
	for ( int k = 0; k < instance.num_objectives; ++k )
	{
		for ( int j = 0; j < instance.num_facilities; ++j )
		{
			is >> instance.f[k][j];
		}
	}

	// Read CFLP additional information
	if ( instance.capacitated )
	{
		// Read demands
		for ( int i = 0; i < instance.num_customers; ++i )
		{
			is >> instance.d[i];
		}

		// Read capacities
		for ( int j = 0; j < instance.num_facilities; ++j )
		{
			is >> instance.Q[j];
		}
	}

	return is;
}

std::ostream & operator << ( std::ostream & os, const problem & instance )
{
	// Number of customers and facilities
	os << instance.num_customers << std::endl;
	os << instance.num_facilities << std::endl;
	os << std::endl;

	// Read assignment costs
	for ( int k = 0; k < instance.num_objectives; ++k )
	{
		for ( int i = 0; i < instance.num_customers; ++i )
		{
			for ( int j = 0; j < instance.num_facilities; ++j )
			{
				if ( j > 0 ) os << ' ';
				os << instance.c[k][i][j];
			}
			os << std::endl;
		}
		os << std::endl;
	}

	// Read opening costs
	for ( int k = 0; k < instance.num_objectives; ++k )
	{
		for ( int j = 0; j < instance.num_facilities; ++j )
		{
			if ( j > 0 ) os << ' ';
			os << instance.f[k][j];
		}
		os << std::endl << std::endl;
	}

	// Read CFLP additional information
	if ( instance.capacitated )
	{
		// Read demands
		for ( int i = 0; i < instance.num_customers; ++i )
		{
			if ( i > 0 ) os << ' ';
			os << instance.d[i];
		}
		os << std::endl << std::endl;

		// Read capacities
		for ( int j = 0; j < instance.num_facilities; ++j )
		{
			if ( j > 0 ) os << ' ';
			os << instance.Q[j];
		}
		os << std::endl << std::endl;
	}

	return os;
}

