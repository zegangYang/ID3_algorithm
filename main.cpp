#include <stdio.h>
#include <stdlib.h>

#include "id3.h"

#include <map>
#include <iostream>

using namespace std;

#define DATA_PATH "test.csv"
char *dataset_weather[] =
{
	"SUNNY",  	"HOT",    	"HIGH",    	"WEAK",		"NO",
	"SUNNY",	"HOT",    	"HIGH",    	"STRONG",	"NO",
	"OVERCAST",	"HOT",    	"HIGH",   	"WEAK",		"YES",
	"RAIN",		"MILD",   	"HIGH",   	"WEAK",		"YES",
	"RAIN",		"COOL",		"NORMAL",  	"WEAK",		"YES",
	"RAIN",		"COOL",  	"NORMAL",  	"STRONG",	"NO",
	"OVERCAST",	"COOL",  	"NORMAL",  	"STRONG",	"YES",
	"SUNNY", 	"MILD",  	"HIGH",    	"WEAK",		"NO",
	"SUNNY", 	"COOL",  	"NORMAL",  	"WEAK",		"YES",
	"RAIN",		"MILD",  	"NORMAL",  	"WEAK",		"YES",
	"SUNNY", 	"MILD",  	"NORMAL",  	"STRONG",	"YES",
	"OVERCAST",	"MILD",  	"HIGH",    	"STRONG",	"YES",
	"OVERCAST",	"HOT", 		"NORMAL",  	"WEAK",		"YES",
	"RAIN",  	"MILD",  	"HIGH",		"STRONG",	"NO",
	NULL
};
extern char *data_set_p_g[];
extern uint32_t data_set_p_index;
int main()
{    
#ifdef SAMPLE_WEATHER
    int result = 0;
    
    result = id3tree_create( 	dataset_weather,			
								5,					
								14,					
								"天气",			
								"温度",		
								"湿度",			
								"风",				
								"结果",			
								NULL
							);
#endif

#ifdef SAMPLE_CANCER
    char **cancer_data = NULL;
    int rows = 0;
    int result = 0;

    int ret = load_cancer_data(DATA_PATH,rows);
    result = id3tree_create( 	data_set_p_g,			
								10,					
								rows,					
								"厚度"		
								"大小",		
								"附力",			
								"尺寸",				
								"结果",
                                "裸核"		
								"染色质",		
								"核仁",			
								"核分裂",				
								"Class",
								NULL
							);
    printf("说明:在每个属性值的基础上加了属性标签!\n");
    printf("例如：A1、A2、A3代表第一个属性(--厚度--)的属性值是1、2、3\n");
    printf("例如：B1、B2、B3代表第二个属性(--大小--)的属性值是1、2、3\n");
    printf( "result: %d\n", result );
#endif

    return 0;
}
