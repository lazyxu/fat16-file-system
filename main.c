#include <string.h>
#include "time.h"


#include "info.h"
#include "ls.h"


int tempListFileN, tempListFileFlag[100], tempListFileClust[100];
unsigned char tempListFileName[100][50];

int ROOT_ENTRIES;

struct mFILE {
    char *fileName;
    int mode;
    int clusterN;
    unsigned short int *clusterList;
    unsigned short int currentCluster;
    unsigned short int currentOffset;
};
void substr(unsigned char *des, unsigned char *str, int start, int end);
void getLongFileName(unsigned char *str);
void completeFileName(unsigned char *fileName, unsigned char *extension);

void list_cluster(FILE *fpVHD, int clusterNum)
{
    unsigned char fileName[100];
    unsigned char tempName[100];
    unsigned char s[0x100];
    unsigned char *p;
    char longFileNameFlag = 0;
    unsigned char extension[3];
    char cAttribute;
    char attribute[30];
    int firstClust;
    int i = -1;
    clusterNum = (clusterNum)?(clusterNum+0x116):(0xf8);
    fseek(fpVHD, clusterNum<<9, SEEK_SET);
    while (1) {
        if (clusterNum==0xf8) {
            if (i==ROOT_ENTRIES)  break;
        }
        else if (i==0x10)
            break;
        i++;
        tempListFileN++;
        fread(s, sizeof(char), 0x20, fpVHD);
        tempListFileName[tempListFileN][0] = s[0];
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
        tempListFileFlag[tempListFileN] = 0;
        if ((cAttribute&0x10))  tempListFileFlag[tempListFileN]=2;//目录
        else if(!(cAttribute&4) && !(cAttribute&8)) tempListFileFlag[tempListFileN]=1;//文件
        
        substr(extension, s, 8, 0xa);
        if (longFileNameFlag==0) {
            substr(fileName, s, 0, 7);
            completeFileName(fileName, extension);
        }
        longFileNameFlag = 0;
        tempListFileName[tempListFileN][0]=0;
        strcpy((char *)tempListFileName[tempListFileN], (const char *)fileName);
        firstClust = *(p+0x1a)+(*(p+0x1b)<<8);
        tempListFileClust[tempListFileN] = firstClust;
    }
}

void list(FILE *fpVHD, int clusterNum)
{
    tempListFileN = -1;
    if (clusterNum==0) {
        list_cluster(fpVHD, clusterNum);
    }
    else {
        while (1) {
            list_cluster(fpVHD, clusterNum);
            fseek(fpVHD, 0x11000+clusterNum*2, SEEK_SET);
            fread(&clusterNum, sizeof(char), 2, fpVHD);
            if ((clusterNum&0xffff)==0xffff || clusterNum==0 ) {
                break;
            }
        }
    }
}

void clusterRead(char * clusterMem, FILE* fpVHD, int clusterNum)
{
    clusterNum = (clusterNum)?(clusterNum+0x116):(0xf8);
    fseek(fpVHD, clusterNum<<9, SEEK_SET);
    fread(clusterMem, sizeof(char), 0x200, fpVHD);
}

int fileNameCmp(char *str1, unsigned char *str2){
    int i;
    for (i=0; str1[i]!=0&&str2[i]!=0; i++) {
        if (str1[i]>='A' && str1[i]<='Z') {
            str1[i] = str1[i]|0x60;
        }
        if (str2[i]>='A' && str2[i]<='Z') {
            str2[i] = str2[i]|0x60;
        }
        if (str1[i]!=str2[i]) {
            return 1;
        }
    }
    if (str1[i]!=str2[i]) {
        return 1;
    }
    return 0;
}

void vi(FILE* fpVHD, char *fileName, int currentClust)
{
    int i;
    char content[0x200];
    unsigned short int clusterNum;
    int flag = 0;
    list(fpVHD, currentClust);
    for (i=0; i<tempListFileN; i++) {
        if (fileNameCmp(fileName, tempListFileName[i])==0 && tempListFileFlag[i]==1){
            clusterNum = tempListFileClust[i];
            flag = 1;
            break;
        }
    }
    if (clusterNum!=0) {
        while (1) {
            clusterRead(content, fpVHD, clusterNum);
            printf("%s\n", content);
            fseek(fpVHD, 0x11000+clusterNum*2, SEEK_SET);
            fread(&clusterNum, sizeof(char), 2, fpVHD);
            if ((clusterNum&0xffff)==0xffff || clusterNum==0 ) {
                break;
            }
        }
        if (flag == 0) {
            printf("no such file!\n");
        }
    }
    else
        printf("\n");
}

