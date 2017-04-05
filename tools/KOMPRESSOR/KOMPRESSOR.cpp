/*

    This program compresses things with QuickLZ. Yeah.

    Copyright 2011 Josh Tari/Captain DC

    This file is part of the Dolmexica Engine.

    The Dolmexica Engine is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The Dolmexica Engine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with The Dolmexica Engine. If not, see <http://www.gnu.org/licenses/>.

    This tool depends on the QuickLZ compression Archive:

// Fast data compression library
// Copyright (C) 2006-2011 Lasse Mikkel Reinhold
// lar@quicklz.com
//
// QuickLZ can be used for free under the GPL 1, 2 or 3 license (where anything
// released into public must be open source) or under a commercial license if such
// has been acquired (see http://www.quicklz.com/order.html). The commercial license
// does not cover derived or ported versions created by third parties under GPL.





*/
#include <iostream>
using namespace std;

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "quicklz.h"
#include "quicklz.c"

void FetchFileNames(char *argv[], int argc, char KMGFileName[], char PKGFileName[]){
         int i;
         int k;
         int NameArrayLength;
    char* KMGFileNameChar;
         int Bluff;
         char FileName[100];

         KMGFileNameChar = argv[argc];
         Bluff = sprintf(KMGFileName, "%s", KMGFileNameChar);
         NameArrayLength = sizeof(KMGFileName);
         i = 0;
         while(KMGFileName[i] != '\0'){
             i++;
             }
         for(k=0; k<i-4; k++) FileName[k] = KMGFileName[k];
         FileName[k] = '\0';

         Bluff = sprintf(PKGFileName, "%s.pkg", FileName);


    }

int main(int argc, char *argv[]){
    size_t len;
    int Bluff;


    char FileName[FILENAME_MAX];
    char KMGFileName[FILENAME_MAX];
    char PKGFileName[FILENAME_MAX];

FILE *KMGFile, *PKGFile;

    char *src, *dst;
	qlz_state_compress *state_compress = (qlz_state_compress *)malloc(sizeof(qlz_state_compress));
    size_t length;

    if(argc == 1){

    printf("\n");
    printf("Dolmexica Engine Kompressor (Image to PKG Converter)\n");
    printf("Version 1.0.0 - Source available at dolmexicaengine.sourceforge.net\n");
    printf("\n");
    printf("Usage:\n");
    printf("\n");
    printf("This is a drag and drop tool. Select all the files you wish to Kompress and move them on the executable. Supports batch processing and creates the output files in the folder the input files are located at.\n");
    printf("\n");
    printf("\n");
    printf("Copyright 2012 - ...Meh.\n");
    printf("This is an application which just takes QuickLZ and turns it into a usable drag-'n-drop tool. There is nothing for me (Josh Tari) to take credit for, all of that goes to the QuickLZ dev team. Check out their website at 'http://www.quicklz.com/'. This tool complies with their rule that tools using QLZ must be released under the GNU General Public License (or purchase a license).\n");
    printf("\n");
    printf("License: GNU General Public License v3\n");
    printf("\n");
    }

    while(argc>=2){
    //printf("%d\n", argc);
    FetchFileNames(argv, argc-1, KMGFileName, PKGFileName);





    KMGFile = fopen(KMGFileName, "rb+");
    // allocate source buffer and read file

    PKGFile = fopen(PKGFileName, "wb+");

    // allocate source buffer and read file
    fseek(KMGFile, 0, SEEK_END);
    length = ftell(KMGFile);
    fseek(KMGFile, 0, SEEK_SET);
    src = (char*) malloc(length);
    fread(src, 1, length, KMGFile);

    // allocate "uncompressed size" + 400 for the destination buffer
    dst = (char*) malloc(length + 400);

    // compress and write result
    length = qlz_compress(src, dst, length, state_compress);
    fwrite(dst, length, 1, PKGFile);
    fclose(PKGFile);
    fclose(KMGFile);
    free(dst);
    free(src);

//cin.get();
argc-=1;
    }
return(0);
}
