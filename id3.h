#pragma once

//#define DO_DEBUG
//打印数字结果 标签提示一次 好分辨树的层次
#define PRINT_MODE1
//打印label结果，格式乱
//#define PRINT_MODE2

#define DATA_SET_BUFFER_SIZE 1024*1024*10
#define DATA_SET_NUM_DEFAULT 1024*10

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

//show tree
void show_tree(node_t *node);
/*
 * 找出决策树中所有节点儿子最多的个数
 */
void get_max_ch_len(node_t* root, int& result);

void init_tree(node_t* root, int& max_ch_len, int layer);

int load_cancer_data(char *path,int &cols);