int checkCD(char *pathName, char *fileName)
{
    int i, j, pathFlag = 0;
    if (pathName[0]=='/') {
        for (i=0;pathName[i+1]!='\0';i++) {
            pathName[i] = pathName[i+1];
        }
        pathName[i] = '\0';
        return -1;
    }
    for (i=0;pathName[i]!='/' && pathName[i]!='\0';i++) {
        fileName[i] = pathName[i];
    }
    fileName[i] = '\0';
    j = i;
    if (pathName[j]=='/' && pathName[j+1]!='\0') {
        pathFlag = 1;
        for (;pathName[j+1]!='\0';j++) {
            pathName[j-i] = pathName[j+1];
        }
    }
    pathName[j-i] = '\0';
    return pathFlag;
}

int cd(FILE* fpVHD, char *pathName, char *fileName, int currentClust)
{
    int i;
    int pathAnalyse;
    list(fpVHD, currentClust);
    pathAnalyse = checkCD(pathName, fileName);
    if (pathAnalyse==1) {
        for (i=0; i<tempListFileN; i++) {
            if (fileNameCmp(fileName, tempListFileName[i])==0 && tempListFileFlag[i]==2){
                return cd(fpVHD, pathName, fileName, tempListFileClust[i]);
            }
        }
    }
    else if (pathAnalyse==0){
        for (i=0; i<tempListFileN; i++) {
            if (fileNameCmp(fileName, tempListFileName[i])==0 && tempListFileFlag[i]==2){
                return tempListFileClust[i];
            }
        }
    }
    else if (pathAnalyse==-1){
        return cd(fpVHD, pathName, fileName, 0xf8);
    }
    return -1;
}

void mv(FILE* fpVHD, char *fileName, char *fileName1, unsigned short int currentClust)
{
    int i;
    for (i=0; i<tempListFileN; i++) {
        if (fileNameCmp(fileName, tempListFileName[i])==0 && tempListFileFlag[i]==2){
            currentClust = tempListFileClust[i];
            list(fpVHD, currentClust);
            return ;
        }
    }
    printf("no such directory!\n");
}

void add_file(FILE *fpVHD, char *directoryRecord, int n, unsigned short int currentClust)
{
    unsigned short int clusterNum;
    int *index = malloc(sizeof(char)*n);
    int i, j;
    for (i=0, j=0; i<0x20; i++) {
        if (tempListFileName[i][0]!=0xe5 || tempListFileName[i][0]==0x00) {
            index[j]=i;
            j++;
        }
    }
    n--;
    if (n<=0x20-j) {
        while (n>=0) {
            clusterNum = (currentClust)?(currentClust+0x116):(0xf8);
            printf("%x %x %x: %s", currentClust, clusterNum, (clusterNum<<9) + 0x20*index[n], directoryRecord);
            fseek(fpVHD, (clusterNum<<9) + 0x20*index[n], SEEK_SET);
            fwrite(directoryRecord, 0x20, 1, fpVHD);
            n--;
        }
    }
    free(index);
}

