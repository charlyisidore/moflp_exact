#include "flp_solver.hpp"
#include <scip/scipdefplugins.h>
#include <scip/retcode.h>
#include <sstream>
#include <numeric>
#include <algorithm>
#include <cstdio>

// Exception calling macro
#define SCIP_CALL_EXC(x)                        \
{                                               \
	SCIP_RETCODE retcode;                   \
	if( (retcode = (x)) != SCIP_OKAY)       \
	{                                       \
		SCIPretcodePrintError(retcode); \
		throw;                          \
	}                                       \
}

// Definition of the SCIP message handler

static SCIP_DECL_MESSAGEWARNING(messageWarningClog)
{
	if ( file == stdout )
	{
		std::clog << "WARNING: " << msg << std::flush;
	}
	else
	{
		std::fputs( "WARNING: ", file );
		std::fputs( msg, file );
		std::fflush(file);
	}
}

static SCIP_DECL_MESSAGEDIALOG(messageDialogClog)
{
	if ( file == stdout )
	{
		std::clog << msg << std::flush;
	}
	else
	{
		std::fputs( msg, file );
		std::fflush(file);
	}
}

static SCIP_DECL_MESSAGEINFO(messageInfoClog)
{
	if ( file == stdout )
	{
		std::clog << msg << std::flush;
	}
	else
	{
		std::fputs( msg, file );
		std::fflush(file);
	}
}

flp_solver::flp_solver( const problem & instance, bool relaxation ) :
	instance( instance ),
	_scip( 0 ),
	_sol( 0 ),
	_x( instance.num_customers, std::vector<SCIP_VAR *>( instance.num_facilities ) ),
	_y( instance.num_facilities ),
	_assign_cons( instance.num_customers, (SCIP_CONS *)0 ),
	_assign_dual( instance.num_customers ),
	_cap_cons( instance.num_facilities, (SCIP_CONS *)0 ),
	_cap_dual( instance.num_facilities ),
	_open_cons( instance.num_customers, std::vector<SCIP_CONS *>( instance.num_facilities, (SCIP_CONS *)0 ) ),
	_open_dual( instance.num_customers, std::vector<double>( instance.num_facilities ) ),
	_epsilon_cons( 0 ),
	_mainobj( 0 ),
	_relaxation( relaxation )
{
	initialize_problem();
	initialize_variables();
	initialize_assignment_constraints();
	initialize_opening_constraints();

	if ( instance.capacitated )
	{
		initialize_capacity_constraints();
		initialize_valid_inequalities();
	}

	initialize_epsilon_constraints();
}

flp_solver::~flp_solver()
{
	// after releasing all vars and cons we can free the scip problem
	// remember this has allways to be the last call to scip
	SCIP_CALL_EXC( SCIPfree( &_scip ) );
}

bool flp_solver::weighted_sum( double lambda )
{
	int k = _mainobj, l = ( _mainobj == 1 ? 0 : 1 );

	// modify objective of y(j)
	for ( int j = 0; j < instance.num_facilities; ++j )
	{
		SCIP_CALL_EXC( SCIPchgVarObj( _scip, _y[j],
			( 1. - lambda ) * instance.f[k][j] + lambda * instance.f[l][j] ) );
	}

	// modify objective of x(i,j)
	for ( int i = 0; i < instance.num_customers; ++i )
	{
		for ( int j = 0; j < instance.num_facilities; ++j )
		{
			SCIP_CALL_EXC( SCIPchgVarObj( _scip, _x[i][j],
				( 1. - lambda ) * instance.c[k][i][j] + lambda * instance.c[l][i][j] ) );
		}
	}

	// this tells scip to start the solution process
	SCIP_CALL_EXC( SCIPpresolve( _scip ) );
	SCIP_CALL_EXC( SCIPsolve( _scip ) );

	// store dual values before the problem is free
	store_dual();
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

	// store dual values before the problem is free
	store_dual();
	SCIP_CALL_EXC( SCIPfreeTransform( _scip ) );

	_sol = SCIPgetBestSol( _scip );

	return _sol != 0;
}

double flp_solver::z( int k ) const
{
	double obj( 0 );

	for ( int j = 0; j < instance.num_facilities; ++j )
	{
		if ( _relaxation )
			obj += y_real( j ) * instance.f[k][j];
		else if ( y( j ) )
			obj += instance.f[k][j];
	}

	for ( int i = 0; i < instance.num_customers; ++i )
	{
		for ( int j = 0; j < instance.num_facilities; ++j )
		{
			if ( _relaxation || !instance.single_sourcing )
				obj += x_real( i, j ) * instance.c[k][i][j];
			else if ( x( i, j ) )
				obj += instance.c[k][i][j];
		}
	}
	return obj;
}

