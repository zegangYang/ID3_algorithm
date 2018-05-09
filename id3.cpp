#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

#include "id3.h"

#include <map>
#include <iostream>
#include <queue>

using namespace std;

char data_set_g[DATA_SET_BUFFER_SIZE];
char *data_set_p_g[DATA_SET_NUM_DEFAULT];
int result_ok_index_g = -1;
int result_failed_index_g = -1;

#ifdef SAMPLE_WEATHER
    #define STR_OK "YES"
    #define STR_FAILED "NO"
#endif

#ifdef SAMPLE_CANCER
    #define STR_OK "J2"
    #define STR_FAILED "J4"
#endif
uint32_t data_set_p_index = 0;
/*
	Prima scansione dell'albero di decisione per raccoglioere
	info riguardo alla profondita' massima dei rami e al number
	massimo di Rules create
*/
void scantree( node_t *node, long *max_depth, long *max_rules  )
{
	static int depth = 0;
	int j, i;

	if( node != NULL )
	{
		depth += 1;

		// store the depth' max of branches
		if( depth > *max_depth )
		{
			*max_depth = depth;
		}
		// store the number max of rules found
		if( node->tot_nodes == 0 )
		{
			*max_rules += 1;
		}
		/*
		printf( "Current node @ %p:\n", node );
		printf( "\twinvalue        : %d\n", node->winvalue );
		printf( "\ttot_attrib        : %d\n", node->tot_attrib );
		if(node->tot_samples > 0)
		{
			printf( "\ttot_samples     : %d\n", node->tot_samples );
			printf( "\tsamples         : " );
			for( i = 0; i < node->tot_samples; i++ )
				printf( "%-2d ", node->samples[ i ] );
		}
		
		if(node->tot_attrib > 0)
		{
			
			printf( "\n\ttot_attrib      : %d (%d %d %d %d )\n", node->tot_attrib, node->avail_attrib[0],node->avail_attrib[1],node->avail_attrib[2],node->avail_attrib[3] );
			
		}
		printf( "\ttot_nodes       : %d\n", node->tot_nodes );
		printf( "\tnodes           @ %p\n\n\n", node->nodes );*/
		j = 0;
		while( j < node->tot_nodes )
		{
			scantree( node->nodes+j, max_depth, max_rules );

			depth -= 1;
			++j;
		}
	}
}

/*
	Seconda scansione dell'albero di decisioni per la raccolta of
	Rules per every classe
*/
void scanrules( node_t *node, long class_id, long *depth, long *path, long maxdepth, long *table, long *tid )
{
	int j, i;

	if( node != NULL )
	{
		*depth += 1;

		// update the path current
		*( path + ( *depth - 1 ) ) = node->winvalue;

		// Is the last Node Leaf branch
		if( node->tot_nodes == 0 && node->winvalue == class_id )
		{
			for( i = 0; i < *(depth)-1; i++ )
				*( table + ((*tid)*maxdepth) + i ) = path[ i ];
				*(tid)	+=1;
		}

		j = 0;
		while( j < node->tot_nodes )
		{
			scanrules( node->nodes+j, class_id, depth, path, maxdepth, table, tid );

			*depth -= 1;
			++j;
		}
	}
}
/*
	Extraction of Rules content In the decision tree
*/
void explain_rules( node_t *node, long cols, struct dsinfo_t *info, char **titles, long maxdepth, long maxrules )
{
	struct dsinfo_t 	*infoptr 		= info;
	struct dsinfo_t 	*infoptr2 		= NULL;
	long				*rules_table	= NULL;
	long				tableins_id		= 0;
	long				rulestable_sz	= 0;
	long				*temp_path		= NULL;
	long				attrb			= 0;
	long				attrb_id		= 0;
	long				*attrb_name		= 0;
	long				depth			= 0;
	long				i, j, k;

	// allocazione memoria per contenere le Rules
	rulestable_sz 	= sizeof( long ) * maxdepth * maxrules;
	rules_table 	= (long *)malloc( rulestable_sz );
	temp_path 		= (long *)malloc( sizeof( long ) * maxdepth );

	printf( "Rules found:\n\n");
	while( infoptr->next != NULL )
	{
		if( infoptr->column == ( cols - 1 ) )
		{
			printf( "Class %s\n", infoptr->name );

			i = 0;
			while( i < ( maxdepth * maxrules ) )
			{
				*( rules_table + i ) = -1;
				++i;
			}

			for( i = 0; i < maxdepth; i++ )	temp_path[ i ] = -1;
			depth 		= 0;
			tableins_id = 0;
			scanrules( node, infoptr->value, &depth, temp_path, maxdepth, rules_table, &tableins_id );


			/*
				Classe (4): NO
								-1  0  2 -1
								-1  8  5 -1
								-1 -1 -1 -1
								-1 -1 -1 -1
								-1 -1 -1 -1
				Classe (7): YES
								-1  0 11 -1
								-1  6 -1 -1
								-1  8  3 -1
								-1 -1 -1 -1
								-1 -1 -1 -1
			*/
			// stampa le Rules trovate per la classe corrente
			printf("\t\t");
			for( i = 0; i < maxrules; i++ )
			{
				for( j = 0; j < (maxdepth-1); j++ )
				{
					attrb 		= *( rules_table + i*maxdepth + j );
					if( attrb >= 0 )
					{
						attrb_id 	= 0;
						infoptr2 	= info;
						while( infoptr2 != NULL )
						{
							if( attrb == infoptr2->value )
							{
								attrb_id 	= infoptr2->column;
								attrb_name 	= (long *)infoptr2->name;
								break;
							}
							infoptr2 = infoptr2->next;
						}
						printf( "Is %s = %s ", *( titles + attrb_id ), attrb_name );
						if( *( rules_table + i*maxdepth + j+1 ) >= 0 )
							printf( "and " );
						else
							printf( "\n\t\t" );
					}
				}
			}
			printf("\n");

		}
		infoptr = infoptr->next;
	}

	free( temp_path );
	free( rules_table );
}

