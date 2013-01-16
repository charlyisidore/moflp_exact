#ifndef FLP_SOLVER_HPP
#define FLP_SOLVER_HPP

#include "problem.hpp"
#include <scip/scip.h>
#include <vector>

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
	flp_solver( const problem & instance );

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

protected:
	const problem & _instance;
	SCIP * _scip;
	SCIP_SOL * _sol;
	SCIP_CONS * _epsilon_cons;
	std::vector< std::vector<SCIP_VAR *> > _x;
	std::vector<SCIP_VAR *> _y;
};

////////////////////////////////////////////////////////////////////////////////

inline double flp_solver::z() const
{
	return SCIPgetSolOrigObj( _scip, _sol );
}

inline bool flp_solver::x( int i, int j ) const
{
	return SCIPgetSolVal( _scip, _sol, _x[i][j] ) > 0.5;
}

inline bool flp_solver::y( int j ) const
{
	return SCIPgetSolVal( _scip, _sol, _y[j] ) > 0.5;
}

#endif
