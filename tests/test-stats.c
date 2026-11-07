#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "stats.h"

const double epsilon = 0.000001;
double p_val;
double reference;

int main(int argc, char* argv[])
{
    double array1[16] = {0.1, 0.1, 0.1, 0.1,  0.1, 0.1, 0.1, 0.1,
                         0.1, 0.1, 0.1, 0.1,  0.1, 0.1, 0.1, 0.1};
    double array2[16] = {1, 1, 1, 1,  1, 1, 1, 1,
                         1, 1, 1, 1,  1, 1, 1, 1};
    double array3[16] = {0.1, 0.1, 0.1, 0.1,  0.1, 0.1, 0.1, 0.1,
                         0.1, 0.1, 0.1, 0.1,  0.1, 0.1, 0.1, 0.1};
    double array4[16] = {1, 1, 1, 1,  1, 1, 1, 1,
                         1, 1, 1, 1,  1, 1, 1, 1};

    p_val = Pvalue(array1, 16, array1, 16);
    //printf("p = %lf\n", p_val);
    reference = 1.0;
    assert(fabs(p_val - reference) < epsilon);

    p_val = Pvalue(array1, 16, array2, 16);
    //printf("p = %lf\n", p_val);
    reference = 0.0;
    assert(fabs(p_val - reference) < epsilon);

    p_val = Pvalue(array1, 16, array3, 15);
    //printf("p = %lf\n", p_val);
    reference = 1;
    assert(fabs(p_val - reference) < epsilon);

    p_val = Pvalue(array3, 15, array1, 16);
    //printf("p = %lf\n", p_val);
    reference = 1;
    assert(fabs(p_val - reference) < epsilon);

    array1[0] = 16;
    p_val = Pvalue(array1, 16, array2, 16);
    //printf("p = %lf\n", p_val);
    reference = 0.926088;
    assert(fabs(p_val - reference) < epsilon);

    array1[0] = 5; array1[1] = 5; array1[2] = 6;
    p_val = Pvalue(array1, 16, array2, 16);
    //printf("p = %lf\n", p_val);
    reference = 0.880211;
    assert(fabs(p_val - reference) < epsilon);

    array4[0] = 0;
    p_val = Pvalue(array1, 16, array4, 16);
    //printf("p = %lf\n", p_val);
    reference = 0.791237;
    assert(fabs(p_val - reference) < epsilon);

    ////printf("p = %lf\n", p_val);

    double a[4] = {3, 4, 1, 2.1};
    double b[3] = {490.2, 340, 433.9};
    p_val = Pvalue(a, 4, b, 3);
    reference = 0.010752;
    //printf("p = %lf\n", p_val);
    //assert(fabs(p_val - reference) < epsilon);

    return EXIT_SUCCESS;
}
