#include "argument.hpp"
#include <getopt.h>
#include <cstdlib>

int argument::capacitated( 1 );
int argument::lexicographic( 0 );
int argument::supported( 0 );
int argument::efficient( 1 );
int argument::verbose( 1 );
int argument::help( 0 );
std::string argument::filename;

// getopt long options array
static const struct option long_options[] = {
	{ "uncapacitated", no_argument,       &argument::capacitated,   0 },
	{ "capacitated",   no_argument,       &argument::capacitated,   1 },
	{ "lexicographic", no_argument,       &argument::lexicographic, 1 },
	{ "supported",     no_argument,       &argument::supported,     1 },
	{ "efficient",     no_argument,       &argument::efficient,     1 },
	{ "verbose",       no_argument,       &argument::verbose,       1 },
	{ "quiet",         no_argument,       &argument::verbose,       0 },
	{ "help",          no_argument,       &argument::help,          1 },
	{ 0, 0, 0, 0 }
};

void argument::parse( int argc, char *argv[] )
{
	int next_option;
	int option_index( 0 );

	const char * const short_options = "uclsevqh";

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

			case 'l':
				lexicographic = 1;
				break;

			case 's':
				supported = 1;
				break;

			case 'e':
				efficient = 1;
				break;

			case 'v':
				verbose = 1;
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
		supported = 0;
		efficient = 0;
	}

	if ( supported )
	{
		efficient = 0;
	}
}

void argument::print( std::ostream & os )
{
	os
		<< "File: " << filename << std::endl
		<< "Options:" << std::endl
		<< "\tcapacitated   = " << capacitated   << std::endl
		<< "\tlexicographic = " << lexicographic << std::endl
		<< "\tsupported     = " << supported     << std::endl
		<< "\tefficient     = " << efficient     << std::endl
		<< "\tverbose       = " << verbose       << std::endl;
}

void argument::usage( const char * program_name, std::ostream & os )
{
	os
		<< "Usage: " << program_name << " [OPTIONS] <instance>" << std::endl
		<< "Options:" << std::endl
		<< "\t-u,--uncapacitated  for uncapacitated facility location" << std::endl
		<< "\t-c,--capacitated    for capacitated facility location" << std::endl
		<< "\t-l,--lexicographic  to get lexicographic solutions"    << std::endl
		<< "\t-s,--supported      to get supported solutions"        << std::endl
		<< "\t-e,--efficient      to get efficient solutions"        << std::endl
		<< "\t-q,--quiet          for quiet mode"                    << std::endl
		<< "\t-v,--verbose        for verbose mode"                  << std::endl
		<< "\t-h,--help           to display this help"              << std::endl;
}

