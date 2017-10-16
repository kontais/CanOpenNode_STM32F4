#include <stdio.h>


void uart4_init(void);
void systick_init(void);
int CO_main(void);


int main(void)
{
    uart4_init();
    printf("UART4 OK\n");
    
    systick_init();
    
    while (1) {
        CO_main();
    }

//    return 0;
}
