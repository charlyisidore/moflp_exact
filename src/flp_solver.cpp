#include "flp_solver.hpp"
#include <scip/scipdefplugins.h>
#include <scip/retcode.h>
#include <string>
#include <sstream>

#define SCIP_CALL_EXC(x)                        \
{                                               \
	SCIP_RETCODE retcode;                   \
	if( (retcode = (x)) != SCIP_OKAY)       \
	{                                       \
		SCIPretcodePrintError(retcode); \
		throw;                          \
	}                                       \
}

flp_solver::flp_solver( const problem & instance ) :
	_instance( instance ),
	_scip( 0 ),
	_sol( 0 ),
	_epsilon_cons( 0 ),
	_x( _instance.num_customers, std::vector<SCIP_VAR *>( _instance.num_facilities ) ),
	_y( _instance.num_facilities )
{
	// initialize scip
	SCIP_CALL_EXC( SCIPcreate( &_scip ) );

	// load default plugins linke separators, heuristics, etc.
	SCIP_CALL_EXC( SCIPincludeDefaultPlugins( _scip ) );

	// disable scip output to stdout
	SCIP_CALL_EXC( SCIPsetMessagehdlr( _scip, 0 ) );

	// create an empty problem
	SCIP_CALL_EXC( SCIPcreateProb( _scip, "flp", 0, 0, 0, 0, 0, 0, 0 ) );

	// set the objective sense to minimize, default is minimize
	SCIP_CALL_EXC( SCIPsetObjsense( _scip, SCIP_OBJSENSE_MINIMIZE ) );

	// create a binary variable for every y(j)
	for ( int j = 0; j < _instance.num_facilities; ++j )
	{
		SCIP_VAR * var;
		std::ostringstream namebuf;
		namebuf << "y[" << j << "]";

		// create the SCIP_VAR object
		SCIP_CALL_EXC( SCIPcreateVar( _scip, &var, namebuf.str().c_str(), 0.0, 1.0,
			_instance.f[0][j], SCIP_VARTYPE_BINARY, true, false, 0, 0, 0, 0, 0 ) );

		// add the SCIP_VAR object to the scip problem
		SCIP_CALL_EXC( SCIPaddVar( _scip, var ) );

		// storing the SCIP_VAR pointer for later access
		_y[j] = var;
	}

	// create a binary variable for every x(i,j)
	for ( int i = 0; i < _instance.num_customers; ++i )
	{
		for ( int j = 0; j < _instance.num_facilities; ++j )
		{
			SCIP_VAR * var;
			std::ostringstream namebuf;
			namebuf << "x[" << i << "," << j << "]";

			// create the SCIP_VAR object
			SCIP_CALL_EXC( SCIPcreateVar( _scip, &var, namebuf.str().c_str(), 0.0, 1.0,
				_instance.c[0][i][j], SCIP_VARTYPE_BINARY, true, false, 0, 0, 0, 0, 0 ) );

			// add the SCIP_VAR object to the scip problem
			SCIP_CALL_EXC( SCIPaddVar( _scip, var ) );

			// storing the SCIP_VAR pointer for later access
			_x[i][j] = var;
		}
	}

	// create constraints
	// assignment constraints
	for ( int i = 0; i < _instance.num_customers; ++i )
	{
		SCIP_CONS * cons;
		std::ostringstream namebuf;
		namebuf << "assign_" << i;

		// create SCIP_CONS object, this is an equality
		SCIP_CALL_EXC( SCIPcreateConsLinear( _scip, &cons, namebuf.str().c_str(), 0, 0, 0, 1.0, 1.0,
			true, true, true, true, true, false, false, false, false, false ) );

		// sum(j=1 to n) x(i,j) = 1
		for ( int j = 0; j < _instance.num_facilities; ++j )
		{
			SCIP_CALL_EXC( SCIPaddCoefLinear( _scip, cons, _x[i][j], 1.0 ) );
		}

		// add the constraint to scip
		SCIP_CALL_EXC( SCIPaddCons( _scip, cons ) );
	}

	// facility opening constraints
	for ( int i = 0; i < _instance.num_customers; ++i )
	{
		for ( int j = 0; j < _instance.num_facilities; ++j )
		{
			SCIP_CONS * cons;
			std::ostringstream namebuf;
			namebuf << "open_" << i << "_" << j;

			// create SCIP_CONS object, this is an equality
			SCIP_CALL_EXC( SCIPcreateConsLinear( _scip, &cons, namebuf.str().c_str(), 0, 0, 0, -SCIPinfinity( _scip ), 0,
				true, true, true, true, true, false, false, false, false, false ) );

			SCIP_CALL_EXC( SCIPaddCoefLinear( _scip, cons, _x[i][j], 1.0 ) );
			SCIP_CALL_EXC( SCIPaddCoefLinear( _scip, cons, _y[j], -1.0 ) );

			// add the constraint to scip
			SCIP_CALL_EXC( SCIPaddCons( _scip, cons ) );
		}
	}

	// capacity constraints (CFLP)
	if ( instance.capacitated )
	{
		for ( int j = 0; j < _instance.num_facilities; ++j )
		{
			SCIP_CONS * cons;
			std::ostringstream namebuf;
			namebuf << "cap_" << j;

			// create SCIP_CONS object, this is an equality
			SCIP_CALL_EXC( SCIPcreateConsLinear( _scip, &cons, namebuf.str().c_str(), 0, 0, 0, -SCIPinfinity( _scip ), 0,
				true, true, true, true, true, false, false, false, false, false ) );

			// sum(i=1 to m) d(i) x(i,j) - Q(j) y(j) <= 0
			for ( int i = 0; i < _instance.num_customers; ++i )
			{
				SCIP_CALL_EXC( SCIPaddCoefLinear( _scip, cons, _x[i][j], _instance.d[i] ) );
			}
			SCIP_CALL_EXC( SCIPaddCoefLinear( _scip, cons, _y[j], -_instance.Q[j] ) );

			// add the constraint to scip
			SCIP_CALL_EXC( SCIPaddCons( _scip, cons ) );
		}
	}

	// epsilon constraint on z2

	// create SCIP_CONS object, this is an equality
	SCIP_CALL_EXC( SCIPcreateConsLinear( _scip, &_epsilon_cons, "epsilon", 0, 0, 0, -SCIPinfinity( _scip ), SCIPinfinity( _scip ),
		true, true, true, true, true, false, false, false, false, false ) );

	// z2 <= epsilon
	for ( int j = 0; j < _instance.num_facilities; ++j )
	{
		SCIP_CALL_EXC( SCIPaddCoefLinear( _scip, _epsilon_cons, _y[j], _instance.f[1][j] ) );
	}

	for ( int i = 0; i < _instance.num_customers; ++i )
	{
		for ( int j = 0; j < _instance.num_facilities; ++j )
		{
			SCIP_CALL_EXC( SCIPaddCoefLinear( _scip, _epsilon_cons, _x[i][j], _instance.c[1][i][j] ) );
		}
	}

	// add the constraint to scip
	SCIP_CALL_EXC( SCIPaddCons( _scip, _epsilon_cons ) );
}