/*
	Calculation Entropy Portion di samples
	- data: 		puntatore all'intero DataBase
	- cols:			number di colonne DB (attributi + classi)
	- sample:		vettore contenente gli indici dei samples da analizzare
	- totsamples:	totale samples da analizzare
	- info:			informazioni su classi/atttributi
*/
double calc_entropy_set( long *data, long cols, long *samples, long totsamples, struct dsinfo_t *info )
{
	double 			entropy		= 0;
	double				part		= 0;
	long				total		= 0;
	struct dsinfo_t  	*infoptr	= NULL;
	long				j;

	// cerco all'interno della infolist gli indici of classi
	infoptr = info;
	while( infoptr != NULL )
	{
		// quando trovo una classe...
		if( infoptr->column == ( cols - 1 ) )
		{
			// ne Calculation l'Entropy sulla porzione di database indicata da samples
			// samples contiene gli indici dei sample da analizzare percui
			// data[ samples[ j ]*cols + cols - 1 ] contiene il Value della classe (ultima colonna)
			// of elemento del DB con indice indicato da samples[ j ]

			total = 0;
			for( j = 0; j < totsamples; j++ )
				if( data[ samples[ j ]*cols + cols - 1 ] == infoptr->value ) ++total;

			// Calculation il rapporto su cui eseguire la formula
			if( total > 0 && totsamples > 0 )
			{
				part	= (double)total / (double)totsamples;

				// sommo all'Entropy totale l'Entropy per questa classe secondo
				// la formula	Entropy = -p(I) log2( p(I) )
				entropy += ( -part * log2(part) );
			}
		}
		// continuo la ricerca nelle info
		infoptr = infoptr->next;
	}

	return entropy;
}


