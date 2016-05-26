//
//  info.c
//  test
//
//  Created by Meteor on 16/5/25.
//  Copyright © 2016年 meteor. All rights reserved.
//

#include "info.h"

void print_general_info()
{
    printf("warning:千万不要依靠引导扇区数据结构中的BS_FileSysType字段(在FAT12中位于偏移54处)来比较这个字符串，很多时候这个字段很不准确，或许压根就没有(全是0或空格20h)，这个字段存在与否或内容是什么，与文件系统一点关系也没有，采取什么样的文件系统只有一个计算标准：此卷所有簇的数量。\n\
           簇总数=(逻辑扇区总数-(隐藏扇区+保留扇区+FAT表数*FAT表所占用扇区+根目录所占用扇区))/每簇扇区数\n\
           注：逻辑扇区总数位于引导扇区数据结构偏移19处(字段BPB_TotSec16，长度为2字节)或32处(字段BPB_TotSec32，长度为4字节)，例如，在FAT12默认值中：(2880-(0+1+2*9+14))/1=2847\n\
           卷中簇的总数小于4085的为FAT12，总数大于或等于4085并且小于65525的为FAT16，总数大于或等于65525的为FAT32。\n\n");
    printf("general struct:\n\
           +0x0000 ~ +0xffff: 0柱面0磁头1扇区: MBR + DPT + '55AA'(+0x000 ~ +0xfff)\n\
           62个保留扇区(系统扇区)(+0x1000 ~ +0xffff)\n\
           +0x10000 ~ +0x1ffff: 0柱面1磁头1扇区: 操作系统DBR(+0x10000 ~ +0x10fff)\n\
           FAT1(+0x11000 ~ +0x17fff)\n\
           FAT2(+0x18000 ~ +0x1efff)\n\
           根目录(+0x1f000 ~ +0x1ffff)\n\n\
           创建时间:这个文件存在于本机上的时间\n\
           修改时间:文件最近一次进行修改并保存的时间\n\
           访问时间:最近一次打开该文档的时间\n\n");
}

