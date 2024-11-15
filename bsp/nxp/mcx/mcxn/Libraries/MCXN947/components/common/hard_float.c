#include "utilities.h"
#include "littleshell.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

float float_test_code(int i_loop,int j_loop)
{
    float tmp = 5000.0f;
    int i  = 0,j =0;
    
    for(i = 0;i <= i_loop;i++)
    {
        tmp += 0.1f;
        tmp *= 1.2f;
        
        for(j = 0 ;j <= j_loop;j++)
        {
            tmp -= 0.2f;
            tmp /= 1.1f;
        }
    }
    
    return tmp;
}


unsigned int hardfloat(char argc,char ** argv)
{
    int i  = myatoi(argv[1]);
    int j  = myatoi(argv[2]);

    printf("start\r\n");
    printf("float = %f.\r\n",float_test_code(i,j));
    printf("end\r\n");
    
    return 1;

}
LTSH_FUNCTION_EXPORT(hardfloat,"soft float test");


extern float softfloat_test_code(int i_loop,int j_loop);

unsigned int softfloat(char argc,char ** argv)
{
    int i  = myatoi(argv[1]);
    int j  = myatoi(argv[2]);
    
    printf("start\r\n");
    printf("float = %f.\r\n",softfloat_test_code(i,j));
    printf("end\r\n");
    return 1;

}
LTSH_FUNCTION_EXPORT(softfloat,"soft float test");