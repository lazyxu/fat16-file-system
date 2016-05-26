//
//  info.h
//  test
//
//  Created by Meteor on 16/5/25.
//  Copyright © 2016年 meteor. All rights reserved.
//

#ifndef info_h
#define info_h

#include <stdio.h>

void print_general_info();
void print_0x0000_to_0xffff(FILE* fpVHD);
void print_fat32_0x10000_to_0x1ffff(FILE* fpVHD);
void print_fat16_0x10000_to_0x1ffff(FILE* fpVHD);
void print_clust_string(FILE *fpVHD, unsigned short int word);
void print_FAT1_0x11000_to_0x17fff(FILE* fpVHD);
int print_info(FILE* fpVHD, char choice);

#endif /* info_h */