int flp_solver::get_verblevel() const
{
	int level;
	SCIP_CALL_EXC( SCIPgetIntParam( _scip, "display/verblevel", &level ) );
	return level;
}

void flp_solver::set_verblevel( int level ) const
{
	SCIP_CALL_EXC( SCIPsetIntParam( _scip, "display/verblevel", level ) );
}

void flp_solver::write_lp( FILE * fp, const std::string & ext ) const
{
	int level = get_verblevel();
	set_verblevel( SCIP_VERBLEVEL_FULL );
	SCIP_CALL_EXC( SCIPprintOrigProblem( _scip, fp, ext.empty() ? 0 : ext.c_str(), false ) );
	set_verblevel( level );
}

void flp_solver::write_lp( const std::string & filename, const std::string & ext ) const
{
	int level = get_verblevel();
	set_verblevel( SCIP_VERBLEVEL_FULL );
	SCIP_CALL_EXC( SCIPwriteOrigProblem( _scip, filename.c_str(), ext.empty() ? 0 : ext.c_str(), false ) );
	set_verblevel( level );
}

void flp_solver::set_main_objective( int k )
{
	_mainobj = k;

	SCIP_CALL_EXC( SCIPdelCons( _scip, _epsilon_cons ) );
	initialize_epsilon_constraints();

	// modify objective of y(j)
	for ( int j = 0; j < instance.num_facilities; ++j )
	{
		SCIP_CALL_EXC( SCIPchgVarObj( _scip, _y[j], instance.f[k][j] ) );
	}

	// modify objective of x(i,j)
	for ( int i = 0; i < instance.num_customers; ++i )
	{
		for ( int j = 0; j < instance.num_facilities; ++j )
		{
			SCIP_CALL_EXC( SCIPchgVarObj( _scip, _x[i][j], instance.c[k][i][j] ) );
		}
	}
}

void flp_solver::initialize_problem()
{
	SCIP_MESSAGEHDLR * messagehdlr;

	// initialize scip
	SCIP_CALL_EXC( SCIPcreate( &_scip ) );

	// load default plugins linke separators, heuristics, etc.
	SCIP_CALL_EXC( SCIPincludeDefaultPlugins( _scip ) );

	// create message handler
	SCIP_CALL_EXC( SCIPmessagehdlrCreate( &messagehdlr, true, 0, false,
		messageWarningClog, messageDialogClog, messageInfoClog, 0, 0 ) );
	SCIP_CALL_EXC( SCIPsetMessagehdlr( _scip, messagehdlr ) );

	// set verbosity level
	SCIP_CALL_EXC( SCIPsetIntParam( _scip, "display/verblevel", SCIP_VERBLEVEL_NONE ) );

	// create an empty problem
	SCIP_CALL_EXC( SCIPcreateProb( _scip, "flp", 0, 0, 0, 0, 0, 0, 0 ) );

	// set the objective sense to minimize, default is minimize
	SCIP_CALL_EXC( SCIPsetObjsense( _scip, SCIP_OBJSENSE_MINIMIZE ) );
}

void flp_solver::initialize_variables()
{
	int k = _mainobj;

	// create a binary variable for every y(j)
	for ( int j = 0; j < instance.num_facilities; ++j )
	{
		SCIP_VAR * var;
		std::ostringstream namebuf;
		namebuf << "y[" << j << "]";

		// create the SCIP_VAR object
		SCIP_CALL_EXC( SCIPcreateVar( _scip, &var, namebuf.str().c_str(), 0.0, 1.0, instance.f[k][j],
			( _relaxation ) ? SCIP_VARTYPE_CONTINUOUS : SCIP_VARTYPE_BINARY,
			true, false, 0, 0, 0, 0, 0 ) );

		// add the SCIP_VAR object to the scip problem
		SCIP_CALL_EXC( SCIPaddVar( _scip, var ) );

		// storing the SCIP_VAR pointer for later access
		_y[j] = var;
	}

	// create a binary variable for every x(i,j)
	for ( int i = 0; i < instance.num_customers; ++i )
	{
		for ( int j = 0; j < instance.num_facilities; ++j )
		{
			SCIP_VAR * var;
			std::ostringstream namebuf;
			namebuf << "x[" << i << "," << j << "]";

			// create the SCIP_VAR object
			SCIP_CALL_EXC( SCIPcreateVar( _scip, &var, namebuf.str().c_str(), 0.0, 1.0, instance.c[k][i][j],
				( _relaxation || !instance.single_sourcing ) ? SCIP_VARTYPE_CONTINUOUS : SCIP_VARTYPE_BINARY,
				true, false, 0, 0, 0, 0, 0 ) );

			// add the SCIP_VAR object to the scip problem
			SCIP_CALL_EXC( SCIPaddVar( _scip, var ) );

			// storing the SCIP_VAR pointer for later access
			_x[i][j] = var;
		}
	}
}

