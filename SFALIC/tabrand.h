#ifndef __TABRAND_H
#define __TABRAND_H


/* pseudo random number generator */
/* uses array of pseudo-random numbers */


typedef unsigned long int	trres;	/* type returned by the generator */


/* initialize the generator */
void stabrand();

/* return pseudo-random number */
trres tabrand();

#endif
