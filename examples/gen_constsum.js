const N = 10000;

console.log(`
#include <stdio.h>
int main() {
  for(int i = 0; i < 10000; i++){
    printf("%3d: 1/1*2/2*3/3* ... ${N}/${N} = %d\\n",
    i, ${Array(N).fill().map((e,i) => `${i+1}/${i+1}`).join("*\n")});
  }
  return 0;
}`);
