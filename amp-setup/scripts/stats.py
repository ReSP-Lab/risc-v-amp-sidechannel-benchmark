import math

def p_value(array1, n1, array2, n2):
    if n1 <= 1:
        return 1.0
    elif n2 <= 1:
        return 1.0

    m1 = 0; m2 = 0
    for x in range(0, n1):
        if not math.isfinite(array1[x]):
            print("Got a non-finite number in 1st array, "+
                  "can't calculate P-value.")
            return 1
            #return -1
        m1 += array1[x]
    m1 /= n1
    for x in range(0, n2):
        if not math.isfinite(array2[x]):
            print("Got a non-finite number in 1st array, "+
                  "can't calculate P-value.")
            return 1
            #return -1
        m2 += array2[x]
    m2 /= n2

    #print(f"mean1= {m1}\t mean2= {m2}")

    # if the means are equal, the p-value is 1, leave the function
    if (m1 == m2):
        print("the means are equal")
        return 1.0

    # variance estimate
    v1 = 0.0; v2 = 0.0

    # variance calculation
    for x in range(0, n1):
        v1 += (array1[x]-m1)*(array1[x]-m1)
    v1 = v1/(n1-1)
    for x in range(0, n2):
        v2 += (array2[x]-m2)*(array2[x]-m2)
    v2 = v2/(n2-1)

    #print(f"v1= {v1}\t v2= {v2}")

    if v1 == 0.0 and v2 == 0.0:
        if m1 == m2:
            return 1.0
        else:
            return 0.0

    t_numerator = m1-m2
    t_denominator = math.sqrt(v1/n1+v2/n2)
    t = 0
    try:
        t = t_numerator/t_denominator
    except ZeroDivisionError:
        print("Got a ZeroDivisionError: can't calculate P-value, "+
              "case ignored.")
        return 1.0
    #except Exception as err:
    #    print(f"Unexpected {err=}, {type(err)=}")
    #    return 1.0

    dof = pow(v1/n1+v2/n2,2) / ((v1*v1)/(n1*n1*(n1-1))+(v2*v2)/(n2*n2*(n2-1)))
    #print(f"Welch = {t} DOF = {dof}\n")

    # now calculating p value
    a = dof/2
    value = dof/(t*t+dof)
    #print(f"p = {value}\n")
    if (math.isinf(value) != 0) or (math.isnan(value) != 0):
        printf("free degree\n")
        return 1.0
    beta = math.lgamma(a)+0.57236494292470009-math.lgamma(a+0.5)
    acu = 0.1E-14
    ai = 0.0; cx = 0.0
    indx = 0; ns = 0
    pp = 0.0; psq = 0.0; qq = 0.0; rx = 0.0; temp = 0.0; term = 0.0; xx = 0.0

    #   ifault = 0;
    #  Check the input arguments.
    if a <= 0.0:
    #   ifault = 1;
        pass
    if value < 0.0 or 1.0 < value:
    #   ifault = 2;
        return value

    #   Special cases.   #
    if value == 0.0 or value == 1.0:
        return value

    psq = a + 0.5
    cx = 1.0 - value

    if a < psq * value:
        xx = cx
        cx = value
        pp = 0.5
        qq = a
        indx = 1
    else:
        xx = value
        pp = a
        qq = 0.5
        indx = 0

    term = 1.0
    ai = 1.0
    value = 1.0
    ns = int( qq + cx * psq )

    #  Use the Soper reduction formula.
    rx = xx / cx
    temp = qq - ai
    if ns == 0:
        rx = xx

    while True:
        term = term * temp * rx / ( pp + ai )
        value = value + term
        temp = abs( term )

        if temp <= acu and temp <= acu * value:
            value = value * math.exp(pp * math.log(xx)
                                     + (qq - 1.0) * math.log(cx) - beta) / pp

            if indx:
                value = 1.0 - value
            break;

        ai = ai + 1.0
        ns = ns - 1
        if 0 <= ns:
            temp = qq - ai
            if ns == 0:
                rx = xx
        else:
            temp = psq
            psq = psq + 1.0

    #print(f"p = {value}\n")
    return value;
