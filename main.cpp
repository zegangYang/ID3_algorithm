#include <stdio.h>
#include <stdlib.h>

#include "id3.h"

#include <map>
#include <iostream>

using namespace std;

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
char *dataset_lenses[] = 
{
	"young","myope","no","reduced","no lenses",
	"young","myope","no","normal","soft",
	"young","myope","yes","reduced","no lenses",
	"young","myope","yes","normal","hard",
	"young","hyper","no","reduced","no lenses",
	"young","hyper","no","normal","soft",
	"young","hyper","yes","reduced","no lenses",
	"young","hyper","yes","normal","hard",
	"pre",	"myope","no","reduced","no lenses",
	"pre",	"myope","no","normal","soft",
	"pre",	"myope","yes","reduced","no lenses",
	"pre",	"myope","yes","normal","hard",
	"pre",	"hyper","no","reduced","no lenses",
	"pre",	"hyper","no","normal","soft",
	"pre",	"hyper","yes","reduced","no lenses",
	"pre",	"hyper","yes","normal","no lenses",
	"presbyopic","myope","no","reduced","no lenses",
	"presbyopic","myope","no","normal","no lenses",
	"presbyopic","myope","yes","reduced","no lenses",
	"presbyopic","myope","yes","normal","hard",
	"presbyopic","hyper","no","reduced","no lenses",
	"presbyopic","hyper","no","normal","soft",
	"presbyopic","hyper","yes","reduced","no lenses",
	"presbyopic","hyper","yes","normal","no lenses",
	NULL
};

int main()
{
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
	/*result = id3tree_create( 	dataset_lenses,		
							5,					//  cols
							24,					// rows
							"age",			
							"prescription",		
							"astigmatic",			
							"tearRate",
							"Result",
							NULL				
						);*/
	/*result = id3tree_create( 	dataset_cancer,			
			10,	//列数
			7,	//行数	
			"Clump Thickness",
			"Uniformity of Cell Size",
			"Uniformity of Cell Shape",	
			"Marginal Adhesion",		
			"Single Epithelial Cell Size",
			"Bare Nuclei ",
			"Bland Chromatin",
			"Normal Nucleoli",
			"Mitoses",
			"Result",
			NULL
		);	*/
				
    printf( "result: %d\n", result );


    return 0;
}