/*
	Calculation Info gain per un Attribute
*/
double calc_attrib_gain( long *data, long cols, long *samples, long totsamples, struct dsinfo_t *info, long attrib )
{
	struct dsinfo_t  	*infoptr 		= NULL;
	long				tot_attribtype 	= 0;
	long				tot_classtype	= 0;
	double 			gain 			= 0;
	double				vpcgain			= 0;
	double				part			= 0;
	long				size			= 0;
	long				attrvalue		= 0;
	long				*classlist		= NULL;
	long				i = 0, j, k;

	struct vpc_t
	{
		long			class_id;
		long			tot_found;
	};

	struct gdata_t
	{
		long			value;
		long			tot_found;
		struct vpc_t	*vpc;
	};
	struct gdata_t		*gdata, *gdataptr;
	struct	vpc_t		*vpcptr;

	// Calculation totale valori possibili per Attribute e classi
	infoptr = info;
	while( infoptr != NULL )
	{
		// conteggio totale valori possibili per l'Attribute
		if( infoptr->column == attrib ) 		++tot_attribtype;
		// conteggio totale valori possibili per le classi
		if( infoptr->column == ( cols - 1 ) ) 	++tot_classtype;
		// proseguo la ricerca
		infoptr = infoptr->next;
	}

	// classlist diventa un vettore contenente tutte le possibili classi
	classlist = (long *)malloc( sizeof( long ) * tot_classtype );
	infoptr = info , i = 0;
	while( infoptr != NULL )
	{
		if( infoptr->column == ( cols - 1 ) ) *( classlist + i++ ) = infoptr->value;
		infoptr = infoptr->next;
	}

	// allocazione memoria per le strutture per every tipo di Value dell'Attribute
	size 	= sizeof( struct gdata_t ) * tot_attribtype;
	gdata 	= (struct gdata_t *)malloc( size );
	memset( gdata, 0, size );

	// inizializzazione struttura per every Value dell'Attribute
	i = 0, infoptr = info;
	while( infoptr != NULL )
	{
		if( infoptr->column == attrib )
		{
			gdataptr 				= gdata + i;
			gdataptr->value 		= infoptr->value;
			gdataptr->tot_found 	= 0;

			size = sizeof( struct vpc_t ) * tot_classtype;
			gdataptr->vpc 			= (struct vpc_t *)malloc( size );

			for( j = 0; j < tot_classtype; j++ )
			{
				vpcptr 				= gdataptr->vpc + j;
				vpcptr->class_id	= *( classlist + j );
				vpcptr->tot_found	= 0;
			}
			++i;
		}
		// proseguo la ricerca
		infoptr = infoptr->next;
	}

	// raccolta dati dai samples riguardo al number di valori per every
	// Attribute; per every Value viene calcolato inoltre quanti corrispondono
	// a una classe piuttosto che ad un'altra
	for( i = 0; i < totsamples; i++ )
	{
		for( j = 0; j < tot_attribtype; j++ )
		{

			gdataptr = gdata + j;
			if( gdataptr->value == data[ samples[ i ]*cols + attrib ] )
			{
				gdataptr->tot_found += 1;
				for( k = 0; k < tot_classtype; k++ )
				{
					vpcptr = gdataptr->vpc;
					if( data[ samples[ i ]*cols + cols - 1 ] == ( vpcptr+k )->class_id )
							( vpcptr+k )->tot_found += 1;
				}
			}
		}
	}

	// Calculation information gain
	for( i = 0; i < tot_attribtype; i++ )
	{
		gdataptr 	= gdata + i;
		vpcgain		= 0;

		for( j = 0; j < tot_classtype; j++ )
		{
			vpcptr 	= 	gdataptr->vpc + j;
			if( vpcptr->tot_found > 0 && gdataptr->tot_found > 0 )
			{
				part	= 	(double)vpcptr->tot_found / (double)gdataptr->tot_found;
				vpcgain +=	( -( part ) * log2( part ) );
			}
 		}
		if( gdataptr->tot_found > 0 && totsamples > 0 )
		{
			part	= (double) gdataptr->tot_found / (double) totsamples;
			gain 	+= ( -( part ) * vpcgain );
		}
	}

	// Libera tutta la memoria allocata
	for( i = 0; i < tot_attribtype; i++ )
	{
		gdataptr = gdata + i;
		free( gdataptr->vpc );
	}
	free( gdata );
	free( classlist );

	return 	gain;
}

