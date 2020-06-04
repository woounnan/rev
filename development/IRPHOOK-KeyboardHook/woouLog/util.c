#include <ntddk.h>


void addListTail(PLIST_ENTRY dst, PLIST_ENTRY src)
{
	dst->Blink = src;
	src->Flink = dst;
}

void deleteListTail()
{

}