void get_short_file_name(char *directoryRecord, char *fileName, int x, int y, unsigned char attributeb, unsigned char attributec, unsigned short int clusterNum, unsigned int len)
{
    int j,k;
    unsigned short int cin;
    for (j=0,k=0; j<x; j++,k++) { // 0x0~0x7文件名
        directoryRecord[j] = fileName[k];
    }
    for (; j<8; j++) {
        directoryRecord[j] = ' ';
    }
    for (k++; j<8+y; j++,k++) { // 0x8~0xa文件拓展名
        directoryRecord[j] = fileName[k];
    }
    for (; j<8+3; j++) {
        directoryRecord[j] = ' ';
    }
//    0xb 属性 目录标志 属性字节	00000000(读写)
//    00000001(只读)
//    00000010(隐藏)
//    00000100(系统)
//    00001000(卷标)
//    00010000(子目录)
//    00100000(归档)
    // 0xc 如果目录项需要长文件名项或全部为大写，该位为0x00；如果该目录项全部为小写：directory设置为0x08、file设置为0x18
    directoryRecord[0xb] = attributeb;
    directoryRecord[0xc] = attributec;
    
    time_t rawtime;
    struct tm * timeinfo;
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    printf ( "The current date/time is: %d %s", timeinfo->tm_sec, asctime (timeinfo) );
    directoryRecord[0xd] = timeinfo->tm_sec?0x80:0; // 0xd 创建时间的10毫秒位 假装~
    
    cin = 0;
    cin = timeinfo->tm_hour<<11|timeinfo->tm_min<<5|timeinfo->tm_sec>>1;
    directoryRecord[0xe] = cin&0xff;// 0xe~0xf 文件创建时间
    directoryRecord[0xf] = (cin>>8)&0xff;
    directoryRecord[0x16] = cin&0xff;// 0x16~0x17 文件最近修改时间
    directoryRecord[0x17] = (cin>>8)&0xff;
    
    // 0x14~0x15 文件起始簇号的高16位
    cin = 0;
    cin = (timeinfo->tm_year-80)<<9|(timeinfo->tm_mon+1)<<5|timeinfo->tm_mday;
    directoryRecord[0x10] = cin&0xff;// 0x10~0x11 文件创建日期
    directoryRecord[0x11] = (cin>>8)&0xff;
    directoryRecord[0x12] = cin&0xff;// 0x12~0x13 文件最后访问日期
    directoryRecord[0x13] = (cin>>8)&0xff;
    directoryRecord[0x18] = cin&0xff;// 0x18~0x19 文件最近修改日期
    directoryRecord[0x19] = (cin>>8)&0xff;
    
    directoryRecord[0x1a] = clusterNum&0xff;// 0x1a~0x1b 文件起始簇号
    directoryRecord[0x1b] = (clusterNum>>8)&0xff;
    
    directoryRecord[0x1c] = len&0xff;// 0x1c~0x1f 文件长度
    directoryRecord[0x1d] = (len>>8)&0xff;
    directoryRecord[0x1e] = (len>>16)&0xff;
    directoryRecord[0x1f] = (len>>24)&0xff;
    for (j=0; j<0x20; j++) {
        printf("%02x %c\n", directoryRecord[j]&0xff, directoryRecord[j]);
    }
}

void add_sub_file(FILE *fpVHD, unsigned short int current_clust, unsigned upper_clust)
{
    unsigned short int clusterNum = (current_clust)?(current_clust+0x116):(0xf8);
    fseek(fpVHD, (clusterNum<<9), SEEK_SET);
    char directoryRecord[0x20];
    get_short_file_name(directoryRecord, ".", 1, 0, 0x10, 0x00, current_clust, 0);
    fwrite(directoryRecord, 0x20, 1, fpVHD);
    get_short_file_name(directoryRecord, "..", 2, 0, 0x10, 0x00, upper_clust, 0);
    fwrite(directoryRecord, 0x20, 1, fpVHD);
}

void set_short_file_name(FILE *fpVHD, char *fileName, int x, int y, unsigned short int clusterNum, unsigned short int currentClust)
{
    char directoryRecord[0x20];
    get_short_file_name(directoryRecord, fileName, x, y, 0x10, 0x08, clusterNum, 0);
    add_file(fpVHD, directoryRecord, 1, currentClust);
    add_sub_file(fpVHD, clusterNum, currentClust);
}

char *set_long_file_name(FILE *fpVHD, char *fileName, int x, int y, unsigned short int clusterNum, unsigned short int currentClust)
{
    char *directoryRecord;
    
    return directoryRecord;
}

void set_directory_entry(FILE *fpVHD, char *fileName, unsigned short int clusterNum, unsigned short int currentClust)
{
    int i;
    int n;
    int flag = 0;
    n = (int)strlen(fileName);
    for (i=n-1; i>=0; i--) {
        if (fileName[i]=='.') {
            //printf("%d.%d\n", i, n-1-i);
            flag = 1;
            if (i<=8 && n-1-i<=3)
                set_short_file_name(fpVHD, fileName, i, n-1-i, clusterNum, currentClust);
            else
                set_long_file_name(fpVHD, fileName, i, n-1-i, clusterNum, currentClust);
            break;
        }
    }
    if(i==-1){
        if (n<=8)
            set_short_file_name(fpVHD, fileName, n, 0, clusterNum, currentClust);
        else
            set_long_file_name(fpVHD, fileName, i, n-1-i, clusterNum, currentClust);
    }
    else
        set_long_file_name(fpVHD, fileName, i, n-1-i, clusterNum, currentClust);
    
}

