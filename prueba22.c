#include <stdio.h>
int main(){
system("stress-ng --vm 5 --vm-bytes 2G --hdd 3 &> /dev/null");
return 0;
}
