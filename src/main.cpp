#include "problem.hpp"
#include "flp_solver.hpp"
#include "argument.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <list>
#include <utility>
#include <algorithm>
#include <ctime>

/*
	Function: lexicographic

	Get the lexicographic solutions of a problem.

	Parameters:
		solve - A flp_solver instance.

	Returns:
		A set of lexicographic solutions.
*/
std::list< std::vector<double> > lexicographic( flp_solver & solve );

/*
	Function: weighted_sum

	Get the solution of a weighted sum.

	Parameters:
		solve - A flp_solver instance.

	Returns:
		A set of one solution.
*/
std::list< std::vector<double> > weighted_sum( flp_solver & solve );

/*
	Function: dichotomic_method

	Get the supported solutions of a problem using a dichotomic method.

	Parameters:
		solve - A flp_solver instance.

	Returns:
		A set of supported solutions.
*/
std::list< std::vector<double> > dichotomic_method( flp_solver & solve );

/*
	Function: epsilon_constraint

	Apply epsilon-constraint method to a problem.

	Parameters:
		solve - A flp_solver instance.

	Returns:
		A set of efficient solutions.
*/
std::list< std::vector<double> > epsilon_constraint( flp_solver & solve );

/*
	Function: display_solution

	Display the x and y values to the terminal.

	Parameters:
		solve - A flp_solver instance.
		os - An output stream.
*/
void display_solution( const flp_solver & solve, std::ostream & os );

/*
	Function: display

	Display a point to the terminal.

	Parameters:
		z - A point.
		os - An output stream.
*/
void display( const std::vector<double> & z, std::ostream & os );

/*
	Function: display

	Display the Pareto front to the terminal.

	Parameters:
		pareto_front - A Pareto front.
		os - An output stream.
*/
void display( const std::list< std::vector<double> > & pareto_front, std::ostream & os );

/*
	Function: display_last

	Display the last solution to the terminal if verbose mode is enabled.

	Parameters:
		solve - A flp_solver instance.
		pareto_front - A Pareto front.
		os - An output stream.
*/
void display_last( const flp_solver & solve, const std::list< std::vector<double> > & pareto_front, std::ostream & os );

/*
	Function: predicate_is_dominated

	Return true if point is dominated in the Pareto front, false otherwise.

	Parameters:
		z - A point.
*/
struct predicate_is_dominated
{
	const std::list< std::vector<double> > & s;
	predicate_is_dominated( const std::list< std::vector<double> > & s ) : s( s ) {}
	bool operator () ( const std::vector<double> & z ) const;
};

////////////////////////////////////////////////////////////////////////////////

