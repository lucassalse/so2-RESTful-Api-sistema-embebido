#include <stdio.h>
#include <stdlib.h>

int main()
{
   int num;
   FILE *fptr;

   // use appropriate location if you are using MacOS or Linux
   fptr = fopen("/home/lucas/hola_mundo.txt","w");

   if(fptr == NULL)
   {
      printf("Error!");   
      exit(1);             
   }
   fprintf(fptr,"%s \n","Hola mundo");
   fclose(fptr);

   return 0;
}
