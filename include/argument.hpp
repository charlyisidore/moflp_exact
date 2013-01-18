#ifndef ARGUMENT_HPP
#define ARGUMENT_HPP

#include <iostream>
#include <string>

/*
	Class: argument

	A static argument parser using getopt.
*/
struct argument
{
	/*
		Function: parse

		Parse the arguments using getopt.

		Parameters:
			argc - The number of arguments.
			argv - The array of arguments strings.
	*/
	static void parse( int argc, char * argv[] );

	/*
		Function: print

		Display what arguments the program has understood.

		Parameters:
			os - An output stream.
	*/
	static void print( std::ostream & os = std::cout );

	/*
		Function: usage

		Display a short user manual.

		Parameters:
			program_name - The executable name (use argv[0]).
			os - An output stream.
	*/
	static void usage( const char * program_name, std::ostream & os = std::cout );

	// Integer or boolean parameters
	static int
		capacitated,
		single_sourcing,
		relaxation,
		lexicographic,
		supported,
		efficient,
		verbose,
		help;

	// Floating point parameters
	static double
		from,
		step;

	// Instance file name
	static std::string filename;

	// Identifiers
	enum
	{
		id_step = 0x100
	};
};

#endif
