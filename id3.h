#pragma once

//#define DO_DEBUG

#ifdef DO_DEBUG
	#define	DEBUG	printf
#else
	#define	DEBUG(...) do{}while(0);
#endif

#ifdef __cplusplus
extern "C" {
#endif
/*
    Struttura dati info dataset
*/
struct dsinfo_t
{
    char            	*name;
    long            	value;
	long				column;
    struct dsinfo_t   	*next;
    struct dsinfo_t   	*prev;
};

/*
	Struttura dati foglia
*/
typedef struct node_tag
{
	long				winvalue;
	long				tot_attrib;
	long				*avail_attrib;
	long				tot_samples;
	long				*samples;
	long				tot_nodes;
	struct node_tag	*nodes;
} node_t;

//functions declerations 
int id3tree_create( char **data, long cols, long rows, ... );

void printtree( node_t *node, long cols, struct dsinfo_t *info, char **titles, long maxdepth, long maxrules );

void scantree( node_t *node, long *max_depth, long *max_rules  );
#ifdef __cplusplus
}
#endif
