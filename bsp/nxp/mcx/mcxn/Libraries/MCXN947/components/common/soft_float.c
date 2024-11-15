#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

float softfloat_test_code(int i_loop,int j_loop)
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