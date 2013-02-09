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

#include "argument.hpp"
#include <getopt.h>
#include <sstream>
#include <limits>
#include <cstdlib>

int argument::capacitated( 1 );
int argument::single_source( 1 );
int argument::relaxation( 0 );
int argument::lexicographic( 0 );
int argument::weighted_sum( 0 );
int argument::supported( 0 );
int argument::efficient( 1 );
int argument::objective( 0 );
int argument::display_solution( 0 );
int argument::verblevel( 0 );
int argument::verbose( 1 );
int argument::help( 0 );
double argument::lambda( 0. );
double argument::from( std::numeric_limits<double>::infinity() );
double argument::step( 1 );
std::string argument::filename;

// getopt long options array
static const struct option long_options[] = {
	{ "uncapacitated",    no_argument,       &argument::capacitated,      0   },
	{ "capacitated",      no_argument,       &argument::capacitated,      1   },
	{ "single-source",    no_argument,       &argument::single_source,    1   },
	{ "multi-source",     no_argument,       &argument::single_source,    0   },
	{ "relaxation",       no_argument,       &argument::relaxation,       1   },
	{ "lexicographic",    optional_argument, 0,                           'l' },
	{ "weighted-sum",     required_argument, 0,                           'w' },
	{ "supported",        no_argument,       &argument::supported,        1   },
	{ "efficient",        optional_argument, 0,                           'e' },
	{ "from",             required_argument, 0,                           'f' },
	{ "step",             required_argument, 0,                           argument::id_step },
	{ "display-solution", no_argument,       &argument::display_solution, 1   },
	{ "verblevel",        required_argument, 0,                           'v' },
	{ "verbose",          optional_argument, 0,                           'v' },
	{ "quiet",            no_argument,       &argument::verbose,          0   },
	{ "help",             no_argument,       &argument::help,             1   },
	{ 0, 0, 0, 0 }
};

void argument::parse( int argc, char *argv[] )
{
	int next_option;
	int option_index( 0 );

	const char * const short_options = "ucmrlw:sef:vqh";

	do
	{
		next_option = getopt_long( argc, argv, short_options, long_options, &option_index );
		switch ( next_option )
		{
			case 'u':
				capacitated = 0;
				break;

			case 'c':
				capacitated = 1;
				break;

			case 'm':
				single_source = 0;
				break;

			case 'r':
				relaxation = 1;
				break;

			case 'l':
				lexicographic = 1;
				if ( optarg )
				{
					std::istringstream( optarg ) >> objective;
				}
				break;

			case 'w':
				weighted_sum = 1;
				std::istringstream( optarg ) >> lambda;
				break;

			case 's':
				supported = 1;
				break;

			case 'e':
				efficient = 1;
				if ( optarg )
				{
					std::istringstream( optarg ) >> objective;
					--objective;
				}
				break;

			case 'f':
				std::istringstream( optarg ) >> from;
				break;

			case argument::id_step:
				std::istringstream( optarg ) >> step;
				break;

			case 'v':
				verbose = 1;
				if ( optarg )
				{
					std::istringstream( optarg ) >> verblevel;
				}
				break;

			case 'q':
				verbose = 0;
				break;

			case 'h':
				help = 1;
				break;

			case 0:
			case -1:
				break;

			default:
				std::abort();
		}
	}
	while ( next_option != -1 );

	if ( optind < argc )
	{
		filename = argv[optind];
	}

	if ( lexicographic )
	{
		weighted_sum = 0;
		supported = 0;
		efficient = 0;
	}

	if ( weighted_sum )
	{
		supported = 0;
		efficient = 0;
	}

	if ( supported )
	{
		efficient = 0;
	}

	if ( !verbose )
	{
		verblevel = 0;
	}
}

void argument::print( std::ostream & os )
{
	os
		<< "File: " << filename << std::endl
		<< "Options:" << std::endl
		<< std::boolalpha
		<< "\tcapacitated      = " << (bool)capacitated   << std::endl
		<< "\tsingle-source    = " << (bool)single_source << std::endl
		<< "\trelaxation       = " << (bool)relaxation    << std::endl
		<< "\tlexicographic    = " << (bool)lexicographic << std::endl
		<< "\tweighted-sum     = " << (bool)weighted_sum  << std::endl
		<< "\tsupported        = " << (bool)supported     << std::endl
		<< "\tefficient        = " << (bool)efficient     << std::endl;

	if ( lexicographic && objective )
	{
		os
			<< "\tobjective        = " << objective << std::endl;
	}

	if ( efficient )
	{
		os
			<< "\tobjective        = " << objective+1 << std::endl;
	}

	if ( weighted_sum )
	{
		os
			<< "\tlambda           = " << lambda << std::endl;
	}

	if ( efficient )
	{
		os
			<< "\tfrom             = " << from << std::endl
			<< "\tstep             = " << step << std::endl;
	}

	os
		<< "\tdisplay-solution = " << (bool)display_solution << std::endl
		<< "\tverbose          = " << (bool)verbose          << std::endl;

	if ( verbose )
	{
		os
			<< "\tverblevel        = " << verblevel << std::endl;
	}
}

void argument::usage( const char * program_name, std::ostream & os )
{
	os
		<< "Usage: " << program_name << " [OPTIONS] <instance>" << std::endl
		<< "Options:" << std::endl
		<< "\t-u,--uncapacitated     for uncapacitated facility location"   << std::endl
		<< "\t-c,--capacitated       for capacitated facility location"     << std::endl
		<< "\t-m,--multi-source      for multi source facility location"    << std::endl
		<< "\t-r,--relaxation        for relaxed problem"                   << std::endl
		<< "\t-l,--lexicographic     to get lexicographic solutions"        << std::endl
		<< "\t   --lexicographic=<k> to compute only for objective k"       << std::endl
		<< "\t-w,--weighted-sum <w>  to get a solution of a weighted sum"   << std::endl
		<< "\t-s,--supported         to get supported solutions"            << std::endl
		<< "\t-e,--efficient         to get efficient solutions"            << std::endl
		<< "\t   --efficient=<k>     to set objective k as main objective"  << std::endl
		<< "\t-f,--from <epsilon>    starting value for epsilon-constraint" << std::endl
		<< "\t   --step <delta>      step value for epsilon-constraint"     << std::endl
		<< "\t   --display-solution  to display x and y values"             << std::endl
		<< "\t-q,--quiet             for quiet mode"                        << std::endl
		<< "\t-v,--verbose           for verbose mode"                      << std::endl
		<< "\t   --verblevel <lv>    SCIP verbosity level"                  << std::endl
		<< "\t-h,--help              to display this help"                  << std::endl;
}