void print_0x0000_to_0xffff(FILE* fpVHD)
{
    unsigned char byte;
    unsigned short int word;
    unsigned int dword;
    char type[0x1d][30]={"分区未用", "FAT12", "XENIX  root", "XENIX  usr",
        "FAT16 分区小于32M", "Extended 拓展分区", "FAT16", "HPFS/NTFS",
        "AIX", "AIX bootable", "OS/2  Boot Manage", "OWin95 FAT32",
        "OWin95 FAT32", "unknown", "OWin95 FAT16", "Win95 Extended > 8GB",
        
        "OPUS", "Hidden FAT12", "Compaq diagmost", "unknown",
        "Hidden FAT16 < 32MB", "unknown", "HiddenFAT16",  "Hidden HPFS/NTFS",
        "AST Windows swap", "unknown", "unknown", "Hidden FAT32", "Hidden FAT32 partition" };
    
    printf("+0x000 ~ +0x1ff: MBR + DPT + '55AA' \n\
           位于整个硬盘的0柱面0磁头1扇区(可以看作是硬盘的第一个扇区)\n\
           在总共512byte的主引导记录中\n\
           MBR的引导程序占了其中的前446个字节(偏移0H~偏移1BDH)\n\
           随后的64个字节(偏移1BEH~偏移1FDH)为DPT(Disk PartitionTable，硬盘分区表)\n\
           最后的两个字节“55 AA”(偏移1FEH~偏移1FFH)是分区有效结束标志。\n\n");
    
    printf("+0x000 ~ +0x1bd: MBR(master boot record,主引导记录)\n\
           bios在执行自己固有的程序以后就会jump到mbr中的第一条指令,将系统的控制权交由mbr来执行。\n\n");
    
    printf("+0x1be ~ +0x1fd: DPT(Disk PartitionTable,硬盘分区表)\n");
    int offset = 0x1be;
    fseek(fpVHD, offset, SEEK_SET);
    fread(&byte, sizeof(char), 1, fpVHD);
    printf("+%x %02x: 引导指示符(Boot Indicator) %x ", offset, byte, byte);
    if ( byte == 0x80 ) printf("活动分区\n");
    else if ( byte == 0x00 ) printf("非活动分区\n");
    else printf("错误的值\n");
    offset++;
    
    fread(&byte, sizeof(byte), 1, fpVHD);
    printf("+%x %02x: 开始磁头(Starting Head) %x \n", offset, byte, byte);
    offset++;
    
    fread(&byte, sizeof(byte), 1, fpVHD);
    printf("+%x %02x: 开始扇区(Starting Sector)(只用了0~5位) %x \n", offset, byte, byte&0b11111);
    offset++;
    
    word = (byte>>6);
    fread(&byte, sizeof(byte), 1, fpVHD);
    word = (byte<<2) | word;
    printf("+%x %02x: 开始柱面(Starting Cylinder)(加上+%x的6~7位) %x \n", offset, byte, offset-1, word);
    offset++;
    
    fread(&byte, sizeof(byte), 1, fpVHD);
    printf("+%x %02x: 系统ID(System ID)(定义了分区的类型) %x %s\n", offset, byte, byte, type[byte]);
    offset++;
    
    fread(&byte, sizeof(byte), 1, fpVHD);
    printf("+%x %02x: 结束磁头(Ending Head) %x \n", offset, byte, byte);
    offset++;
    
    fread(&byte, sizeof(byte), 1, fpVHD);
    printf("+%x %02x: 结束扇区(Ending Sector)(只用了0~5位) %x \n", offset, byte, byte&0b11111);
    offset++;
    
    word = (byte>>6);
    fread(&byte, sizeof(byte), 1, fpVHD);
    word = (byte<<2) | word;
    printf("+%x %02x: 结束柱面(Ending Cylinder)(加上+%x的6~7位) %x \n", offset, byte, offset-1, word);
    offset++;
    
    fread(&dword, sizeof(dword), 1, fpVHD);
    printf("+%x %08x: 相对扇区数(Relative Sectors)(从该磁盘的开始到该分区的开始的位移量，以扇区来计算) %x \n", offset, dword, dword);
    offset += 4;
    
    fread(&dword, sizeof(dword), 1, fpVHD);
    printf("+%x %08x: 总扇区数(Total Sectors)(该分区中的扇区总数) %x \n", offset, dword, dword);
    offset += 4;
    printf("+%x 55 AA: 有效结束标志 \n\n", offset);
    
}

