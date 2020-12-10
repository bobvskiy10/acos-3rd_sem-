#include <stdlib.h>
#include <stdio.h>
#include <math.h>

float f(float a) {
    return (float)sin((double)a);
}

float definite_integral (float a, float b, float dx) {
    float func_val, result = 0;
    while(a <= b) {
        func_val = f(a);
        result += func_val;
        a += dx;
    }
    result *= dx;
    return result;
}

int main(int argc, char* argv[]) {
    if (argc == 4) {
        float a = atof(argv[1]);
        float b = atof(argv[2]);
        float dx = atof(argv[3]);
        printf("integral from %f to %f with step %f is %f\n",a, b, dx, definite_integral(a,b, dx));
        return 0;
    }
    printf("not enough arguments for calling of file\n");
    return -1;
}