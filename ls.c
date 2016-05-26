//
//  ls.c
//  test
//
//  Created by Meteor on 16/5/25.
//  Copyright © 2016年 meteor. All rights reserved.
//
#include "ls.h"
#include "func.h"
extern unsigned short int ROOT_ENTRIES;
void ls_cluster(FILE* fpVHD, int clusterNum)
{
    unsigned char s[0x100];
    unsigned char *p;
    unsigned char cCreateAccurateTime; // 10ms
    unsigned short int cCreateTime;
    unsigned short int cCreateDate;
    unsigned short int cVisitDate;
    unsigned short int cTime;
    unsigned short int cDate;
    char cAttribute;
    char attribute[30];
    unsigned int firstClust;// 簇
    unsigned short int fileLen;
    int i = -1;
    unsigned char fileName[100];
    unsigned char tempName[100];
    char longFileNameFlag = 0;
    unsigned char extension[3];
    clusterNum = (clusterNum)?(clusterNum+0x116):(0xf8);
    fseek(fpVHD, clusterNum<<9, SEEK_SET);
    while (1) {
        if (clusterNum==0xf8) {
            if (i==ROOT_ENTRIES)  break;
        }
        else if (i==0x10)
            break;
        i++;
        fread(s, sizeof(char), 0x20, fpVHD);
        if (s[0]==0xe5)
            continue;
        if (s[0]==0)
            break;
        p=&s[0];
        cAttribute = s[0xb];
        if (cAttribute==0xf){// 长文件名
            longFileNameFlag = s[0];
            if ((longFileNameFlag&0xf0)==0x40) {
                fileName[0]='\0';
                getLongFileName(s);
                strcpy((char *)fileName, (const char *)s);
            }
            else {
                getLongFileName(s);
                strcpy((char *)tempName, (const char *)s);
                strcat((char *)tempName, (const char *)fileName);
                strcpy((char *)fileName, (const char *)tempName);
            }
            continue;
        }
        attribute[0]='\0';
        attribute[1]='\0';
        if ((cAttribute&1)==0) strcpy(attribute, "读写 ");
        else strcpy(attribute, "只读 ");
        if ((cAttribute&2)==0) strcat(attribute, "可显 ");
        else strcat(attribute, "隐藏 ");
        if ((cAttribute&4)) strcat(attribute, "系统 ");
        else if ((cAttribute&8)) strcat(attribute, "卷名 ");
        else if ((cAttribute&0x10)) strcat(attribute, "目录 ");
        else strcat(attribute, "文件 ");
        
        substr(extension, s, 8, 0xa);
        if (longFileNameFlag==0) {
            substr(fileName, s, 0, 7);
            completeFileName(fileName, extension);
        }
        longFileNameFlag = 0;
        cCreateAccurateTime = *(p+0xd);
        cCreateTime = *(p+0xe)+(*(p+0xf)<<8);
        cCreateDate = *(p+0x10)+(*(p+0x11)<<8);
        cVisitDate = *(p+0x12)+(*(p+0x13)<<8);
        cTime = *(p+0x16)+(*(p+0x17)<<8);
        cDate = *(p+0x18)+(*(p+0x19)<<8);
        firstClust = *(p+0x1a)+(*(p+0x1b)<<8);
        //firstClust = firstClust? (firstClust + 0x116) : 0xf8;
        fileLen = *(p+0x1c)+(*(p+0x1d)<<8)+(*(p+0x1e)<<16)+(*(p+0x1f)<<24);
        //if ( strcmp((const char *)fileName,".")!=0 && strcmp((const char *)fileName,"..")!=0 ) {
        //if ( fileName[0]!=0xe5 ) {
        printf("%-15s %04d/%02d/%02d %02d:%02d:%02d  %04d/%02d/%02d %02d:%02d:%02d  %04d/%02d/%02d  %5x  %4d b %s\n",fileName,
               ((cCreateDate>>0x9)&127)+1980, (cCreateDate>>5)&15, cCreateDate&31,
               (cCreateTime>>0xb)&31, (cCreateTime>>5)&63, (cCreateTime&0b11111)*2+(int)(cCreateAccurateTime*0.01),
               ((cDate>>0x9)&127)+1980, (cDate>>5)&15, cDate&31,
               (cTime>>0xb)&31, (cTime>>5)&63, (cTime&0b11111)*2,
               ((cVisitDate>>0x9)&127)+1980, (cVisitDate>>5)&15, cDate&31,
               firstClust, fileLen, attribute);
        //}
    }
}

void ls(FILE* fpVHD, int clusterNum)
{
    printf("filename        创建时间              修改时间               最近访问时间  起始簇    大小 属性\n");// 2 118 23000   212 328 65000   213 329 65200
    if (clusterNum==0) {
        ls_cluster(fpVHD, clusterNum);
    }
    else {
        while (1) {
            ls_cluster(fpVHD, clusterNum);
            fseek(fpVHD, 0x11000+clusterNum*2, SEEK_SET);
            fread(&clusterNum, sizeof(char), 2, fpVHD);
            if ((clusterNum&0xffff)==0xffff || clusterNum==0 ) {
                break;
            }
        }
    }
}