#include "problem.hpp"
#include "flp_solver.hpp"
#include "argument.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <utility>
#include <limits>
#include <algorithm>
#include <ctime>

/*
	Function: lexicographic

	Get the lexicographic solutions of a problem.

	Parameters:
		instance - A problem instance.

	Returns:
		A set of lexicographic solutions.
*/
std::vector< std::vector<double> > lexicographic( const problem & instance );

/*
	Function: dichotomic_method

	Get the supported solutions of a problem using a dichotomic method.

	Parameters:
		instance - A problem instance.

	Returns:
		A set of supported solutions.
*/
std::vector< std::vector<double> > dichotomic_method( const problem & instance );

/*
	Function: epsilon_constraint

	Apply epsilon-constraint method to a problem.

	Parameters:
		instance - A problem instance.

	Returns:
		A set of efficient solutions.
*/
std::vector< std::vector<double> > epsilon_constraint( const problem & instance );

/*
	Function: filter_dominated

	Filter dominated solutions in the Pareto front.

	Parameters:
		pareto_front - A Pareto front.
*/
void filter_dominated( std::vector< std::vector<double> > & pareto_front );

/*
	Function: display

	Display the Pareto front to the terminal.

	Parameters:
		pareto_front - A Pareto front.
*/
void display( const std::vector< std::vector<double> > & pareto_front );

/*
	Function: predicate_is_dominated

	Return true if point is dominated in the Pareto front, false otherwise.

	Parameters:
		z - A point.
*/
struct predicate_is_dominated;

////////////////////////////////////////////////////////////////////////////////

