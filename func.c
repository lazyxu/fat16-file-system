//
//  func.c
//  test
//
//  Created by Meteor on 16/5/25.
//  Copyright © 2016年 meteor. All rights reserved.
//

#include "func.h"
void substr(unsigned char *des, unsigned char *str, int start, int end)
{
    int i;
    for (i = start; i <= end; i++) {
        des[i-start] = str[i];
    }
    des[i]='\0';
}

void getLongFileName(unsigned char *str)
{
    int i, j;
    for (i = 1, j = 0; i < 0x20; i += 2, j++) {
        str[j] = str[i];
        if(i==0x9) i+=3;
        if(i==0x18) i+=2;
    }
    str[j]='\0';
}

void completeFileName(unsigned char *fileName, unsigned char *extension)
{
    int i,j;
    for (i=0; fileName[i]!=' '&&fileName[i]!='\0'; i++);
    j = i;
    if (extension[0]!=' ') {
        fileName[j] = '.';
        j++;
    }
    for (i=0; extension[i]!=' '&&extension[i]!='\0'; i++, j++) {
        fileName[j] = extension[i];
    }
    fileName[j] = 0;
}