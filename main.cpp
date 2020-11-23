#include <iostream>
#include <stdio.h>
using namespace std;

char buf1[100];
char buf2[100];
char bufHash[100];
string userName;

int main() {

    //string strCMD = "dmidecode -s system-uuid";
    string strCMD = "blkid | grep /dev/sda1";
    
    //convert strCMD to const char * cmd
    const char * cmd = strCMD.c_str();

    //execute the command cmd and store the output in a file named output
    FILE * output = popen(cmd, "r");

    fgets (buf1, 100, output);

    //fprintf (stdout, "%s", buf1);

    strCMD = "rpm -qi setup | grep Install";

    cmd = strCMD.c_str();

    output = popen(cmd, "r");

    fgets (buf2, 100, output);

    //fprintf (stdout, "%s", buf2);

    int j,k = 0;

    int p1 = 15;
    j = p1 + j;
    for(int i = 20; i<=35; i++){
        while(buf2[j] == ' ' | buf2[j] == ':')
            j++;
        bufHash[k++] = buf1[i];
        bufHash[k++] = buf2[j];
        j++;
    }
    bufHash[k]=0;



//    while (fgets (buf, 1000, output)) {
//        fprintf (stdout, "%s", buf);
 //   }
//    pclose(output);



    fprintf (stdout, "%s", bufHash);
    
    return 0;
}