/*
	Creazione nodes albero
*/
void create_leaves( node_t *node, long *data, long cols, long rows, struct dsinfo_t *info )
{

	struct dsinfo_t  	*infoptr 		= NULL;
	double 			entropy_set 	= 0;
	double				*gains			= NULL;
	double				max_gain		= 0;
	long				max_gain_id		= 0;
	long				gbuf_sz			= 0;
	long				max_attr_values	= 0;
	long				tot_new_samples	= 0;
	long				tot_avattrib	= 0;
	long				*sampleptr		= NULL;
	node_t				*node_ptr		= NULL;
	node_t				*new_node		= NULL;
	long				j, i;

	struct smplid_t
	{
		long 				value;
		struct smplid_t 	*next;
		struct smplid_t 	*prev;
	};

	struct smplid_t	*samplelist		= NULL;
	struct smplid_t	*samplelistptr	= NULL;
	struct smplid_t	*samplelistprv	= NULL;


	DEBUG( "Current node @ %p:\n", node );
	DEBUG( "\twinvalue        : %d\n", node->winvalue );
	//DEBUG( "\ttot_samples     : %d\n", node->tot_samples );
	//DEBUG( "\tsamples         : " );
	//for( i = 0; i < node->tot_samples; i++ )
		//DEBUG( "%-2d ", node->samples[ i ] );
	//DEBUG( "\n\ttot_attrib      : %d (%d %d %d %d )\n", node->tot_attrib, node->avail_attrib[0],node->avail_attrib[1],node->avail_attrib[2],node->avail_attrib[3] );
	//DEBUG( "\ttot_nodes       : %d\n", node->tot_nodes );
	//DEBUG( "\tnodes           @ %p\n", node->nodes );


	// Calculation Entropy of part of samples da Examine
	entropy_set = calc_entropy_set( data, cols, node->samples, node->tot_samples, info );

	DEBUG( "Entropy set = %3.6f\n", entropy_set );
    
    
	// Il Value di entropy_set e' fondamentale per proseguire o meno nella crezione
	// dei rami e dei nodes foglia. Se il suo Value e' 0 significa che gli elementi
	// esaminati sono prerfettamente classificati, se il suo Value e' significa che
	// gli elementi non hanno Rules, sono totalmente casuali.
	// Se invece il Value e' compreso tra 0 e 1 proseguo e Calculation il Gain per every
	// Attribute disponibile..
	if( entropy_set == 0.000f )
	{

		node->nodes 				= (node_t *)malloc( sizeof( node_t ) );
		node->tot_nodes				= 1;
		node->nodes->tot_nodes 		= 0;
		node->nodes->winvalue 		= data[ node->samples[ 0 ] * cols + cols - 1 ];

		node->nodes->tot_attrib		= 0;
		node->nodes->avail_attrib	= NULL;
		node->nodes->tot_samples	= 0;
		node->nodes->samples		= NULL;
		node->nodes->nodes			= NULL;

		DEBUG( "\t\t\tNode Terminal @ %p:\n", node->nodes );
		DEBUG( "\t\t\twinvalue        : %d\n", node->nodes->winvalue );
		DEBUG( "\t\t\ttot_samples     : %d\n", node->nodes->tot_samples );
		DEBUG( "\t\t\ttot_attrib      : %d\n", node->nodes->tot_attrib );
		DEBUG( "\t\t\ttot_nodes       : %d\n", node->nodes->tot_nodes );
		DEBUG( "\t\t\tnodes           @ %p\n", node->nodes->nodes );
	}
	else if( entropy_set == 1 )
	{
		// Dati totalmente casuali, nessuna regola
	}
	else
	{
		// calcola il totale degli attributi su cui calcolare...
		tot_avattrib = 0;
		for( j = 0; j < ( cols - 1 ); j++ )
			if( node->avail_attrib[ j ] == 1 ) tot_avattrib += 1;

		DEBUG( "\tCalculation Entropy per every Attribute ( tot. Available %d )\n", tot_avattrib );
		// se c'e' piu' di un Attribute disponibile
		if( tot_avattrib > 0 )
		{

			// allocazione memoria per il buffer di dimensione n. attributi
			// all'interno del buffer
			gains = (double *)malloc( sizeof( double ) * ( cols - 1 ) );
			for( i = 0; i < ( cols - 1 ); i++ ) gains[ i ] = 0;

			for( j = 0; j < ( cols - 1 ); j++ )
				if( node->avail_attrib[ j ] == 1 )
				{
					gains[ j ] = entropy_set + calc_attrib_gain( data, cols, node->samples, node->tot_samples, info, j );
					DEBUG( "\tInfo Gain per Attribute %d = %3.3f\n", j, gains[ j ] );
				}
			// cerca il Value piu' alto...
			for( j = 0; j < ( cols - 1 ); j++ )
				if( gains[ j ] > max_gain )
				{
					max_gain	= gains[ j ];
					max_gain_id = j;
				}

			// calcola il number massimo possibile di valori per l'Attribute vincente
			max_attr_values = 0;
			infoptr 		= info;
			while( infoptr != NULL )
			{
				if( infoptr->column == max_gain_id ) ++max_attr_values;
				infoptr = infoptr->next;
			}
			DEBUG( "\tL'Attribute %d ha il massimo IG (%3.3f) e %d tipi di valori\n", max_gain_id, max_gain, max_attr_values );

			// crea i nodes per every Value possibile dell'Attribute
			// il number dei nodes e' pari a tutti i valori possibili per l'Attribute
			node->nodes 	= ( node_t* ) malloc( sizeof( node_t ) * max_attr_values );
			node->tot_nodes = max_attr_values;
			DEBUG( "\tAllocazione memoria per %d nodes @ %p\n", max_attr_values, node->nodes );

			infoptr 		= info;
			j = 0;
			while( infoptr != NULL )
			{
				if( infoptr->column == max_gain_id )
				{
					DEBUG( "\t\tSetting Node per Value %d dell'Attribute %d\n", infoptr->value, max_gain_id );

					node_ptr 	= node->nodes;
					node_ptr 	+= j;
					DEBUG( "\t\t\tnode_ptr = %p ( j = %d )\n", node_ptr, j );

					tot_new_samples = 0;
					// cercare nei sample del DB indicati da node->samples tutti
					// quelli che nella colonna indicata da max_gain_id hanno il Value indicato
					// da infoptr->value, calcolarne il totale e metterlo in tot_samples
					// creare un vettore della dimensione di tot_samples e assegnarlo a node_ptr->samples
					for( i = 0; i < node->tot_samples; i++ )
					{
						if( data[ node->samples[ i ] * cols + max_gain_id ] == infoptr->value )
						{
							if( samplelist == NULL )
							{
								samplelist 				= (struct smplid_t	*)malloc( sizeof( struct smplid_t ) );
								samplelist->value 		= node->samples[ i ];
								samplelist->next		= NULL;
								samplelist->prev		= NULL;
							}
							else
							{
								samplelistptr				= samplelist;
								while( samplelistptr->next != NULL ) samplelistptr = samplelistptr->next;
								samplelistptr->next			= (struct smplid_t *)malloc( sizeof( struct smplid_t ) );
								samplelistptr->next->prev 	= samplelistptr;
								samplelistptr 				= samplelistptr->next;
								samplelistptr->value 		= node->samples[ i ];
								samplelistptr->next			= NULL;
							}
							tot_new_samples += 1;
						}
					}

					node_ptr->winvalue		= infoptr->value;
					node_ptr->tot_nodes 	= 0;
					node_ptr->tot_samples 	= tot_new_samples;
					node_ptr->samples		= (long *)malloc( sizeof( long ) * tot_new_samples );
					sampleptr				= node_ptr->samples;

					samplelistptr			= samplelist;
					while( samplelistptr != NULL )
					{
						*( sampleptr++ ) 	= samplelistptr->value;
						samplelistptr 		= samplelistptr->next;
					}
					// una volta inseriti gli indici dei nuovi sample in test nel vettore
					// puntato da node_ptr->samples posso distruggere la lista temporanea
					samplelistptr			= samplelist;
					samplelistprv			= samplelist;
					while( samplelistptr != NULL )
					{
						samplelistprv = samplelistptr->next;
						free( samplelistptr );
						samplelistptr = samplelistprv;
					}
					samplelist = NULL;

					node_ptr->tot_attrib 	= ( cols - 1 );
					node_ptr->avail_attrib	= (long *)malloc( sizeof( long ) * ( cols - 1 ) );

					for( i = 0; i < cols-1; i++ )
						node_ptr->avail_attrib[ i ] = node->avail_attrib[ i ];
					node_ptr->avail_attrib[ max_gain_id ] = 0;

					DEBUG( "\t\t\tnode_ptr->winvalue    : %d\n", node_ptr->winvalue );
					DEBUG( "\t\t\tnode_ptr->tot_samples : %d\n", node_ptr->tot_samples );
					DEBUG( "\t\t\tnode_ptr->samples     : %p\n", node_ptr->samples );


					// creazione ricorsiva dei nodes foglia
					if( node_ptr->tot_samples > 0 ) create_leaves( node_ptr, data, cols, rows, info );


					++j;
				}
				infoptr = infoptr->next;
			}
			free( gains );
		}
		else
		{
			node->nodes 				= (node_t *)malloc( sizeof( node_t ) );
			node->tot_nodes				= 1;
			node->nodes->tot_nodes 		= 0;
			node->nodes->winvalue 		= data[ node->samples[ 0 ] * cols + cols - 1 ];

			node->nodes->tot_attrib		= 0;
			node->nodes->avail_attrib	= NULL;
			node->nodes->tot_samples	= 0;
			node->nodes->samples		= NULL;
			node->nodes->nodes			= NULL;

			DEBUG( "\t\t\tNode Terminal @ %p:\n", node->nodes );
			DEBUG( "\t\t\twinvalue        : %d\n", node->nodes->winvalue );
			DEBUG( "\t\t\ttot_samples     : %d\n", node->nodes->tot_samples );
			DEBUG( "\t\t\ttot_attrib      : %d\n", node->nodes->tot_attrib );
			DEBUG( "\t\t\ttot_nodes       : %d\n", node->nodes->tot_nodes );
			DEBUG( "\t\t\tnodes           @ %p\n", node->nodes->nodes );
		}
	}
}