void mdir(FILE* fpVHD, char *fileName, unsigned short int currentClust)
{
    int i;
    int offset;
    unsigned short int cin = 0xffff;
    unsigned short int clusterNum;
    list(fpVHD, currentClust);
    for (i=0; i<tempListFileN; i++) {
        if (fileNameCmp(fileName, tempListFileName[i])==0 && tempListFileFlag[i]==2){
            printf("directory name has been used!\n");
            return ;
        }
    }
    offset = 0x11000;
    while(1)
    {
        fseek(fpVHD, offset, SEEK_SET);
        fread(&clusterNum, sizeof(clusterNum), 1, fpVHD);
        if (clusterNum==0x0000) {
            fseek(fpVHD, offset, SEEK_SET);
            fwrite(&cin,1, 2, fpVHD);
            clusterNum = (offset-0x11000)/2;
            break;
        }
        offset += 2;
    }
    set_directory_entry(fpVHD, fileName, clusterNum, currentClust);
    list(fpVHD, currentClust);
}

void get_info(FILE* fpVHD)
{
    fseek(fpVHD, 0x10011, SEEK_SET);
    fread(&ROOT_ENTRIES, sizeof(char), 2, fpVHD);
    printf("0x10011 %02x: 根目录项数 %d\n", ROOT_ENTRIES, ROOT_ENTRIES);
    printf("请输入已完成命令 或输入以下序号\n\
           0.exit\n\
           1.general_info\n\
           2.0x0000_to_0xffff\n\
           3.fat16_0x10000_to_0x1ffff\n\
           4.fat32_0x10000_to_0x1ffff\n\
           5.FAT1\n");
}

void rm_cluster(FILE* fpVHD, unsigned short int currentClust)
{
    int i;
    unsigned short int cin;
    unsigned short int clusterNum;
    int offset;
    list(fpVHD, currentClust);
    for (i=0; i<tempListFileN; i++) {
        if (tempListFileFlag[i]==2){
            clusterNum = tempListFileClust[i];
            fseek(fpVHD, (((currentClust)?(currentClust+0x116):(0xf8))<<9)+i*0x20, SEEK_SET);
            fread(&cin, sizeof(char), 1, fpVHD);
            if (cin==0x00) {
                break;
            }
            else if (cin!=0xe5) {
                cin = 0xe5;
                fseek(fpVHD, (clusterNum<<9)+i*20, SEEK_SET);
                fwrite(&cin, sizeof(char), 1, fpVHD);
                cin = 0x0000;
                while(1)
                {
                    offset = 0x11000+clusterNum*2;
                    fseek(fpVHD, offset, SEEK_SET);
                    fread(&clusterNum, sizeof(char), 2, fpVHD);
                    fseek(fpVHD, offset, SEEK_SET);
                    fwrite(&cin,1, 2, fpVHD);
                    if (clusterNum==0xffff) {
                        break;
                    }
                }
                rm_cluster(fpVHD, clusterNum);
                list(fpVHD, currentClust);
                return;
            }
        }
        else {
            clusterNum = tempListFileClust[i];
            fseek(fpVHD, (((currentClust)?(currentClust+0x116):(0xf8))<<9)+i*0x20, SEEK_SET);
            fread(&cin, sizeof(char), 1, fpVHD);
            if (cin==0x00) {
                break;
            }
            else if (cin!=0xe5) {
                cin = 0xe5;
                fseek(fpVHD, (clusterNum<<9)+i*20, SEEK_SET);
                fwrite(&cin, sizeof(char), 1, fpVHD);
                cin = 0x0000;
                while(1)
                {
                    offset = 0x11000+clusterNum*2;
                    fseek(fpVHD, offset, SEEK_SET);
                    fread(&clusterNum, sizeof(char), 2, fpVHD);
                    fseek(fpVHD, offset, SEEK_SET);
                    fwrite(&cin,1, 2, fpVHD);
                    if (clusterNum==0xffff) {
                        break;
                    }
                }
                list(fpVHD, currentClust);
                return;
            }
        }
    }
}

