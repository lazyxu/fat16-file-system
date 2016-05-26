# fat16-file-system
fat16文件系统简单模拟（腊鸡版）
还有很多相对复杂的情况没考虑，要去赶别的大程了先放一边orz
不过还是对文件系统挺感兴趣，暑假应该要好好学习一下

file fat.vhd
fat.vhd: x86 boot sector; partition 1: ID=0x1, starthead 2, startsector 128, 14336 sectors, code offset 0xc0, OEM-ID "      м", Bytes/sector 190, sectors/cluster 124, reserved sectors 191, FATs 6, root entries 185, sectors 64514 (volumes <=32 MB) , Media descriptor 0xf3, sectors/FAT 20644, heads 6, hidden sectors 309755, sectors 2147991229 (volumes > 32 MB) , physical drive 0x7e, dos < 4.0 BootSector (0x0)

FAT16文件系统简介
http://blog.csdn.net/yeruby/article/details/41978199

硬盘基本知识
http://bbs.mydigit.cn/read.php?tid=331754

fat文件结构
http://blog.chinaunix.net/uid-24611346-id-3246892.html

长文件名存储
http://blog.csdn.net/wchp314/article/details/5536088
0X0c
如果目录项需要长文件名项或全部为大写，该位为0x00；如果该目录项全部为小写：directory设置为0x08、file设置为0x18；这样做确实可以区分出大小写的显示；如果大小写都有，则用长文件名存储
假设文件名11个字符组成字符串shortname[],校验和用chknum表示。得到过程如下：
    int i，j,chknum=0;
    for (i=11; i>0; i--)
        chksum = ((chksum & 1) ? 0x80 : 0) + (chksum >> 1) + shortname[j++];

目录项属性
http://www.th7.cn/system/win/201408/66832.shtml

4MB一下的就默认格式化为FAT12、512MB以下的就是FAT16、32GB以下的就是FAT32；32GB以上的就要用现在的NTFS文件系统了
FAT16单个文件大小不能超过2G  FAT32单个文件不能超过4G
FAT16 2个字节描述来FAT表项，FAT32 4个字节描述
FAT16删除文件，文件首字节标记E5，FAT表项不变，数据区不变。  FAT32文件删除，文件首字节标记为E5，根目录高位簇号清零
FAT16格式化，根目录清零，FAT表清零，子目录下数据不变。  FAT32格式化，根目录清零，FAT表清零，子目录高位簇号清零

编写程序模拟磁盘工作。以大文件模拟磁盘，建立FAT、文件目录等数据结构，确定文件操作策略，编写基于虚拟磁盘的文件操作函数： 
mfopen：寻找文件目录，分配文件缓冲区，文件指针初始化。
mFILE mfopen(filename,mode)
mFILE {
char *filename;
int modeFlag;
int clusterN;//文件所占簇的个数
unsigned short int *clusterList;//文件所占簇
int currentFp;
}
mfread：读文件。实际是将数据从系统文件缓冲区拷贝到程序变量。如果文件指针超过当前文件缓冲区，则读入下一簇到缓冲区。 
int mfread(char *content, int size, int count, mFILE mfp)
mfwrite：写文件。将程序变量拷贝到文件缓冲区。如果超过缓冲区，则将当前缓冲区写入磁盘(VHD)，准备下一段内容。 
int mfwrite(char *content, int size, int count, mFILE mfp)
mfseek：移动文件指针。 
int mfseek(mFILE mfp, long offset, int where) where{mSEEK_SET, mSEEK_CUR, mSEEK_END}
mfclose：文件关闭。特别对于写文件，要将文件缓冲区内容存盘(写VHD)，并同时修改FAT与目录区。 
int mfclose(mFILE mfp)
注意：要尽量少读VHD(磁盘)。读写VHD虚拟磁盘只能整块(簇)读写，每次能且只能读写一个块(簇)。所以基础的VHD读写可编写以下函数： 
int clusterRead(byte&, int); //int为簇号 
int clusterWrite(byte*, int);
参考C语言的文件操作。利用这些函数直接读写虚拟磁盘文件。虚拟磁盘只能按块(簇)读写，但实际文件可以读一个int、float、byte等。