int main( int argc, char * argv[] )
{
	std::vector< std::vector<double> > pareto_front;
	std::ifstream file;
	std::time_t t_start, t_end;

	// Parse program options
	argument::parse( argc, argv );

	// Print usage
	if ( argument::filename.empty() || argument::help )
	{
		argument::usage( argv[0] );
		return 0;
	}

	// Print options
	if ( argument::verbose )
	{
		argument::print( std::clog );
	}

	// Try to open the file
	file.open( argument::filename.c_str() );
	if ( !file.is_open() )
	{
		std::cerr << "Error: unable to open '" << argument::filename << "'" << std::endl;
		return 0;
	}

	// Parse the instance
	if ( argument::verbose )
	{
		std::clog << "Parsing " << argument::filename << "..." << std::endl;
	}

	problem instance( argument::capacitated );
	file >> instance;

	// Begin benchmark
	t_start = std::clock();

	// Solve
	if ( argument::verbose )
	{
		std::clog << "Solving..." << std::endl;
	}

	if ( argument::efficient )
	{
		pareto_front = epsilon_constraint( instance );
	}
	else if ( argument::supported )
	{
		pareto_front = dichotomic_method( instance );
	}
	else if ( argument::lexicographic )
	{
		pareto_front = lexicographic( instance );
	}

	// End benchmark
	t_end = std::clock();

	// Filter
	if ( argument::verbose )
	{
		std::clog << "Filtering..." << std::endl;
	}

	filter_dominated( pareto_front );

	// Display
	display( pareto_front );

	if ( argument::verbose )
	{
		std::clog << "Elapsed time: " << ( t_end - t_start ) / (double)CLOCKS_PER_SEC << "s" << std::endl;
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////

struct predicate_is_dominated
{
	const std::vector< std::vector<double> > & s;
	predicate_is_dominated( const std::vector< std::vector<double> > & s ) : s( s ) {}
	bool operator () ( const std::vector<double> & z ) const;
};

std::vector< std::vector<double> > lexicographic( const problem & instance )
{
	std::vector< std::vector<double> > pareto_front;
	flp_solver solve( instance );
	std::vector<double> y( 2 );

	// Find the lexicographically optimal solutions
	solve.weighted_sum( 0 );
	y[0] = solve.z( 0 );
	y[1] = solve.z( 1 );
	pareto_front.push_back( y );

	solve.weighted_sum( 1 );
	y[0] = solve.z( 0 );
	y[1] = solve.z( 1 );
	pareto_front.push_back( y );

	return pareto_front;
}

std::vector< std::vector<double> > dichotomic_method( const problem & instance )
{
	std::vector< std::vector<double> > pareto_front;
	std::queue< std::pair< std::vector<double>, std::vector<double> > > triangles;
	flp_solver solve( instance );
	std::vector<double> y1( 2 ), y2( 2 ), y( 2 );

	// Find the lexicographically optimal solutions
	solve.weighted_sum( 0 );
	y1[0] = solve.z( 0 );
	y1[1] = solve.z( 1 );
	pareto_front.push_back( y1 );

	solve.weighted_sum( 1 );
	y2[0] = solve.z( 0 );
	y2[1] = solve.z( 1 );
	pareto_front.push_back( y2 );

	// Add the first triangle
	triangles.push( std::make_pair( y1, y2 ) );

	if ( argument::verbose )
	{
		for ( std::size_t i = 0; i < pareto_front.size(); ++i )
		{
			for ( std::size_t k = 0; k < pareto_front[i].size(); ++k )
			{
				if ( k > 0 )
					std::clog << ' ';
				std::clog << pareto_front[i].at( k );
			}
			std::clog << std::endl;
		}
	}

	// Solve all triangles
	while ( !triangles.empty() )
	{
		y1 = triangles.front().first;
		y2 = triangles.front().second;
		triangles.pop();

		// Define the current direction
		double lambda = ( y2[0] - y1[0] ) / ( y1[1] - y2[1] + y2[0] - y1[0] );

		if ( solve.weighted_sum( lambda ) )
		{
			y[0] = solve.z( 0 );
			y[1] = solve.z( 1 );
			pareto_front.push_back( y );

			// New point ?
			if ( y != y1 && y != y2 )
			{
				// Solve recursion
				triangles.push( std::make_pair( y1, y ) );
				triangles.push( std::make_pair( y, y2 ) );

				if ( argument::verbose )
				{
					for ( std::size_t k = 0; k < pareto_front.back().size(); ++k )
					{
						if ( k > 0 )
							std::clog << ' ';
						std::clog << pareto_front.back().at( k );
					}
					std::clog << std::endl;
				}
			}
		}
	}

	return pareto_front;
}

std::vector< std::vector<double> > epsilon_constraint( const problem & instance )
{
	std::vector< std::vector<double> > pareto_front;
	flp_solver solve( instance );
	std::vector<double> y( 2 );

	// Initialize epsilon = infinity
	double epsilon = std::numeric_limits<double>::infinity();

	while ( solve.epsilon_constraint( epsilon ) )
	{
		// Retrieve solution
		y[0] = solve.z( 0 );
		y[1] = solve.z( 1 );
		pareto_front.push_back( y );

		// Update the epsilon value
		epsilon = y[1] - 1;

		if ( argument::verbose )
		{
			for ( std::size_t k = 0; k < pareto_front.back().size(); ++k )
			{
				if ( k > 0 )
					std::clog << ' ';
				std::clog << pareto_front.back().at( k );
			}
			std::clog << std::endl;
		}
	}

	return pareto_front;
}

void filter_dominated( std::vector< std::vector<double> > & pareto_front )
{
	for ( std::vector< std::vector<double> >::iterator it = pareto_front.begin(); it != pareto_front.end(); )
	{
		it = std::find_if( it, pareto_front.end(), predicate_is_dominated( pareto_front ) );
		if ( it != pareto_front.end() )
		{
			it = pareto_front.erase( it );
		}
	}
}

void display( const std::vector< std::vector<double> > & pareto_front )
{
	for ( std::size_t i = 0; i < pareto_front.size(); ++i )
	{
		for ( std::size_t k = 0; k < pareto_front[i].size(); ++k )
		{
			if ( k > 0 )
				std::cout << ' ';
			std::cout << pareto_front[i][k];
		}
		std::cout << std::endl;
	}
}

bool predicate_is_dominated::operator () ( const std::vector<double> & z ) const
{
	for ( std::size_t i = 0; i < s.size(); ++i )
	{
		bool dominated( true );

		if ( &z == &s[i] )
			dominated = false;

		for ( std::size_t k = 0; dominated && k < z.size(); ++k )
		{
			if ( z[k] < s[i][k] )
				dominated = false;
		}

		if ( dominated )
			return true;
	}
	return false;
}