void rm(FILE* fpVHD, char *fileName, unsigned short int currentClust)
{
    int i;
    unsigned short int cin;
    unsigned short int clusterNum;
    int offset;
    list(fpVHD, currentClust);
    for (i=0; i<tempListFileN; i++) {
        if (fileNameCmp(fileName, tempListFileName[i])==0) {
            if (tempListFileFlag[i]==2){
                clusterNum = tempListFileClust[i];
                fseek(fpVHD, (((currentClust)?(currentClust+0x116):(0xf8))<<9)+i*0x20, SEEK_SET);
                cin = 0xe5;
                fwrite(&cin, sizeof(char), 1, fpVHD);
                cin = 0x0000;
                while(1)
                {
                    offset = 0x11000+clusterNum*2;
                    fseek(fpVHD, offset, SEEK_SET);
                    fread(&clusterNum, sizeof(char), 2, fpVHD);
                    fseek(fpVHD, offset, SEEK_SET);
                    fwrite(&cin,1, 2, fpVHD);
                    if (clusterNum==0xffff) {
                        break;
                    }
                }
                rm_cluster(fpVHD, clusterNum);
                list(fpVHD, currentClust);
                return;
            }
            else if (tempListFileFlag[i]==1){
                clusterNum = tempListFileClust[i];
                fseek(fpVHD, (((currentClust)?(currentClust+0x116):(0xf8))<<9)+i*0x20, SEEK_SET);
                cin = 0xe5;
                fwrite(&cin, sizeof(char), 1, fpVHD);
                cin = 0x0000;
                while(1)
                {
                    offset = 0x11000+clusterNum*2;
                    fseek(fpVHD, offset, SEEK_SET);
                    fread(&clusterNum, sizeof(char), 2, fpVHD);
                    fseek(fpVHD, offset, SEEK_SET);
                    fwrite(&cin,1, 2, fpVHD);
                    if (clusterNum==0xffff) {
                        break;
                    }
                }
                list(fpVHD, currentClust);
                return;
            }
        }
    }
    printf("directory/file not found!\n");
}

int analyse_mode(char *mode)
{
    return 0;
}

int get_clusterN(FILE *fpVHD, unsigned short int clusterNum)
{
    int n=0;
    while (1) {
        n++;
        fseek(fpVHD, 0x11000+clusterNum*2, SEEK_SET);
        fread(&clusterNum, sizeof(clusterNum), 1, fpVHD);
        if (clusterNum==0xffff) {
            break;
        }
    }
    return n;
}

void get_clusterList(unsigned short int *list, FILE *fpVHD, unsigned short int clusterNum, int n)
{
    while (1) {
        n++;
        *(list++) = clusterNum;
        fseek(fpVHD, 0x11000+clusterNum*2, SEEK_SET);
        fread(&clusterNum, sizeof(clusterNum), 1, fpVHD);
        if (clusterNum==0xffff) {
            break;
        }
    }
}

void print_mfp(struct mFILE *mfp)
{
    int i;
    printf("fileName:%s \n\
           mode:%d \n\
           currentOffset:0x%x \n\
           currentCluster:0x%x \n\
           clusterN:0x%x \n\
           clusterList: ", mfp->fileName, mfp->mode, mfp->currentOffset, mfp->currentCluster, mfp->clusterN);
    for (i=0; i<mfp->clusterN; i++) {
        printf("%x ", mfp->clusterList[i]);
    }
    printf("\n");
}

struct mFILE *mfopen(FILE *fpVHD, char *fileName, char *mode,  unsigned short int currentClust)
{
    struct mFILE *mfp = malloc(sizeof(struct mFILE));
    int i;
    unsigned short int clusterNum;
    list(fpVHD, currentClust);
    for (i=0; i<tempListFileN; i++) {
        if (fileNameCmp(fileName, tempListFileName[i])==0 && tempListFileFlag[i]==1){
            clusterNum = tempListFileClust[i];
            mfp->fileName = malloc(sizeof(char)*20);
            strcpy(mfp->fileName, fileName);
            mfp->mode = analyse_mode(mode);
            mfp->currentOffset = 0;
            mfp->currentCluster = 0;
            mfp->clusterN = get_clusterN(fpVHD, clusterNum);
            mfp->clusterList = malloc(sizeof(unsigned short int)*mfp->clusterN);
            get_clusterList(mfp->clusterList, fpVHD, clusterNum, mfp->clusterN);
            break;
        }
    }
    return mfp;
}

