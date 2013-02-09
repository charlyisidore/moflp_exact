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
		single_source,
		relaxation,
		lexicographic,
		weighted_sum,
		supported,
		efficient,
		objective,
		display_solution,
		verblevel,
		verbose,
		help;

	// Floating point parameters
	static double
		lambda,
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