void print_fat32_0x10000_to_0x1ffff(FILE* fpVHD)
{
    unsigned char byte;
    unsigned short int word;
    unsigned int dword;
    unsigned long str[2];
    int offset;
    
    offset = 0x10000;
    fseek(fpVHD, offset, SEEK_SET);
    printf("+0x10000 ~ +0x10fff: DBR区(DOS BOOT RECORD)\n\
           即操作系统引导记录区的意思，通常占用分区的第0扇区共512个字节\n\
           在这512个字节中，其实又是由跳转指令，厂商标志和操作系统版本号，BPB(BIOS Parameter Block)，扩展BPB，os引导程序，结束标志几部分组成\n");
    
    fread(&dword, sizeof(byte), 3, fpVHD);
    printf("+%x %06x: 合法的可执行的基于x86的CPU指令,通常是一条跳转指令\n\
           该指令负责跳过接下来的几个不可执行的字节(BPB和扩展BPB)\n\
           跳到操作系统引导代码部分) %x \n", offset, dword&0xffffff, dword&0xffffff);
    offset += 3;
    
    fread(str, sizeof(byte), 8, fpVHD);
    str[1]='\0';
    printf("+%x %16lx: OEM ID标识了格式化该分区的操作系统的名称和版本号\n\
           Windows 2000格式化该盘是在FAT16和FAT32磁盘上的该字段中记录了“MSDOS 5.0”\n\
           在NTFS磁盘上(关于ntfs，另述)，Windows 2000记录的是“NTFS”\n\
           Windows 95格式化的磁盘上OEM ID字段出现“MSWIN4.0”\n\
           Windows 95 OSR2和Windows 98格式化的磁盘上OEM ID字段出现“MSWIN4.1” %s \n\n", offset, *str, (char *)str);
    offset += 8;
    
    printf("+%x ~ +%x : BPB(BIOS Parameter Block) \n", offset, offset+53);
    fread(&word, sizeof(byte), 2, fpVHD);
    printf("+%x %04x: 扇区字节数(Bytes Per Sector) 硬件扇区的大小\n\
           本字段合法的十进制值有512、1024、2048和4096 %d \n", offset, word, word);
    offset += 2;
    
    fread(&byte, sizeof(byte), 1, fpVHD);
    printf("+%x %04x: 每簇扇区数(Sectors Per Cluster) 一簇中的扇区数 \n\
           FAT32文件系统只能跟踪有限个簇(最多为4 294 967 296个)\n\
           本字段的合法十进制值有1、2、4、8、16、32、64和128 %d \n", offset, byte, byte);
    offset += 1;
    
    fread(&word, sizeof(byte), 2, fpVHD);
    printf("+%x %04x: 保留扇区数(Reserved Sector)\n\
           第一个FAT开始之前的扇区数，包括引导扇区 %d \n", offset, word, word);
    offset += 2;
    
    fread(&byte, sizeof(byte), 1, fpVHD);
    printf("+%x %04x: FAT数(Number of FAT) 该分区上FAT的副本数 %d \n", offset, byte, byte);
    offset += 1;
    
    fread(&word, sizeof(byte), 2, fpVHD);
    printf("+%x %04x: 根目录项数(Root Entries)\n\
           只有FAT12/FAT16使用此字段\n\
           对FAT32分区而言,本字段必须设置为0 %d \n", offset, word, word);
    offset += 2;
    
    fread(&word, sizeof(byte), 2, fpVHD);
    printf("+%x %04x: 小扇区数(Small Sector)\n\
           只有FAT12/FAT16使用此字段\n\
           对FAT32分区而言,本字段必须设置为0 %d \n", offset, word, word);
    offset += 2;
    
    fread(&byte, sizeof(byte), 1, fpVHD);
    printf("+%x %04x: 媒体描述符( Media Descriptor)提供有关媒体被使用的信息\n\
           值0xF8表示硬盘，0xF0表示高密度的3.5寸软盘\n\
           媒体描述符要用于MS-DOS FAT16磁盘，在Windows 2000中未被使用 0x%x \n", offset, byte, byte);
    offset += 1;
    
    fread(&word, sizeof(byte), 2, fpVHD);
    printf("+%x %04x: 每FAT扇区数(Sectors Per FAT)\n\
           只被FAT12/FAT16所使用\n\
           对FAT32分区而言,本字段必须设置为0 %d \n", offset, word, word);
    offset += 2;
    
    fread(&word, sizeof(byte), 2, fpVHD);
    printf("+%x %04x: 每道扇区数(Sectors Per Track)\n\
           包含使用INT13h的磁盘的“每道扇区数”几何结构值\n\
           该分区被多个磁头的柱面分成了多个磁道 %d \n", offset, word, word);
    offset += 2;
    
    fread(&word, sizeof(byte), 2, fpVHD);
    printf("+%x %04x: 磁头数(Number of Head)\n\
           本字段包含使用INT 13h的磁盘的“磁头数”几何结构值 %d \n", offset, word, word);
    offset += 2;
    
    fread(&dword, sizeof(dword), 1, fpVHD);
    printf("+%x %04x: 隐藏扇区数(Hidden Sector) 该分区上引导扇区之前的扇区数\n\
           在引导序列计算到根目录的数据区的绝对位移的过程中使用了该值\n\
           本字段一般只对那些在中断13h上可见的媒体有意义\n\
           在没有分区的媒体上它必须总是为0 %d \n", offset, dword, dword);
    offset += 4;
    
    fread(&dword, sizeof(dword), 1, fpVHD);
    printf("+%x %04x: 总扇区数(Large Sector) 本字段包含FAT32分区中总的扇区数 %d \n", offset, dword, dword);
    offset += 4;
    
    fread(&dword, sizeof(dword), 1, fpVHD);
    printf("+%x %04x: 每FAT扇区数(Sectors Per FAT)(只被FAT32使用)该分区每个FAT所占的扇区数\n\
           计算机利用这个数和FAT数以及隐藏扇区数(本表中所描述的)来决定根目录从哪里开始\n\
           该计算机还可以从目录中的项数决定该分区的用户数据区从哪里开始 %u \n", offset, dword, dword);
    offset += 4;
    
    fread(&word, sizeof(byte), 2, fpVHD);
    printf("+%x %04x: 扩展标志(Extended Flag)(只被FAT32使用)该两个字节结构中各位的值为：\n\
           位0-3：活动 FAT数(从0开始计数，而不是1),只有在不使用镜像时才有效 %d\n\
           位4-6：保留\n\
           位7：0值意味着在运行时FAT被映射到所有的FAT\n\
           1值表示只有一个FAT是活动的 %x\n\
           位8-15：保留 %d \n", offset, word, word&0b111, word&0b10000000, word);
    offset += 2;
    
    fread(&word, sizeof(byte), 2, fpVHD);
    printf("+%x %04x: 文件系统版本(File ystem Version)\n\
           只供FAT32使用,高字节是主要的修订号，而低字节是次要的修订号\n\
           本字段支持将来对该FAT32媒体类型进行扩展\n\
           如果本字段非零，以前的Windows版本将不支持这样的分区 %04x \n", offset, word, word);
    offset += 2;
    
    fread(&dword, sizeof(dword), 1, fpVHD);
    printf("+%x %04x: 根目录簇号(Root Cluster Number)\n\
           只供FAT32使用,根目录第一簇的簇号 %u \n", offset, dword, dword);
    offset += 4;
    
    fread(&word, sizeof(byte), 2, fpVHD);
    printf("+%x %04x: 文件系统信息扇区号(File System Information SectorNumber)(只供FAT32使用)\n\
           FAT32分区的保留区中的文件系统信息(File System Information, FSINFO)结构的扇区号\n\
           %04x \n", offset, word, word);
    offset += 2;
    
    fread(&word, sizeof(byte), 2, fpVHD);
    printf("+%x %04x:备份引导扇区(只供FAT32使用)\n\
           为一个非零值，这个非零值表示该分区保存引导扇区的副本的保留区中的扇区号 %04x \n", offset, word, word);
    offset += 2;
    
    fseek(fpVHD, 12, SEEK_CUR);
    printf("+%x ~ +%x : 保留(只供FAT32使用)供以后扩充使用的保留空间 \n\n", offset, offset+12);
    offset += 12;
    
    fread(&byte, sizeof(byte), 1, fpVHD);
    printf("+%x ~ +%x : 扩展BPB \n", offset, offset+26);
    printf("+%x %04x: 物理驱动器号( Physical Drive Number) 与BIOS物理驱动器号有关\n\
           软盘驱动器被标识为0x00，物理硬盘被标识为0x80，而与物理磁盘驱动器无关\n\
           一般地，在发出一个INT13h BIOS调用之前设置该值，具体指定所访问的设备\n\
           只有当该设备是一个引导设备时，这个值才有意义 0x%x \n", offset, byte, byte);
    offset += 1;
    
    fread(&byte, sizeof(byte), 1, fpVHD);
    printf("+%x %04x: 保留(Reserved) FAT32分区总是将本字段的值设置为0 %x \n", offset, byte, byte);
    offset += 1;
    
    fread(&byte, sizeof(byte), 1, fpVHD);
    printf("+%x %04x: 扩展引导标签(Extended Boot Signature)\n\
           本字段必须要有能被Windows 2000所识别的值0x28或0x29 %x \n", offset, byte, byte);
    offset += 1;
    
    fread(&dword, sizeof(dword), 1, fpVHD);
    printf("+%x %04x: 分区序号(Volume Serial Number)\n\
           在格式化磁盘时所产生的一个随机序号，它有助于区分磁盘 0x%x \n", offset, dword, dword);
    offset += 4;
    
    fread(str, sizeof(byte), 11, fpVHD);
    printf("+%x %06lx%016lx: 卷标(Volume Label)\n\
           本字段只能使用一次，它被用来保存卷标号\n\
           现在，卷标被作为一个特殊文件保存在根目录中 %s \n", offset, *(str+1), *str, (char *)str);
    offset += 11;
    
    fread(str, sizeof(byte), 8, fpVHD);
    str[1]='\0';
    printf("+%x %16lx: 系统ID(System ID)\n\
           FAT32文件系统中一般取为FAT32 %s \n", offset, *str, (char *)str);
    offset += 8;
    
    printf("+%x : DBR的偏移0x5A开始的数据为操作系统引导代码。这是由偏移0x00开始的跳转指令所指向的。在图8所列出的偏移0x00~0x02的跳转指令EB 58 90清楚地指明了OS引导代码的偏移位置。jump 58H加上跳转指令所需的位移量，即开始于0x5A。此段指令在不同的操作系统上和不同的引导方式上，其内容也是不同的\n", offset);
    offset += 420;
    
    printf("+%x 55 AA: 有效结束标志 \n", offset);
    offset += 2;
}

