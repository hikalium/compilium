#include <stdio.h>

int main(int argc, char *argv[])
{
  FILE *fp;
  int c;
  if(argc < 2) return 1;
  fp = fopen(argv[1], "rb");
  if(!fp) return 1;
  while((c = getc(fp)) != EOF){
    putchar(c);
  }
  return 0;
}
