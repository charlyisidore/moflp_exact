#ifndef FLP_SOLVER_HPP
#define FLP_SOLVER_HPP

#include "problem.hpp"
#include <scip/scip.h>
#include <vector>
#include <string>
#include <cstdlib>

/*
	Class: flp_solver

	Solve [U|C]FLP with SCIP.
*/
class flp_solver
{
public:
	/*
		Constructor: flp_solver
	*/
	flp_solver( const problem & instance, bool relaxation );

	/*
		Destructor: flp_solver
	*/
	~flp_solver();

	/*
		Method: weighted_sum
	*/
	bool weighted_sum( double lambda );

	/*
		Method: epsilon_constraint
	*/
	bool epsilon_constraint( double epsilon );

	/*
		Method: z
	*/
	double z() const;

	/*
		Method: z
	*/
	double z( int k ) const;

	/*
		Method: x
	*/
	bool x( int i, int j ) const;

	/*
		Method: y
	*/
	bool y( int j ) const;

	/*
		Method: x_real
	*/
	double x_real( int i, int j ) const;

	/*
		Method: y_real
	*/
	double y_real( int j ) const;

	/*
		Method: write_lp
	*/
	void write_lp( FILE * fp = 0, const std::string & ext = "" ) const;

	/*
		Method: write_lp
	*/
	void write_lp( const std::string & filename, const std::string & ext = "" ) const;

protected:
	const problem & _instance;
	SCIP * _scip;
	SCIP_SOL * _sol;
	SCIP_CONS * _epsilon_cons;
	std::vector< std::vector<SCIP_VAR *> > _x;
	std::vector<SCIP_VAR *> _y;
	bool _relaxation;
};

////////////////////////////////////////////////////////////////////////////////

inline double flp_solver::z() const
{
	return SCIPgetSolOrigObj( _scip, _sol );
}

inline bool flp_solver::x( int i, int j ) const
{
	return x_real( i, j ) > 0.5;
}

inline bool flp_solver::y( int j ) const
{
	return y_real( j ) > 0.5;
}

inline double flp_solver::x_real( int i, int j ) const
{
	return SCIPgetSolVal( _scip, _sol, _x[i][j] );
}

inline double flp_solver::y_real( int j ) const
{
	return SCIPgetSolVal( _scip, _sol, _y[j] );
}

#endif