void print_fat16_0x10000_to_0x1ffff(FILE* fpVHD)
{
    unsigned char byte;
    unsigned short int word;
    unsigned int dword;
    unsigned long str[2];
    int offset;
    
    offset = 0x10000;
    fseek(fpVHD, offset, SEEK_SET);
    printf("+0x10000 ~ +0x10fff: DBR区(DOS BOOT RECORD)\n\
           即操作系统引导记录区的意思，通常占用分区的第0扇区共512个字节\n\
           在这512个字节中，其实又是由跳转指令，厂商标志和操作系统版本号，BPB(BIOS Parameter Block)，扩展BPB，os引导程序，结束标志几部分组成\n");
    
    fread(&dword, sizeof(byte), 3, fpVHD);
    printf("+%x %06x: 合法的可执行的基于x86的CPU指令,通常是一条跳转指令\n\
           该指令负责跳过接下来的几个不可执行的字节(BPB和扩展BPB)\n\
           跳到操作系统引导代码部分) %x \n", offset, dword&0xffffff, dword&0xffffff);
    offset += 3;
    
    fread(str, sizeof(byte), 8, fpVHD);
    str[1]='\0';
    printf("+%x %16lx: OEM ID标识了格式化该分区的操作系统的名称和版本号\n\
           Windows 2000格式化该盘是在FAT16和FAT32磁盘上的该字段中记录了“MSDOS 5.0”\n\
           在NTFS磁盘上(关于ntfs，另述)，Windows 2000记录的是“NTFS”\n\
           Windows 95格式化的磁盘上OEM ID字段出现“MSWIN4.0”\n\
           Windows 95 OSR2和Windows 98格式化的磁盘上OEM ID字段出现“MSWIN4.1” %s \n", offset, *str, (char *)str);
    offset += 8;
    
    printf("+%x ~ +%x: BPB(BIOS Parameter Block) \n", offset, offset+25);
    fread(&word, sizeof(byte), 2, fpVHD);
    printf("+%x %04x: 扇区字节数(Bytes Per Sector) 硬件扇区的大小\n\
           本字段合法的十进制值有512、1024、2048和4096 %d \n", offset, word, word);
    offset += 2;
    
    fread(&byte, sizeof(byte), 1, fpVHD);
    printf("+%x %04x: 每簇扇区数(Sectors Per Cluster) 一簇中的扇区数 \n\
           FAT16文件系统只能跟踪有限个簇(最多为65536个)\n\
           本字段的合法十进制值有1、2、4、8、16、32、64和128 %d \n", offset, byte, byte);
    offset += 1;
    
    fread(&word, sizeof(byte), 2, fpVHD);
    printf("+%x %04x: 保留扇区数(Reserved Sector)\n\
           第一个FAT开始之前的扇区数，包括引导扇区 %d \n", offset, word, word);
    offset += 2;
    
    fread(&byte, sizeof(byte), 1, fpVHD);
    printf("+%x %04x: FAT数(Number of FAT) 该分区上FAT的副本数 %d \n", offset, byte, byte);
    offset += 1;
    
    fread(&word, sizeof(byte), 2, fpVHD);
    printf("+%x %04x: 根目录项数(Root Entries)\n\
           能够保存在该分区的根目录文件夹中的32个字节长的文件和文件夹名称项的总数\n\
           在一个典型的硬盘上，本字段的值为512\n\
           其中一个项常常被用作卷标号(Volume Label)，长名称的文件和文件夹每个文件使用多个项\n\
           文件和文件夹项的最大数一般为511，但是如果使用的长文件名，往往都达不到这个数%d \n", offset, word, word);
    offset += 2;
    
    fread(&word, sizeof(byte), 2, fpVHD);
    printf("+%x %04x: 小扇区数(Small Sector)\n\
           该分区上的扇区数，表示为16位(<65536)\n\
           对大于65536个扇区的分区来说，本字段的值为0，而使用大扇区数来取代它 %d \n", offset, word, word);
    offset += 2;
    
    fread(&byte, sizeof(byte), 1, fpVHD);
    printf("+%x %04x: 媒体描述符( Media Descriptor)提供有关媒体被使用的信息\n\
           值0xF8表示硬盘，0xF0表示高密度的3.5寸软盘\n\
           媒体描述符要用于MS-DOS FAT16磁盘，在Windows 2000中未被使用 0x%x \n", offset, byte, byte);
    offset += 1;
    
    fread(&word, sizeof(byte), 2, fpVHD);
    printf("+%x %04x: 每FAT扇区数(Sectors Per FAT)\n\
           只被FAT12/FAT16所使用\n\
           对FAT32分区而言,本字段必须设置为0 %d \n", offset, word, word);
    offset += 2;
    
    fread(&word, sizeof(byte), 2, fpVHD);
    printf("+%x %04x: 每道扇区数(Sectors Per Track) %d \n", offset, word, word);
    offset += 2;
    
    fread(&word, sizeof(byte), 2, fpVHD);
    printf("+%x %04x: 磁头数(Number of Head) %d \n", offset, word, word);
    offset += 2;
    
    fread(&dword, sizeof(dword), 1, fpVHD);
    printf("+%x %04x: 隐藏扇区数(Hidden Sector) 该分区上引导扇区之前的扇区数\n\
           在引导序列计算到根目录和数据区的绝对位移的过程中使用了该值 %d \n", offset, dword, dword);
    offset += 4;
    
    fread(&dword, sizeof(dword), 1, fpVHD);
    printf("+%x %04x: 大扇区数(Large Sector)\n\
           如果小扇区数字段的值为0，本字段就包含该FAT16分区中的总扇区数\n\
           如果小扇区数字段的值不为0，那么本字段的值为0 %d \n", offset, dword, dword);
    offset += 4;
    
    fread(&byte, sizeof(byte), 1, fpVHD);
    printf("+%x ~ +%x : 扩展BPB \n", offset, offset+26);
    printf("+%x %04x: 物理驱动器号( Physical Drive Number) 与BIOS物理驱动器号有关\n\
           软盘驱动器被标识为0x00，物理硬盘被标识为0x80，而与物理磁盘驱动器无关\n\
           一般地，在发出一个INT13h BIOS调用之前设置该值，具体指定所访问的设备\n\
           只有当该设备是一个引导设备时，这个值才有意义 0x%x \n", offset, byte, byte);
    offset += 1;
    
    fread(&byte, sizeof(byte), 1, fpVHD);
    printf("+%x %04x: 保留(Reserved) FAT16分区一般将本字段的值设置为0 %x \n", offset, byte, byte);
    offset += 1;
    
    fread(&byte, sizeof(byte), 1, fpVHD);
    printf("+%x %04x: 扩展引导标签(Extended Boot Signature)\n\
           本字段必须要有能被Windows 2000所识别的值0x28或0x29 %x \n", offset, byte, byte);
    offset += 1;
    
    fread(&dword, sizeof(dword), 1, fpVHD);
    printf("+%x %04x: 卷序号(Volume Serial Number)\n\
           在格式化磁盘时所产生的一个随机序号，它有助于区分磁盘 0x%x \n", offset, dword, dword);
    offset += 4;
    
    fread(str, sizeof(byte), 11, fpVHD);
    printf("+%x %06lx%016lx: 卷标(Volume Label)\n\
           本字段只能使用一次，它被用来保存卷标号\n\
           现在，卷标被作为一个特殊文件保存在根目录中 %s \n", offset, *(str+1), *str, (char *)str);
    offset += 11;
    
    fread(str, sizeof(byte), 8, fpVHD);
    str[1]='\0';
    printf("+%x %16lx: 文件系统类型(File System Type)\n\
           根据该磁盘格式，该字段的值可以为FAT、FAT12或FAT16 %s \n", offset, *str, (char *)str);
    offset += 8;
    
    printf("+%x : DBR的偏移0x5A开始的数据为操作系统引导代码。这是由偏移0x00开始的跳转指令所指向的。在图8所列出的偏移0x00~0x02的跳转指令EB 58 90清楚地指明了OS引导代码的偏移位置。jump 58H加上跳转指令所需的位移量，即开始于0x5A。此段指令在不同的操作系统上和不同的引导方式上，其内容也是不同的\n", offset);
    offset += 448;
    
    printf("+%x 55 AA: 有效结束标志 \n", offset);
    offset += 2;
}