flp_solver::~flp_solver()
{
	// after releasing all vars and cons we can free the scip problem
	// remember this has allways to be the last call to scip
	SCIP_CALL_EXC( SCIPfree( &_scip ) );
}

bool flp_solver::weighted_sum( double lambda )
{
	// modify objective of y(j)
	for ( int j = 0; j < _instance.num_facilities; ++j )
	{
		SCIP_CALL_EXC( SCIPchgVarObj( _scip, _y[j], ( 1. - lambda ) * _instance.f[0][j] + lambda * _instance.f[1][j] ) );
	}

	// modify objective of x(i,j)
	for ( int i = 0; i < _instance.num_customers; ++i )
	{
		for ( int j = 0; j < _instance.num_facilities; ++j )
		{
			SCIP_CALL_EXC( SCIPchgVarObj( _scip, _x[i][j], ( 1. - lambda ) * _instance.c[0][i][j] + lambda * _instance.c[1][i][j] ) );
		}
	}

	// this tells scip to start the solution process
	SCIP_CALL_EXC( SCIPpresolve( _scip ) );
	SCIP_CALL_EXC( SCIPsolve( _scip ) );
	SCIP_CALL_EXC( SCIPfreeTransform( _scip ) );

	_sol = SCIPgetBestSol( _scip );

	return _sol != 0;
}

bool flp_solver::epsilon_constraint( double epsilon )
{
	// modify right hand side of epsilon constraint
	SCIP_CALL_EXC( SCIPchgRhsLinear( _scip, _epsilon_cons, epsilon ) );

	// this tells scip to start the solution process
	SCIP_CALL_EXC( SCIPpresolve( _scip ) );
	SCIP_CALL_EXC( SCIPsolve( _scip ) );
	SCIP_CALL_EXC( SCIPfreeTransform( _scip ) );

	_sol = SCIPgetBestSol( _scip );

	return _sol != 0;
}

double flp_solver::z( int k ) const
{
	double obj( 0 );

	for ( int j = 0; j < _instance.num_facilities; ++j )
	{
		if ( y(j) )
			obj += _instance.f[k][j];
	}

	for ( int i = 0; i < _instance.num_customers; ++i )
	{
		for ( int j = 0; j < _instance.num_facilities; ++j )
		{
			if ( x(i,j) )
				obj += _instance.c[k][i][j];
		}
	}
	return obj;
}

