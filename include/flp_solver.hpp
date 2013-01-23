#ifndef FLP_SOLVER_HPP
#define FLP_SOLVER_HPP

#include "problem.hpp"
#include <scip/scip.h>
#include <vector>
#include <string>

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
		Method: capacity_dual
	*/
	double capacity_dual( int j ) const;

	/*
		Method: assignment_dual
	*/
	double assignment_dual( int i ) const;

	/*
		Method: opening_dual
	*/
	double opening_dual( int i, int j ) const;

	/*
		Method: get_verblevel
	*/
	int get_verblevel() const;

	/*
		Method: set_verblevel
	*/
	void set_verblevel( int level ) const;

	/*
		Method: write_lp
	*/
	void write_lp( FILE * fp = 0, const std::string & ext = "" ) const;

	/*
		Method: write_lp
	*/
	void write_lp( const std::string & filename, const std::string & ext = "" ) const;

	// Public attributes
	const problem & instance;

protected:
	SCIP * _scip;
	SCIP_SOL * _sol;
	std::vector< std::vector<SCIP_VAR *> > _x;
	std::vector<SCIP_VAR *> _y;
	std::vector<SCIP_CONS *> _assign_cons;
	std::vector<double>      _assign_dual;
	std::vector<SCIP_CONS *> _cap_cons;
	std::vector<double>      _cap_dual;
	std::vector< std::vector<SCIP_CONS *> > _open_cons;
	std::vector< std::vector<double> >      _open_dual;
	SCIP_CONS * _epsilon_cons;
	bool _relaxation;

	/*
		Method: initialize_problem
	*/
	void initialize_problem();

	/*
		Method: initialize_variables
	*/
	void initialize_variables();

	/*
		Method: initialize_assignment_constraints
	*/
	void initialize_assignment_constraints();

	/*
		Method: initialize_opening_constraints
	*/
	void initialize_opening_constraints();

	/*
		Method: initialize_capacity_constraints
	*/
	void initialize_capacity_constraints();

	/*
		Method: initialize_valid_inequalities
	*/
	void initialize_valid_inequalities();

	/*
		Method: initialize_epsilon_constraints
	*/
	void initialize_epsilon_constraints();

	/*
		Method: store_dual
	*/
	void store_dual();
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

inline double flp_solver::capacity_dual( int j ) const
{
	return _cap_dual[j];
}

inline double flp_solver::assignment_dual( int i ) const
{
	return _assign_dual[i];
}

inline double flp_solver::opening_dual( int i, int j ) const
{
	return _open_dual[i][j];
}


#endif