void print_clust_string(FILE *fpVHD, unsigned short int word)
{
    while (1) {
        if (word==0xffff) {
            break;
        }
        printf("%04x ", word);
        fseek(fpVHD, 0x11000+word*2, SEEK_SET);
        fread(&word, sizeof(word), 1, fpVHD);
    }
}

void print_FAT1_0x11000_to_0x17fff(FILE* fpVHD)
{
    unsigned short int word;
    unsigned int dword;
    int offset;
    
    offset = 0x11000;
    fseek(fpVHD, offset, SEEK_SET);
    printf("+0x10000 ~ +0x10fff: DBR区(DOS BOOT RECORD)\n\
           即操作系统引导记录区的意思，通常占用分区的第0扇区共512个字节\n\
           在这512个字节中，其实又是由跳转指令，厂商标志和操作系统版本号，BPB(BIOS Parameter Block)，扩展BPB，os引导程序，结束标志几部分组成\n\n");
    
    fread(&dword, sizeof(dword), 1, fpVHD);
    printf("+%x %08x: 介质描述单元 %x \n", offset, dword, dword);
    offset += 4;
    
    while(1)
    {
        fseek(fpVHD, offset, SEEK_SET);
        fread(&word, sizeof(word), 1, fpVHD);
        if (word==0x0000) {
            break;
        }
        printf("+%x %04x: %04x ", offset, word, (offset-0x11000)/2);
        print_clust_string(fpVHD, word);
        printf("\n");
        offset += 2;
    }
}

int print_info(FILE* fpVHD, char choice)
{
    switch (choice) {
        case '1':
            print_general_info();
            break;
        case '2':
            print_0x0000_to_0xffff(fpVHD);
            break;
        case '3':
            print_fat16_0x10000_to_0x1ffff(fpVHD);
            break;
        case '4':
            print_fat32_0x10000_to_0x1ffff(fpVHD);
            break;
        case '5':
            print_FAT1_0x11000_to_0x17fff(fpVHD);
            break;
        default:
            return 0;
            break;
    }
    return 1;
}