/*
	Creazione Rules
*/
int id3tree_create( char **data, long cols, long rows, ... )
{
    long				*dataset		= NULL;
	unsigned long		dataset_sz 		= 0;
	struct dsinfo_t  	*infolist		= NULL;
	struct dsinfo_t  	*insptr 		= NULL;
	struct dsinfo_t  	*prvptr 		= NULL;
	struct dsinfo_t  	*prvass 		= NULL;
	char				label_found		= 0;
	char				infolisterror	= 0;
	long				string_id		= 0;
	long				assign_id		= 0;
	node_t		*root			= NULL;

	va_list				llistptr;
	char				**cols_titles	= NULL;
	char				**ctptr			= NULL;
	char				*label;
	long				totlabels		= 0;

	long				tree_max_depth	= 0;
	long				tree_max_rules	= 0;


	int					result			= 0;
	int 				i = 0, j = 0, col = 0;

	DEBUG( "ID3 Init: cols = %d rows = %d dataset %p\n", cols, rows, data );

	do {

		// Riempimento della lista di puntatori a stringa contenenti
		// le etichette (titoli) per every Attribute
		va_start( llistptr, rows );
		do {
			label = va_arg( llistptr, char* );
			if( label != NULL ) totlabels += 1;
		} while( label != NULL );
		va_end( llistptr );

		cols_titles = (char **)malloc( sizeof( char* )*totlabels );
		// controllo se posso allocare memoria per le labels
		if( cols_titles == NULL )
		{
			result = -1;
			break;
		}
		// riempio il vettore con dimensione pari al number di colonne del database appena creato
		ctptr = cols_titles;
		va_start( llistptr, rows );
		for( i = 0; i < totlabels; i++ )
		{
			label 	= va_arg( llistptr, char* );
			*ctptr 	= label;
			++ctptr;
		}
		va_end( llistptr );

		DEBUG( "Columns labels:\n" );
		for( i = 0; i < totlabels; i++ )
			DEBUG( "Label %3d = %s\n", i, cols_titles[ i ] );


		// Essendo la comparazione di valori long piu' veloce rispetto alla
		// comparazione di stringhe l'intero data set costituito da stringhe viene
		// convertito assegnando un indice univoco ad every stringa
		// Calculation quantita' di memoria necessaria per tabella di conversione
		dataset_sz = sizeof( long ) * cols * rows;

		// Allocazione memoria per la conversione stringa->value
		if( ( dataset = (long *)malloc( dataset_sz ) ) == NULL )
		{
			result = -2;
			break;
		}
		// azzero la tabella completamente
		memset( dataset, 0, dataset_sz );


		// L'intero data set (in stringa) viene scorso completamente per
		// recuperare tutte le informazioni necessarie ai calcoli per la creazione
		// dell'albero of Rules
		i = 0, col = 0;
		while( i < (cols*rows) )
		{
			// se infolist e' NULL significa che l'elemento va ovviamente inserito nella lista
			insptr = NULL;
			if( infolist == NULL )
			{
				infolist 	= (struct dsinfo_t *)malloc( sizeof( struct dsinfo_t ) );
				if( infolist == NULL )
				{
					infolisterror = 1;
					break;
				}
				// TODO inserire controllo Value infolist
				insptr		= infolist;
				prvass		= NULL;
			}
			else
			{
				// altrimenti viene cercato fra tutti quelli trovati finora. se non compare
				// nella lista viene aggiunto
				insptr 		= infolist;
				prvptr		= infolist;
				label_found	= 0;
				do {
					if( !strcmp( insptr->name, data[ i ] ) )
					{
						label_found = 1;
						assign_id	= insptr->value;
						insptr		= NULL;
						break;
					}
					prvptr	= insptr;
					insptr 	= insptr->next;
				} while( insptr != NULL );

				if( label_found == 0 )
				{
					prvptr->next 	= (struct dsinfo_t *)malloc( sizeof( struct dsinfo_t ) );
					if( prvptr->next == NULL )
					{
						infolisterror = 1;
						break;
					}
					insptr			= prvptr->next;
					prvass			= prvptr;
				}
			}
			// se insptr e' diverso da NULL significa che punta a una zona di memoria
			// gia' allocata per poter contenere i dati sulla nuova etichetta
			if( insptr != NULL )
			{
				assign_id		= string_id;
				insptr->name 	= (char *)malloc( sizeof( char )*strlen( data[ i ] ) + 1 );
				if( insptr->name == NULL )
				{
					infolisterror = 1;
					break;
				}

				sprintf( insptr->name, data[ i ] );
				insptr->value	= string_id++;
				insptr->column	= col;
				insptr->next	= NULL;
				insptr->prev	= prvass;
			}

			// aggiorno il corrispondente long nella tabella di conversione string->long
			dataset[ i ] = assign_id;


			// la variabile col tiene conto della colonna corrente all'interno del dataset
			// in caso di accodamento tiene traccia dell'Attribute/classe a cui appartiene l'elemento
			if( ++col >= cols ) col = 0;
			// incremento indice di scorrimento elementi nel dataset
			i += 1;
		}
		// controllo se ci sono stati errori di allocazione memoria durante la creazione del dataset
		if( infolisterror )
		{
			result = -3;
			break;
		}


		// creazione del Node radice: da qui parte la creazione dell'intero albero
		if( ( root = ( node_t* ) malloc( sizeof( node_t ) ) ) == NULL )
		{
			result = -4;
			break;
		}
		// essendo alla radice il set di samples da esaminare e' l'intero albero
		root->tot_samples	= rows;
		// creo un vettore contenente gli indici ( da 0 a row - 1 ) di tutti i samples da esaminare
		if( ( root->samples = (long *)malloc( sizeof( long ) * rows ) ) == NULL )
		{
			result = -5;
			break;
		}
		// il Node radice contiene gli indici di tutti i samples del database
		for( j = 0; j < rows; j++ ) root->samples[ j ] = j;

		// imposto tutti gli attributi possibili ( tutte le colonne meno una, quella of classi )
		root->tot_attrib = ( cols - 1 );
		// tutti gli attributi ( cols - 1 ) devono essere presi in considerazione
		if( ( root->avail_attrib	= (long *)malloc( sizeof( long ) * ( cols - 1 ) ) ) == NULL )
		{
			result = -6;
			break;
		}
		// al Node radice tutti gli attributi sono da controllare
		for( j = 0; j < ( cols - 1 ); j++ ) root->avail_attrib[ j ] = 1;

		// Value (-1) che identifica il Node radice, inoltre il Node non ha (all'inizio) sotto nodes
		root->winvalue		= -1;
		root->tot_nodes		= 0;

		DEBUG( "Root node @ %p:\n", root );
		DEBUG( "\twinvalue        : %d\n", root->winvalue );
		DEBUG( "\ttot_samples     : %d\n", root->tot_samples );
		DEBUG( "\tsamples         : " );
		for( i = 0; i < root->tot_samples; i++ )
			DEBUG( "%2d ", root->samples[ i ] );
		DEBUG( "\n\ttot_attrib      : %d (%d %d %d %d )\n", root->tot_attrib, root->avail_attrib[0],root->avail_attrib[1],root->avail_attrib[2],root->avail_attrib[3] );
		DEBUG( "\ttot_nodes       : %d\n", root->tot_nodes );
		DEBUG( "\tnodes           @ %p\n", root->nodes );


		// Creating tree and node leaf
		create_leaves( root, dataset, cols, rows, infolist );
		
		// view tree
		scantree( root, &tree_max_depth, &tree_max_rules );
        
        init_show_parameters(root, cols, infolist, cols_titles);
		//print tree
		printtree(root, cols, infolist, cols_titles, 0, tree_max_rules);
        
        printf("\n\n\n\n\n winvalue对应的属性以及Titles\n");
        printAttribute(root, cols, infolist, cols_titles, 0, tree_max_rules);
		//show_tree(root);
        // Explanation of rules
		explain_rules( root, cols, infolist, cols_titles, tree_max_depth, tree_max_rules );

	} while( 0 );

	// TODO Libero memoria albero

	// Libero memoria info dataset
	insptr = infolist;
	while( insptr != NULL )
	{
		prvass = insptr->next;
		free( insptr );
		insptr = prvass;
	}
	// Libero memoria allocata per la tabella conversione stringa->Value
	if( dataset != NULL ) 	free( dataset );
	// Libero memoria allocata per etichette attributi/classi
	if( cols_titles != NULL ) free( cols_titles );

	return result;
}
/*
 *获取结果的索引 
 * 方式为遍历生成的决策树
 *  
*/
void init_show_parameters(node_t *node, long cols, struct dsinfo_t *info, char **titles)
{
    struct dsinfo_t *infoptr= info;
    int j = 0;
    if(result_ok_index_g != -1 && result_failed_index_g != -1)
        return;
    if( node != NULL )
	{
        while( infoptr->next != NULL)
        {
            if( infoptr->value == node->winvalue )
            {
                break;                
            }
            infoptr = infoptr->next;
        }
        if(result_ok_index_g == -1)
        {
            if(strcmp(STR_OK,infoptr->name) == 0)
            {
                result_ok_index_g = infoptr->value;
            }
        }
        if(result_failed_index_g == -1)
        {
            if(strcmp(STR_FAILED,infoptr->name) == 0)
            {
                result_failed_index_g = infoptr->value;
            }
        }
        //#endif
        
		while( j < node->tot_nodes )
		{
            init_show_parameters(node->nodes+j,cols,info,titles);
            ++j;
		}
	}
}
void printtree( node_t *node, long cols, struct dsinfo_t *info, char **titles, long maxdepth, long maxrules )
{
	struct dsinfo_t *infoptr= info;
    int j = 0,i;
    if( node != NULL )
	{
        
       //#ifdef PRINT_MODE2
        while( infoptr->next != NULL)
        {
            if( infoptr->value == node->winvalue )
            {
                //printf("%s:",*(titles+infoptr->column));
                break;                
            }
            infoptr = infoptr->next;
        }
        //#endif
        
        #ifdef PRINT_MODE1
            
            //printf("%s(%s)\n",*(titles+infoptr->column),infoptr->name);            
            //printf("%d\n",node->winvalue);
            if(node->tot_nodes == 1 && (node->nodes->winvalue == result_ok_index_g || node->nodes->winvalue == result_failed_index_g))
            {
                if(node->winvalue >= 0 && node->nodes->winvalue == result_ok_index_g)
                    printf("%s(%s) :结果:(%s)\n",*(titles+infoptr->column),infoptr->name,STR_OK);
                else if(node->winvalue >= 0 && node->nodes->winvalue == result_failed_index_g)
                    printf("%s(%s) :结果:(%s)\n",*(titles+infoptr->column),infoptr->name,STR_FAILED);                    
                return;
            }
            else
            {
                if(node->winvalue >= 0)
                    printf("%s(%s)\n",*(titles+infoptr->column),infoptr->name);
            }
        #endif
            
		while( j < node->tot_nodes )
		{
            for (int i = 0; i < maxdepth; i++)
                printf("\t");
            printtree(node->nodes+j,cols,info,titles,maxdepth+1,maxrules);
            ++j;
		}
        /*while( infoptr->next != NULL )
        {
            //printf("name %s value %d infoptr->column %d title = %s \n",infoptr->name,infoptr->value,infoptr->column,*(titles+infoptr->column));
            if( infoptr->column == ( cols - 1 ) )
            {
            }
            infoptr = infoptr->next;
        }*/
	}
}


