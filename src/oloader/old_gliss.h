/*
 *	$Id$
 *	Copyright (c) 2007, IRIT UPS <casse@irit.fr>
 *
 *	Old GLISS definitions
 */
#ifndef _GLISS_H
#define _GLISS_H

#include <stdio.h>

#if defined(__cplusplus)
    extern  "C" {
#endif


/* Types */
typedef void state_t;
typedef unsigned long code_t;
typedef struct instruction_t_struct instruction_t;
typedef unsigned long long address_t;


/* the four main primitives (provide interface with 'simulator') */
void iss_fetch(address_t,code_t *);
instruction_t *iss_decode(state_t *, address_t,code_t *); /* allocated, must be iss_freed !*/
void iss_compute(instruction_t *,state_t *);
void iss_complete(instruction_t *,state_t *);

/* Others primitives */
/* initialize processors and extern modules (for each : void *[]) */
state_t	*iss_init(void *mem_arg_list[],void *loader[],void *system[],void *exception[],void *random[]);

/* dump into <fd> an interface (state, parameters if implemented) */
void iss_dump(FILE *fd,state_t *state);

/* free a decoded-instruction */
void iss_free(instruction_t *);
/* halt the iss (free all memory) */
void iss_halt(state_t *);

/* disasm part */
void iss_disasm(char *,instruction_t *);

/* for intern/extern functions */
void iss_error(char *msg,...); /* formated error message, printf-like */


#if defined(__cplusplus)
    }
#endif


#endif	// _GLISS_H
