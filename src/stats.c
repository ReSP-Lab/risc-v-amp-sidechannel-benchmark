/*
* This file provides statistical functions to obtain Welch t-test p values
*
* Copyright (C) 2024-2025 Cédrick Austa <cedrick.austa@ulb.be>
* Copyright (C) 2020 Shuwen Deng <shuwen.deng@yale.edu>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software Foundation,
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*
*/

#include "stats.h"

double Pvalue (const double *restrict array1, const size_t n1, const double *restrict array2, const size_t n2) {
    if (n1 <= 1) {
        return 1.0;
    } else if (n2 <= 1) {
        return 1.0;
    }
    double m1 = 0.0, m2 = 0.0;

    // get sum of values in array1
    for (size_t x = 0; x < n1; x++) {
        // check to make sure this is a real number
        if (isfinite(array1[x]) == 0) {
            puts("Got a non-finite number in 1st array, can't calculate P-value.");
            exit(EXIT_FAILURE);
        }
        m1 += array1[x];
    }
    m1 /= n1;

    //get sum of values in array2
    for (size_t x = 0; x < n2; x++) {
        //check to make sure this is a real number
        if (isfinite(array2[x]) == 0) {
            puts("Got a non-finite number in 2nd array, can't calculate P-value.");
            exit(EXIT_FAILURE);
        }
        m2 += array2[x];
    }
    m2 /= n2;

    // printf("mean1 = %lf mean2 = %lf\n", m1, m2);
    // if the means are equal, the p-value is 1, leave the function
    if (m1 == m2) {
        printf("[DEBUG] the means are equal\n");
        return 1.0;
    }

    // variance estimate
    double v1 = 0.0, v2 = 0.0;

    //1st part of added v
    for (size_t x = 0; x < n1; x++)
    {
        v1 += (array1[x]-m1)*(array1[x]-m1);
    }
    for (size_t x = 0; x < n2; x++)
    {
        v2 += (array2[x]-m2)*(array2[x]-m2);
    }
    // printf("v1 = %lf\\tv2 = %lf\n",v1,v2);//DEBUGGING
    v1 = v1/(n1-1);
    v2 = v2/(n2-1);

    if (v1 ==  0.0 && v2 == 0.0)
        return m1 == m2 ? 1.0 : 0.0;

    const double t = (m1-m2)/sqrt(v1/n1+v2/n2);
    const double dof = pow((v1/n1+v2/n2),2.0)//numerator
            / ((v1*v1)/(n1*n1*(n1-1))+(v2*v2)/(n2*n2*(n2-1)));
//  printf("Welch = %lf DOF = %lf\n", t, dof);
//
    const double a = dof/2;
    double value = dof/(t*t+dof);
    if ((isinf(value) != 0) || (isnan(value) != 0)) {
        printf("free degree\n");
        return 1.0;
    }
    const double beta = lgammal(a)+0.57236494292470009-lgammal(a+0.5);
    const double acu = 0.1E-14;
    double ai;
    double cx;
    int indx;
    int ns;
    double pp;
    double psq;
    double qq;
    double rx;
    double temp;
    double term;
    double xx;

    //  ifault = 0;
    // Check the input arguments.
    if ( (a <= 0.0)) {// || (0.5 <= 0.0 )){
    //    *ifault = 1;
    //    return value;
    }
    if ( value < 0.0 || 1.0 < value )
    {
    //    *ifault = 2;
        return value;
    }

    /*
        Special cases.
    */
    if ( value == 0.0 || value == 1.0 ) {
        return value;
    }
    psq = a + 0.5;
    cx = 1.0 - value;

    if ( a < psq * value )
    {
        xx = cx;
        cx = value;
        pp = 0.5;
        qq = a;
        indx = 1;
    }
    else
    {
        xx = value;
        pp = a;
        qq = 0.5;
        indx = 0;
    }

    term = 1.0;
    ai = 1.0;
    value = 1.0;
    ns = ( int ) ( qq + cx * psq );

    /*
        Use the Soper reduction formula.
    */
    rx = xx / cx;
    temp = qq - ai;
    if ( ns == 0 )
    {
        rx = xx;
    }

    for ( ; ; )
    {
        term = term * temp * rx / ( pp + ai );
        value = value + term;
        temp = fabs ( term );

        if ( temp <= acu && temp <= acu * value )
        {
            value = value * exp ( pp * log ( xx )
            + ( qq - 1.0 ) * log ( cx ) - beta ) / pp;

            if ( indx )
            {
                value = 1.0 - value;
            }
            break;
        }
        ai = ai + 1.0;
        ns = ns - 1;

        if ( 0 <= ns )
        {
            temp = qq - ai;
            if ( ns == 0 )
            {
                rx = xx;
            }
        }
        else
        {
            temp = psq;
            psq = psq + 1.0;
        }
    }
    return value;
}