void flp_solver::initialize_assignment_constraints()
{
	// assignment constraints
	for ( int i = 0; i < instance.num_customers; ++i )
	{
		SCIP_CONS * cons;
		std::ostringstream namebuf;
		namebuf << "assign_" << i;

		// create SCIP_CONS object, this is an equality
		SCIP_CALL_EXC( SCIPcreateConsLinear( _scip, &cons, namebuf.str().c_str(), 0, 0, 0, 1.0, 1.0,
			true, true, true, true, true, false, false, false, false, false ) );

		// sum(j=1 to n) x(i,j) = 1
		for ( int j = 0; j < instance.num_facilities; ++j )
		{
			SCIP_CALL_EXC( SCIPaddCoefLinear( _scip, cons, _x[i][j], 1.0 ) );
		}

		// add the constraint to scip
		SCIP_CALL_EXC( SCIPaddCons( _scip, cons ) );

		// storing the SCIP_CONS pointer for later access
		_assign_cons[i] = cons;
	}
}

void flp_solver::initialize_opening_constraints()
{
	// facility opening constraints
	for ( int i = 0; i < instance.num_customers; ++i )
	{
		for ( int j = 0; j < instance.num_facilities; ++j )
		{
			SCIP_CONS * cons;
			std::ostringstream namebuf;
			namebuf << "open_" << i << "_" << j;

			// x(i,j) <= y(j)
			SCIP_CALL_EXC( SCIPcreateConsLinear( _scip, &cons, namebuf.str().c_str(), 0, 0, 0,
				-SCIPinfinity( _scip ), 0.0,
				true, true, true, true, true, false, false, false, false, false ) );

			SCIP_CALL_EXC( SCIPaddCoefLinear( _scip, cons, _x[i][j], 1.0 ) );
			SCIP_CALL_EXC( SCIPaddCoefLinear( _scip, cons, _y[j], -1.0 ) );

			// add the constraint to scip
			SCIP_CALL_EXC( SCIPaddCons( _scip, cons ) );

			// storing the SCIP_CONS pointer for later access
			_open_cons[i][j] = cons;
		}
	}
}

void flp_solver::initialize_capacity_constraints()
{
	// capacity constraints
	for ( int j = 0; j < instance.num_facilities; ++j )
	{
		SCIP_CONS * cons;
		std::ostringstream namebuf;
		namebuf << "cap_" << j;

		SCIP_CALL_EXC( SCIPcreateConsLinear( _scip, &cons, namebuf.str().c_str(), 0, 0, 0,
			-SCIPinfinity( _scip ), 0.0,
			true, true, true, true, true, false, false, false, false, false ) );

		// sum(i=1 to m) d(i) x(i,j) <= Q(j) y(j)
		for ( int i = 0; i < instance.num_customers; ++i )
		{
			SCIP_CALL_EXC( SCIPaddCoefLinear( _scip, cons, _x[i][j], instance.d[i] ) );
		}
		SCIP_CALL_EXC( SCIPaddCoefLinear( _scip, cons, _y[j], -instance.Q[j] ) );

		// add the constraint to scip
		SCIP_CALL_EXC( SCIPaddCons( _scip, cons ) );

		// storing the SCIP_CONS pointer for later access
		_cap_cons[j] = cons;
	}
}