int main( int argc, char * argv[] )
{
	std::list< std::vector<double> > pareto_front;
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

	problem instance( (bool)argument::capacitated, (bool)argument::single_sourcing );
	file >> instance;

	// Begin benchmark
	t_start = std::clock();

	// Solve
	if ( argument::verbose )
	{
		std::clog << "Solving..." << std::endl;
	}

	// Initialize solver
	flp_solver solve( instance, argument::relaxation );
	solve.set_verblevel( argument::verblevel );

	if ( argument::efficient )
	{
		pareto_front = epsilon_constraint( solve );
	}
	else if ( argument::supported )
	{
		pareto_front = dichotomic_method( solve );
	}
	else if ( argument::weighted_sum )
	{
		pareto_front = weighted_sum( solve );
	}
	else if ( argument::lexicographic )
	{
		pareto_front = lexicographic( solve );
	}

	// End benchmark
	t_end = std::clock();

	// Filter
	if ( argument::verbose )
	{
		std::clog << "Filtering..." << std::endl;
	}

	pareto_front.remove_if( predicate_is_dominated( pareto_front ) );

	// Display
	display( pareto_front, std::cout );

	if ( argument::verbose )
	{
		std::clog << "Elapsed time: "
			<< ( t_end - t_start ) / (double)CLOCKS_PER_SEC
			<< "s" << std::endl;
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////

std::list< std::vector<double> > lexicographic( flp_solver & solve )
{
	std::list< std::vector<double> > pareto_front;
	std::vector<double> y( 2 );

	// One objective
	if ( argument::objective )
	{
		solve.weighted_sum( argument::objective == 1 ? 0. : 1. );
		y[0] = solve.z( 0 );
		y[1] = solve.z( 1 );
		pareto_front.push_back( y );
		display_last( solve, pareto_front, std::clog );
	}
	else // All objectives
	{
		solve.weighted_sum( 0 );
		y[0] = solve.z( 0 );
		y[1] = solve.z( 1 );
		pareto_front.push_back( y );
		display_last( solve, pareto_front, std::clog );

		solve.weighted_sum( 1 );
		y[0] = solve.z( 0 );
		y[1] = solve.z( 1 );
		pareto_front.push_back( y );
		display_last( solve, pareto_front, std::clog );
	}
	return pareto_front;
}

std::list< std::vector<double> > weighted_sum( flp_solver & solve )
{
	std::list< std::vector<double> > pareto_front;
	std::vector<double> y( 2 );

	solve.weighted_sum( argument::lambda );
	y[0] = solve.z( 0 );
	y[1] = solve.z( 1 );
	pareto_front.push_back( y );
	display_last( solve, pareto_front, std::clog );

	return pareto_front;
}

std::list< std::vector<double> > dichotomic_method( flp_solver & solve )
{
	std::list< std::vector<double> > pareto_front;
	std::list< std::vector<double> >::const_iterator it;
	std::queue< std::pair< std::vector<double>, std::vector<double> > > triangles;
	std::vector<double> y1( 2 ), y2( 2 ), y( 2 );

	// Find the lexicographically optimal solutions
	solve.weighted_sum( 0 );
	y1[0] = solve.z( 0 );
	y1[1] = solve.z( 1 );
	pareto_front.push_back( y1 );
	display_last( solve, pareto_front, std::clog );

	solve.weighted_sum( 1 );
	y2[0] = solve.z( 0 );
	y2[1] = solve.z( 1 );
	pareto_front.push_back( y2 );
	display_last( solve, pareto_front, std::clog );

	// Add the first triangle
	triangles.push( std::make_pair( y1, y2 ) );

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
				display_last( solve, pareto_front, std::clog );
			}
		}
	}

	return pareto_front;
}

std::list< std::vector<double> > epsilon_constraint( flp_solver & solve )
{
	std::list< std::vector<double> > pareto_front;
	std::vector<double> y( 2 );
	int obj1 = argument::objective, obj2 = ( argument::objective == 0 ? 1 : 0 );

	solve.set_main_objective( obj1 );

	// Initialize epsilon (default: infinity)
	double epsilon = argument::from;

	while ( solve.epsilon_constraint( epsilon ) )
	{
		// Retrieve solution
		y[0] = solve.z( 0 );
		y[1] = solve.z( 1 );
		pareto_front.push_back( y );
		display_last( solve, pareto_front, std::clog );

		// Update the epsilon value
		epsilon = y[obj2] - argument::step;
	}

	return pareto_front;
}

void display_solution( const flp_solver & solve, std::ostream & os )
{
	os << "y =";
	for ( int j = 0; j < solve.instance.num_facilities; ++j )
	{
		os << ' ' << solve.y_real( j );
	}
	os << std::endl;

	for ( int i = 0; i < solve.instance.num_customers; ++i )
	{
		os << "x[" << i+1 << "] =";
		for ( int j = 0; j < solve.instance.num_facilities; ++j )
		{
			os << ' ' << solve.x_real( i, j );
		}
		os << std::endl;
	}
}

void display( const std::vector<double> & z, std::ostream & os )
{
	for ( std::size_t k = 0; k < z.size(); ++k )
	{
		if ( k > 0 )
			os << ' ';
		os << z[k];
	}
}

void display( const std::list< std::vector<double> > & pareto_front, std::ostream & os )
{
	std::list< std::vector<double> >::const_iterator it;

	for ( it = pareto_front.begin(); it != pareto_front.end(); ++it )
	{
		display( *it, os );
		os << std::endl;
	}
}

void display_last( const flp_solver & solve, const std::list< std::vector<double> > & pareto_front, std::ostream & os )
{
	if ( argument::verbose )
	{
		display( pareto_front.back(), os );
		os << std::endl;

		if ( argument::display_solution )
			display_solution( solve, os );
	}
}

bool predicate_is_dominated::operator () ( const std::vector<double> & z ) const
{
	std::list< std::vector<double> >::const_iterator it;

	for ( it = s.begin(); it != s.end(); ++it )
	{
		bool dominated( true );

		if ( &z == &(*it) )
			dominated = false;

		for ( std::size_t k = 0; dominated && k < z.size(); ++k )
		{
			if ( z[k] < it->at( k ) )
				dominated = false;
		}

		if ( dominated )
			return true;
	}
	return false;
}

