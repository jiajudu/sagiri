#pragma once
void panic(char* s);
#define assert(x) do{if(!(x)){panic(#x);}}while(0)