unsigned long mfread(FILE *fpVHD, char *content, int size, int count, struct mFILE *mfp)
{
    unsigned short int clusterNum;
    if (mfp->currentOffset + size*count < 0x200) {
        clusterNum = mfp->clusterList[mfp->currentCluster];
        clusterNum = clusterNum?(clusterNum+0x116):(0xf8);
        fseek(fpVHD, (clusterNum<<9)+mfp->currentOffset, SEEK_SET);
        fread(content, size, count, fpVHD);
        mfp->currentOffset += size*count;
    }
    return size*count;
}

unsigned long  mfwrite(FILE *fpVHD, char *content, int size, int count, struct mFILE *mfp)
{
    unsigned short int clusterNum;
    if (mfp->currentOffset + size*count < 0x200) {
        clusterNum = mfp->clusterList[mfp->currentCluster];
        clusterNum = clusterNum?(clusterNum+0x116):(0xf8);
        fseek(fpVHD, (clusterNum<<9)+mfp->currentOffset, SEEK_SET);
        fwrite(content, size, count, fpVHD);
        mfp->currentOffset += size*count;
    }
    return size*count;
}

int mfclose(struct mFILE *mfp)
{
    free(mfp->fileName);
    free(mfp->clusterList);
    free(mfp);
    return 1;
}

int main(void)
{
    char command[20], command1[20];
    char *content = NULL;
    const char *filename="/Users/meteor/Downloads/fat.vhd";
    int tempClust;
    int i;
    unsigned short int currentClust;
    unsigned long len;
    FILE *fpVHD;
    struct mFILE *mfp = malloc(sizeof(struct mFILE));
    int size = 0, count = 0;
    fpVHD = fopen(filename, "rb+");
    printf("已完成命令 ls cd vi mkdir rm\n");
    printf("未完成命令 cp mv\n");
    currentClust = 0;
    get_info(fpVHD);
    while (1) {
        scanf("%s", command);
        print_info(fpVHD, command[0]);
        if (strcmp(command, "exit")==0)
            break;
        if (strcmp(command, "ls")==0) // useage: ls 功能：显示当前目录下所有文件/目录
            ls(fpVHD, currentClust);
        if (strcmp(command, "vi")==0){ // useage: vi filename.extension 功能：字符形式显示当前目录下所有文件内容
            scanf("%s", command);
            vi(fpVHD, command, currentClust);
        }
        if (strcmp(command, "cd")==0){ // useage: cd ..      cd x/y       cd /root/x/y
            scanf("%s", command);
            command1[0]='\0';
            if ( (tempClust = cd(fpVHD, command, command1, currentClust))!=-1 ) {
                currentClust = tempClust;
                list(fpVHD, currentClust);
            }
            else
                printf("no such directory!\n");
        }
        if (strcmp(command, "mv")==0){
            scanf("%s", command);
            scanf("%s", command1);
            mv(fpVHD, command, command1, currentClust);
        }
        if (strcmp(command, "mkdir")==0){
            scanf("%s", command);
            mdir(fpVHD, command, currentClust);
        }
        if (strcmp(command, "cp")==0){
            scanf("%s", command);
            vi(fpVHD, command, currentClust);
        }
        if (strcmp(command, "rm")==0){
            scanf("%s", command);
            rm(fpVHD, command, currentClust);
        }
        if (strcmp(command, "mfopen")==0){
            scanf("%s", command);
            mfp = mfopen(fpVHD, command, 0, currentClust);
        }
        if (strcmp(command, "mfread")==0){
            scanf("%d", &size);
            scanf("%d", &count);
            content = malloc(sizeof(char)*size*count);
            len = mfread(fpVHD, content, size, count, mfp);
            for (i=0; i<len; i++) {
                printf("%c",content[i]);
            }
            free(content);
        }
        if (strcmp(command, "mfwrite")==0){
            scanf("%d", &size);
            scanf("%d", &count);
            content = malloc(sizeof(char)*size*count);
            scanf("%s", content);
            len = mfwrite(fpVHD, content, size, count, mfp);
        }
        if (strcmp(command, "mfclose")==0){
            mfclose(mfp);
        }
        if (strcmp(command, "mfp")==0){
            print_mfp(mfp);
        }
        
    }
    fclose(fpVHD);
    return 0;
}