void flp_solver::initialize_valid_inequalities()
{
	// initialize a cumulative demand array, sorted by increasing values
	std::vector<double> d_cumul( instance.d );
	std::sort( d_cumul.begin(), d_cumul.end() );
	std::partial_sum( d_cumul.begin(), d_cumul.end(), d_cumul.begin() );

	// compute total demand = sum(i=1 to m) d(i)
	double D = d_cumul.back();

	// demand covering
	{
		SCIP_CONS * cons;

		SCIP_CALL_EXC( SCIPcreateConsLinear( _scip, &cons, "cover", 0, 0, 0,
			D, SCIPinfinity( _scip ),
			true, true, true, true, true, false, false, false, false, false ) );

		// sum(j=1 to n) Q(j) y(j) >= sum(i=1 to m) d(i)
		for ( int j = 0; j < instance.num_facilities; ++j )
		{
			SCIP_CALL_EXC( SCIPaddCoefLinear( _scip, cons, _y[j], instance.Q[j] ) );
		}

		// add the constraint to scip
		SCIP_CALL_EXC( SCIPaddCons( _scip, cons ) );
	}

	// demand limit
	{
		SCIP_CONS * cons;

		SCIP_CALL_EXC( SCIPcreateConsLinear( _scip, &cons, "limit", 0, 0, 0,
			-SCIPinfinity( _scip ), D,
			true, true, true, true, true, false, false, false, false, false ) );

		// sum(i=1 to m) sum(j=1 to n) d(i) x(i,j) <= sum(i=1 to m) d(i)
		for ( int i = 0; i < instance.num_customers; ++i )
		{
			for ( int j = 0; j < instance.num_facilities; ++j )
			{
				SCIP_CALL_EXC( SCIPaddCoefLinear( _scip, cons, _x[i][j], instance.d[i] ) );
			}
		}

		// add the constraint to scip
		SCIP_CALL_EXC( SCIPaddCons( _scip, cons ) );
	}

#if 0
	// maximum number of assignments for a facility
	for ( int j = 0; j < instance.num_facilities; ++j )
	{
		SCIP_CONS * cons;
		std::ostringstream namebuf;
		namebuf << "max_assign_" << j;

		int max_assign = std::upper_bound( d_cumul.begin(), d_cumul.end(), instance.Q[j] ) - d_cumul.begin();

		SCIP_CALL_EXC( SCIPcreateConsLinear( _scip, &cons, namebuf.str().c_str(), 0, 0, 0,
			-SCIPinfinity( _scip ), max_assign,
			true, true, true, true, true, false, false, false, false, false ) );

		// sum(i=1 to m) x(i,j) <= max_assign
		for ( int i = 0; i < instance.num_customers; ++i )
		{
			SCIP_CALL_EXC( SCIPaddCoefLinear( _scip, cons, _x[i][j], 1.0 ) );
		}

		// add the constraint to scip
		SCIP_CALL_EXC( SCIPaddCons( _scip, cons ) );
	}
#endif
}

void flp_solver::initialize_epsilon_constraints()
{
	SCIP_CONS * cons;
	int k = _mainobj == 0 ? 1 : 0;

	// epsilon constraint on second objective
	SCIP_CALL_EXC( SCIPcreateConsLinear( _scip, &cons, "epsilon", 0, 0, 0,
		-SCIPinfinity( _scip ), SCIPinfinity( _scip ),
		true, true, true, true, true, false, false, false, false, false ) );

	// objective <= epsilon
	for ( int j = 0; j < instance.num_facilities; ++j )
	{
		SCIP_CALL_EXC( SCIPaddCoefLinear( _scip, cons, _y[j], instance.f[k][j] ) );
	}

	for ( int i = 0; i < instance.num_customers; ++i )
	{
		for ( int j = 0; j < instance.num_facilities; ++j )
		{
			SCIP_CALL_EXC( SCIPaddCoefLinear( _scip, cons, _x[i][j], instance.c[k][i][j] ) );
		}
	}

	// add the constraint to scip
	SCIP_CALL_EXC( SCIPaddCons( _scip, cons ) );

	// storing the SCIP_CONS pointer for later access
	_epsilon_cons = cons;
}

void flp_solver::store_dual()
{
	SCIP_CONS * transformed;

	for ( int i = 0; i < instance.num_customers; ++i )
	{
		SCIPgetTransformedCons( _scip, _assign_cons[i], &transformed );
		if ( transformed )
			_assign_dual[i] = SCIPgetDualsolLinear( _scip, transformed );
	}

	for ( int j = 0; j < instance.num_facilities; ++j )
	{
		SCIPgetTransformedCons( _scip, _cap_cons[j], &transformed );
		if ( transformed )
			_cap_dual[j] = SCIPgetDualsolLinear( _scip, transformed );
	}

	for ( int i = 0; i < instance.num_customers; ++i )
	{
		for ( int j = 0; j < instance.num_facilities; ++j )
		{
    			SCIPgetTransformedCons( _scip, _open_cons[i][j], &transformed );
			if ( transformed )
				_open_dual[i][j] = SCIPgetDualsolLinear( _scip, transformed );
		}
	}
}

