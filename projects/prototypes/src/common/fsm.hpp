// Finite state machine definitions

#ifdef UNIX
#include <stdlib.h>
#else
#include <cstdlib>
#endif
#include <assert.h>
#include "common.h"

// Finite state machine
class FSM
{

public:

	// Default types
	typedef int STATE_VALUE;
	enum STATE_TRANSITION { AUTO=0 };

	// I/O
	void input(STATE_TRANSITION);
	STATE_VALUE *output();

	// State
	enum STATE_TYPE { SIMPLE, COMPOSITE, RESOLVER };

	struct State
	{
		char *description;

		STATE_TYPE type;

		// State transition
		struct Transition
		{
			STATE_TRANSITION value;
			struct State *state;
		};

		union Var
		{
			struct
			{
				STATE_VALUE *value;
				struct Transition **transitions;
			} simple;

			struct
			{
				class FSM *fsm;
				struct Transition **transitions;
			} composite;

			struct Resolver
			{
				double probability;
				struct Transition transition;
			} **resolvers;
		} var;

	} *initial,*final,*current;

	// Input subfunction
	enum TRANSITION_STATUS { OK, NG, END };
	TRANSITION_STATUS inputSub(STATE_TRANSITION);

	// Create and link states
	struct State *newSimpleState(int, STATE_VALUE *, char *);
	struct State *newCompositeState(int, char *);
	struct State *newResolverState(int, char *);
	void linkStates(struct State *, struct State *, STATE_TRANSITION);
	void linkStates(struct State *, struct State *, double);
};
