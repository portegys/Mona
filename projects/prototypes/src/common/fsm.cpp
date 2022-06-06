// Finite state machine functions

#include "fsm.hpp"

// Create simple state
struct FSM::State *
FSM::newSimpleState(int numTransitions, STATE_VALUE *value, char *description)
{
	struct State *s;
	int i;

	assert(numTransitions >= 0);

	s = new struct State;
	assert(s != NULL);
	s->description = description;
	s->type = SIMPLE;
	s->var.simple.value = value;
	s->var.simple.transitions = new struct State::Transition*[numTransitions+1];
	assert(s->var.simple.transitions != NULL);
	for (i = 0; i < numTransitions; i++)
	{
		s->var.simple.transitions[i] = new struct State::Transition;
		assert(s->var.simple.transitions[i] != NULL);
		(s->var.simple.transitions[i])->state = NULL;
	}
	s->var.simple.transitions[i] = NULL;

	return(s);
}

// Create composite state
struct FSM::State *
FSM::newCompositeState(int numTransitions, char *description)
{
	struct State *s;
	int i;

	assert(numTransitions >= 0);

	s = new struct State;
	assert(s != NULL);
	s->description = description;
	s->type = COMPOSITE;
	s->var.composite.fsm = NULL;
	s->var.composite.transitions = new struct State::Transition*[numTransitions+1];
	assert(s->var.composite.transitions != NULL);
	for (i = 0; i < numTransitions; i++)
	{
		s->var.composite.transitions[i] = new struct State::Transition;
		assert(s->var.composite.transitions[i] != NULL);
		(s->var.composite.transitions[i])->state = NULL;
	}
	s->var.composite.transitions[i] = NULL;

	return(s);
}

// Create resolver state
struct FSM::State *
FSM::newResolverState(int numTransitions, char *description)
{
	struct State *s;
	int i;

	assert(numTransitions >= 0);

	s = new struct State;
	assert(s != NULL);
	s->description = description;
	s->type = RESOLVER;
	s->var.resolvers = new struct State::Var::Resolver*[numTransitions+1];
	assert(s->var.resolvers != NULL);
	for (i = 0; i < numTransitions; i++)
	{
		s->var.resolvers[i] = new struct State::Var::Resolver;
		assert(s->var.resolvers[i] != NULL);
		(s->var.resolvers[i])->transition.state = NULL;
	}
	s->var.resolvers[i] = NULL;

	return(s);
}

// Link simple/composite source state to target state
void
FSM::linkStates(struct State *fromState, struct State *toState, STATE_TRANSITION value)
{
	struct State::Transition **t;

	assert(fromState != NULL);
	assert(toState != NULL);

	switch(fromState->type)
	{

	case SIMPLE:
		assert(fromState->var.simple.transitions != NULL);
		for (t = fromState->var.simple.transitions; *t != NULL && (*t)->state != NULL; t++) {}
		assert(*t != NULL);
		(*t)->value = value;
		(*t)->state = toState;
		break;

	case COMPOSITE:
		assert(fromState->var.composite.transitions != NULL);
		for (t = fromState->var.composite.transitions; *t != NULL && (*t)->state != NULL; t++) {}
		assert(*t != NULL);
		(*t)->value = value;
		(*t)->state = toState;
		break;

	default: assert(0);

	}
}

// Link resolver source state to target state
void
FSM::linkStates(struct State *fromState, struct State *toState, double probability)
{
	struct State::Var::Resolver **r;

	assert(fromState != NULL);
	assert(fromState->type == RESOLVER);
	assert(toState != NULL);
	assert(probability >= 0.0);

	assert(fromState->var.resolvers != NULL);
	for (r = fromState->var.resolvers; *r != NULL && (*r)->transition.state != NULL; r++) {}
	assert(*r != NULL);
	(*r)->probability = probability;
	(*r)->transition.value = AUTO;
	(*r)->transition.state = toState;
}

// FSM input
void
FSM::input(STATE_TRANSITION transition)
{
	if (this->inputSub(transition) == OK)
	{
		while (this->inputSub(AUTO) == OK) {}
	}
}

// FSM input - subfunction
FSM::TRANSITION_STATUS
FSM::inputSub(STATE_TRANSITION transition)
{
	FSM::TRANSITION_STATUS status;
	struct State::Transition **t;
	struct State *s;
	struct State::Var::Resolver **r;
	double p;

	t = NULL;
	switch(this->current->type)
	{

	case COMPOSITE:
		if ((status = this->current->var.composite.fsm->inputSub(transition)) != END)
		{
			return(status);
		}
		t = this->current->var.composite.transitions;

		// fall through

	case SIMPLE:

		if (this->current == this->final) return(END);

		if (t == NULL) { t = this->current->var.simple.transitions; }
		for (; *t != NULL; t++)
		{
			if ((*t)->value == transition) break;
		}
		if (*t == NULL) return(NG);

		this->current = (*t)->state;
		break;

	case RESOLVER:
		for (r = this->current->var.resolvers, p = 0.0; *r != NULL; r++)
		{
			p += (*r)->probability;
			if (RAND_PROB < p) break;
		}
		assert(*r != NULL);

		this->current = (*r)->transition.state;
		break;
	}

	for (s = this->current; s->type == COMPOSITE; s = s->var.composite.fsm->initial)
	{
		s->var.composite.fsm->current = s->var.composite.fsm->initial;
	}

	return(OK);
}

// FSM output
FSM::STATE_VALUE *
FSM::output()
{
	static bool first = true;

	// Take possible initial resolver transitions
	if (first == true)
	{
		this->input(AUTO);
		first = false;
	}

	switch(this->current->type)
	{

	case SIMPLE:
		return(this->current->var.simple.value);

	case COMPOSITE:
		return(this->current->var.composite.fsm->output());

	default: 
		assert(0);
		return NULL;

	}
}
