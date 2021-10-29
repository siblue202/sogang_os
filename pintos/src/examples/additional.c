/*
    JGH 
    additional system calls
    
    for sogang_os 
*/

#include <stdio.h>
#include <syscall.h>

int main(int argc, char *argv[]){
    int result_fib, result_max;
/*
    for(int i=0; i<argc; i++){
        printf("arg[%d] = %d", i, atoi(argv[i]));
    }

*/
    result_fib = fibonacci(atoi(argv[1]));
    result_max = max_of_four_int(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));

    printf("%d %d\n", result_fib, result_max);

    return EXIT_SUCCESS;
}