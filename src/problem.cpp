#include "problem.hpp"
#include <limits>

problem::problem( int num_objectives, bool capacitated, bool single_source ) :
	num_objectives( num_objectives ),
	capacitated( capacitated ),
	single_source( single_source ),
	D( 0. ),
	Q( std::numeric_limits<double>::infinity() )
{
}

problem::problem( bool capacitated, bool single_source ) :
	num_objectives( 2 ),
	capacitated( capacitated ),
	single_source( single_source ),
	D( 0. ),
	Q( std::numeric_limits<double>::infinity() )
{
}

std::istream & operator >> ( std::istream & is, problem & instance )
{
	// Reset all
	instance.c.clear();
	instance.f.clear();
	instance.d.clear();
	instance.q.clear();
	instance.D = 0.;
	instance.Q = std::numeric_limits<double>::infinity();

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
	instance.q.resize( instance.num_facilities, std::numeric_limits<double>::infinity() );

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
			instance.D += instance.d[i];
		}

		// Read capacities
		instance.Q = 0.;
		for ( int j = 0; j < instance.num_facilities; ++j )
		{
			is >> instance.q[j];
			instance.Q += instance.q[j];
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
			os << instance.q[j];
		}
		os << std::endl << std::endl;
	}

	return os;
}

