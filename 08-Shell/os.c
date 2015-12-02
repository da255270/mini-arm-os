#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "reg.h"
#include "threads.h"

/* USART TXE Flag
 * This flag is cleared when data is written to USARTx_DR and
 * set when that data is transferred to the TDR
 */
#define USART_FLAG_TXE	((uint16_t) 0x0080)
#define USART_FLAG_RXNE ((uint16_t) 0x0020)
#define MAX_INPUT  200
#define NUL         0
#define FIBONACCI   1
#define HELP        2
const char* commandType[]={"NUL","fibonacci","help"};
extern int fib(int x);
void usart_init(void)
{
	*(RCC_APB2ENR) |= (uint32_t) (0x00000001 | 0x00000004);
	*(RCC_APB1ENR) |= (uint32_t) (0x00020000);

	/* USART2 Configuration, Rx->PA3, Tx->PA2 */
	*(GPIOA_CRL) = 0x00004B00;
	*(GPIOA_CRH) = 0x44444444;
	*(GPIOA_ODR) = 0x00000000;
	*(GPIOA_BSRR) = 0x00000000;
	*(GPIOA_BRR) = 0x00000000;

	*(USART2_CR1) = 0x0000000C;
	*(USART2_CR2) = 0x00000000;
	*(USART2_CR3) = 0x00000000;
	*(USART2_CR1) |= 0x2000;
}

void print_str(const char *str)
{
	while (*str) {
		while (!(*(USART2_SR) & USART_FLAG_TXE));
		*(USART2_DR) = (*str & 0xFF);
		str++;
	}
}

void reverse(char *s)
{
   for(int i=0,j=strlen(s)-1;i<j;i++,j--){
      int c = s[i];
      s[i] = s[j];
      s[j] = c;
   }
}

void itoa(int n,char *str)
{
  int i,j,sign;
  if((sign = n) < 0)
     n = -n;
  i = 0;
  do{
     str[i++] = n%10+'0';
  }while((n/=10)>0);
  if(sign<0)
     str[i++] = '-';
  str[i] = '\0';
  reverse(str);
}

int strequal(const char *str1,char *str2)
{
  while(1){
    if(*str1 == '\0' || *str2 == '\0'){
      if(*str1 == *str2)return 1;
      return 0;
    }
    else if(*str1++ != *str2++) break; 
  }
  return 0;
}

void bufferCleaner(char* buffer){
  int i=0;
  while(buffer[i]!='\0'){
    buffer[i]='\0';
    i++;
  }
}

int commandDetector(char* input){
  char* command = strtok(input,":"); 
  int i=0; 
  while(commandType[i]){
    if(strequal(command,commandType[i]))
      return i;   
    i++;
  }
  return 0;
}

void fibonacci(char* operand)
{
  char* temp = malloc(20);
  int result = fib(atoi(operand));
  itoa(result,temp);
  print_str(temp);
  print_str("\n Shell > ");
  free(temp);    
  thread_self_terminal();
}

void shell(void* user)
{
  char buffer[MAX_INPUT];
  int i;
  int command;
  char* operand;
  while(1){
    print_str("\n Shell > ");
    i=0;
    buffer[i]='\0';       
    while(1){
      if((*USART2_SR)&(USART_FLAG_RXNE)){
          buffer[i]= (*USART2_DR) & 0xff;        
      }
      if(buffer[i]==13 || buffer[i]=='\n'){
         command = commandDetector(buffer);
         switch (command){
           case NUL:
             break;
           case FIBONACCI:             
             operand = &buffer[10];
             if (thread_create(fibonacci, (void*)operand ) == -1)
	       print_str("fibonacci creation failed\r\n");        
             break;
           case HELP:
             break;
         }
         break;
      }else if(buffer[i]=='\0'){
         
      }else{
         print_str(&buffer[i++]);
      }
    }
    bufferCleaner(buffer);
  }
}

/* 72MHz */
#define CPU_CLOCK_HZ 72000000

/* 100 ms per tick. */
#define TICK_RATE_HZ 10

int main(void)
{
	const char *str1 = "shell";

	usart_init();

	if (thread_create(shell, (void *) str1) == -1)
		print_str("shell creation failed\r\n");
     //   if (thread_create(fibonacci, (void *) str2) == -1)
	//	print_str("fibonacci creation failed\r\n");
	/* SysTick configuration */
	*SYSTICK_LOAD = (CPU_CLOCK_HZ / TICK_RATE_HZ) - 1UL;
	*SYSTICK_VAL = 0;
	*SYSTICK_CTRL = 0x07;

	thread_start();

	return 0;
}


