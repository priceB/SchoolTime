/*
Copyright (c) 2015 , Ben624

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, 
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
THIS APPLICATION CAN BE SHARED ACROSS OWNERS OF THE PEBBLE, BUT CAN NOT BE PUBLISHED PUBLICLY ON THE PEBBLE APP STORE. 

Permission to use, copy, or modify, this software for any purpose with or without fee is hereby granted, provided that the above copyright notice and this permission notice appear in all copies.
*/

/*
The structs and methods shown below take the passed in color string and return the actual color. 
It isn't recommended to change this unless more colors are added. 
*/

#include "pebble.h"
struct colorLookup{
	char *colorName;
	int colorValue;
};
 
struct colorLookup colors[]= {
	{"AAFFAA",0xAAFFAA},
	{"55FF55",0x55FF55},
	{"AAFF55",0xAAFF55},
	{"55FF00",0x55FF00},
	{"00FF00",0x00FF00},
	{"00FF55",0x00FF55},		
	{"55FFAA",0x55FFAA},	
	{"AAFF00",0xAAFF00},
	{"00AA00",0x00AA00},
	{"00AA55",0x00AA55},
	{"00FFAA",0x00FFAA},
	{"55FFFF",0x55FFFF},
	{"AAFFFF",0xAAFFFF},
	{"55AA00",0x55AA00},
	{"55AA55",0x55AA55},
	{"00AAAA",0x00AAAA},
	{"00FFFF",0x00FFFF},
	{"FFFFAA",0xFFFFAA},
	{"AAAA55",0xAAAA55},
	{"005500",0x005500},
	{"55AAAA",0x55AAAA},
	{"FFFF55",0xFFFF55},
	{"FFFF00",0xFFFF00},
	{"AAAA00",0xAAAA00},
	{"555500",0x555500},
	{"005555",0x005555},
	{"0055AA",0x0055AA},
	{"00AAFF",0x00AAFF},
	{"55AAFF",0x55AAFF},
	{"AAAAAA",0xAAAAAA},
	{"FFAA55",0xFFAA55},
	{"FFAA00",0xFFAA00},
	{"0000AA",0x0000AA},
	{"0000FF",0x0000FF},
	{"0055FF",0x0055FF},
	{"FFFFFF",0xFFFFFF},
	{"000000",0x000000},
	{"FF5500",0xFF5500},
	{"AA5500",0xAA5500},
	{"000055",0x000055},
	{"5500FF",0x5500FF},
	{"5555FF",0x5555FF},
	{"555555",0x555555},
	{"FF0000",0xFF0000},
	{"AA5555",0xAA5555},
	{"550000",0x550000},
	{"550055",0x550055},
	{"5500AA",0x5500AA},
	{"5555AA",0x5555AA},
	{"AAAAFF",0xAAAAFF},
	{"FFAAAA",0xFFAAAA},
	{"FF5555",0xFF5555},
	{"FF0055",0xFF0055},
	{"AA0000",0xAA0000},
	{"AA0055",0xAA0055},
	{"AA00AA",0xAA00AA},
	{"AA00FF",0xAA00FF},
	{"FF00AA",0xFF00AA},
	{"FF00FF",0xFF00FF},
	{"AA55AA",0xAA55AA},
	{"AA55FF",0xAA55FF},
	{"FF55AA",0xFF55AA},
	{"FF55FF",0xFF55FF},
	{"FFAAFF",0xFFAAFF},
};



int findColor(char *str){
	for(int i =0;i<(int)(sizeof(colors)/sizeof(struct colorLookup)); i++){
		if(strcmp(str,colors[i].colorName) == 0){
			return colors[i].colorValue;
		}
	}
	return 0;
}