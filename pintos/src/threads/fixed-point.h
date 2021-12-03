#include <stdint.h>
#define F (1 << 14)

/* 
    x and y are fixed-point number 
    n is an integer
    fixed-point numbers are in singed 17.14(p.q) format where p+q = 31
    f is 1 << q (1 << 14) 
*/
int int_to_fp(int n);           // convert n to fixed point, n * f
int fp_to_int_zero(int x);      // convert x to integer (rounding toward zero), x / f 
int fp_to_int_near(int x);      // convert x to integer (rounding to nearest)       
                                // (x+f/2) / f if x>=0, 
                                // (x-f/2) / f if x<=0
int add_x_y(int x, int y);      // add x and y, x + y
int sub_x_y(int x, int y);      // subtract y from x, x - y
int add_x_n(int x, int n);      // add x and n, x+n * f
int sub_x_n(int x, int n);      // subtract n from x, x-n * f
int mul_x_y(int x, int y);      // multiply x by y, ((int64_t) x) * y / f
int mul_x_n(int x, int n);      // multiply x by n, x * n
int div_x_y(int x, int y);      // divide x by y, ((int64_t) x) * f / y
int div_x_n(int x, int n);      // divide x by n, x / n

// 실수 rounding : https://readyfortest.tistory.com/34
// rounding to zero : 반올림 할 때 0과 가장 가까운 방향으로 반올림 
// round to even : 짝수와 가까운 방향으로 반올림. ex) 1.5 -> 2, 2.5 -> 2 

int int_to_fp(int n){
    return n*F;
}

int fp_to_int_zero(int x){
    return x/F;
}

int fp_to_int_near(int x){
    if(x>0){
        return (x + F/2) / F;
    } else{
        return (x - F/2) /F;
    }
}

int add_x_y(int x, int y){
    return x+y;
}

int sub_x_y(int x, int y){
    return x-y;
}

int add_x_n(int x, int n){
    return x+n*F;
}

int sub_x_n(int x, int n){
    return x-n*F;
}

int mul_x_y(int x, int y){
    return ((int64_t)x) * y / F;
}

int mul_x_n(int x, int n){
    return x * n;
}

int div_x_y(int x, int y){
    return ((int64_t)x) * F / y;
}

int div_x_n(int x, int n){
    return x / n;
}