int my_itoa(int val, char* buf)  
{  
    const unsigned int radix = 10;  
  
    char* p;  
    unsigned int a;        //every digit  
    int len;  
    char* b;            //start of the digit char  
    char temp;  
    unsigned int u;  
  
    p = buf;  
  
    if (val < 0)  
    {  
        *p++ = '-';  
        val = 0 - val;  
    }  
    u = (unsigned int)val;  
  
    b = p;  
  
    do  
    {  
        a = u % radix;  
        u /= radix;  
  
        *p++ = a + '0';  
  
    } while (u > 0);  
  
    len = (int)(p - buf);  
  
    *p-- = 0;  
  
    //swap  
    do  
    {  
        temp = *p;  
        *p = *b;  
        *b = temp;  
        --p;  
        ++b;  
  
    } while (b < p);  
  
    return len;  
}
int load_cancer_data(char *path,int &rows)
{
    FILE *fp = NULL;
    char *data_buffer = data_set_g;
    
    fp = fopen(path,"r");
    if(!fp)
        return -1;
    while(!feof(fp))
    {
        uint32_t temp[11];
        int ret  = fscanf(fp,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",temp,temp+1,temp+2,temp+3,temp+4,temp+5,
                                                                        temp+6,temp+7,temp+8,temp+9,temp+10);
        for(int i=1;i<11;i++)
        {
            //int bytes = my_itoa(temp[i],data_buffer) + 1;
            int bytes = sprintf(data_buffer,"%c%d",i+64,temp[i])+1;
            data_set_p_g[data_set_p_index++] = data_buffer;
            data_buffer += bytes;
        }
        rows++;
    }
    
    return data_set_p_index;
}


void printAttribute( node_t *node, long cols, struct dsinfo_t *info, char **titles, long maxdepth, long maxrules )
{
    struct dsinfo_t *infoptr= info;
    int j = 0,i;
    if( node != NULL )
	{
        while( infoptr->next != NULL)
        {
            if( infoptr->value == node->winvalue )
            {
                break;                
            }
            infoptr = infoptr->next;
        }
        
        #ifdef PRINT_MODE1
            if(node->winvalue >= 0)
                printf("算法中的value %d  = %s(%s)\n",node->winvalue,*(titles+infoptr->column),infoptr->name);
        #endif
            
		while( j < node->tot_nodes )
		{
            printAttribute(node->nodes+j,cols,info,titles,maxdepth+1,maxrules);
            ++j;
		}
	}
}








