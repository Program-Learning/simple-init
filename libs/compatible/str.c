#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include "string.h"
#include "stdlib.h"
#include "limits.h"
#include "stdint.h"
#include "defines.h"
#define ALIGN (sizeof(size_t))
#define ONES ((size_t)-1/UCHAR_MAX)
#define HIGHS (ONES *(UCHAR_MAX/2+1))
#define HASZERO(x)(((x)-(ONES))& ~(x)& HIGHS)
#define UNICODE_STRING_MAX    8192
size_t wcslen(const wchar_t*s){
	return(size_t)StrLen((CONST CHAR16*)s);
}
wchar_t*wmemset(wchar_t*s,wchar_t c,size_t n){
	return(wchar_t*)SetMem16(s,(UINTN)(n*sizeof(wchar_t)),(UINT16)c);
}
wchar_t*wcschr(const wchar_t*s,wchar_t c){
	do{if(*s==c)return(wchar_t *)s;}while(*s++!=0);
	return NULL;
}
size_t strlen(const char*buff){
	return(size_t)AsciiStrLen((CONST CHAR8*)buff);
}
size_t strnlen(const char*s,size_t n){
	const char*p=memchr(s,0,n);
	return p?p-s:n;
}
int strcmp(const char*buff1,const char*buff2){
	return(int)AsciiStrCmp((CONST CHAR8*)buff1,(CONST CHAR8*)buff2);
}
int strncmp(const char*buff1,const char*buff2,size_t size){
	return(int)AsciiStrnCmp((CONST CHAR8*)buff1,(CONST CHAR8*)buff2,(UINTN)size);
}
int strcasecmp(const char*buff1,const char*buff2){
	return(int)AsciiStriCmp((CONST CHAR8*)buff1,(CONST CHAR8*)buff2);
}
char*strcpy(char*buff1,const char*buff2){
	AsciiStrCpyS((CHAR8*)buff1,UNICODE_STRING_MAX,(CONST CHAR8*)buff2);
	return buff1;
}
char*strncpy(char*buff1,const char*buff2,size_t size){
	AsciiStrnCpyS((CHAR8*)buff1,UNICODE_STRING_MAX,(CONST CHAR8*)buff2,(UINTN)size);
	return buff1;
}
int strncpyX(char*__restrict s1,const char*__restrict s2,size_t n){
	int NumLeft;
	for(;n!=0;--n)if((*s1++=*s2++)=='\0')break;
	NumLeft=(int)n;
	for(--s1;n!=0;--n)*s1++='\0';
	return NumLeft;
}
char *strcat(char *restrict dest,const char *restrict src){
	strcpy(dest+strlen(dest),src);
	return dest;
}
char *strchrnul(const char *s,int c){
	c=(unsigned char)c;
	if(!c)return(char *)s + strlen(s);
#ifdef __GNUC__
	typedef size_t __attribute__((__may_alias__))word;
	const word *w;
	for(;(uintptr_t)s % ALIGN;s++)
		if(!*s || *(unsigned char *)s==c)return(char *)s;
	size_t k=ONES * c;
	for(w=(void *)s;!HASZERO(*w)&& !HASZERO(*w^k);w++);
	s=(void *)w;
#endif
	for(;*s && *(unsigned char *)s !=c;s++);
	return(char *)s;
}
char *strchr(const char *s,int c){
	char *r=strchrnul(s,c);
	return *(unsigned char *)r==(unsigned char)c ? r : 0;
}
char*strdup(const char*str){
	size_t len;
	char*buff;
	len=strlen(str)+1;
	if(!(buff=malloc(len)))return NULL;
	memcpy(buff,str,len);
	return buff;
}
char*strndup(const char*s,size_t n){
	size_t l=strnlen(s,n);
	char*d=malloc(l+1);
	if(!d)return NULL;
	memcpy(d,s,l);
	d[l]=0;
	return d;
}
char *basename(char *s){
	size_t i;
	if(!s || !*s)return ".";
	i=strlen(s)-1;
	for(;i&&s[i]=='/';i--)s[i]=0;
	for(;i&&s[i-1]!='/';i--);
	return s+i;
}
static char *twobyte_strstr(const unsigned char *h,const unsigned char *n){
	uint16_t nw=n[0]<<8 | n[1],hw=h[0]<<8 | h[1];
	for(h++;*h && hw !=nw;hw=hw<<8 | *++h);
	return *h ?(char *)h-1 : 0;
}
static char *threebyte_strstr(const unsigned char *h,const unsigned char *n){
	uint32_t nw=(uint32_t)n[0]<<24 | n[1]<<16 | n[2]<<8;
	uint32_t hw=(uint32_t)h[0]<<24 | h[1]<<16 | h[2]<<8;
	for(h+=2;*h && hw !=nw;hw=(hw|*++h)<<8);
	return *h ?(char *)h-2 : 0;
}
static char *fourbyte_strstr(const unsigned char *h,const unsigned char *n){
	uint32_t nw=(uint32_t)n[0]<<24 | n[1]<<16 | n[2]<<8 | n[3];
	uint32_t hw=(uint32_t)h[0]<<24 | h[1]<<16 | h[2]<<8 | h[3];
	for(h+=3;*h && hw !=nw;hw=hw<<8 | *++h);
	return *h ?(char *)h-3 : 0;
}
#define BITOP(a,b,op)((a)[(size_t)(b)/(8*sizeof *(a))] op(size_t)1<<((size_t)(b)%(8*sizeof *(a))))
static char *twoway_strstr(const unsigned char *h,const unsigned char *n){
	const unsigned char *z;
	size_t l,ip,jp,k,p,ms,p0,mem,mem0,byteset[32 / sizeof(size_t)]={0},shift[256];
	for(l=0;n[l] && h[l];l++)BITOP(byteset,n[l],|=),shift[n[l]]=l+1;
	if(n[l])return 0;
	ip=-1;jp=0;k=p=1;
	while(jp+k<l){
		if(n[ip+k]==n[jp+k]){
			if(k==p)jp +=p,k=1;
			else k++;
		}else if(n[ip+k] > n[jp+k])jp +=k,k=1,p=jp - ip;
		else ip=jp++,k=p=1;
	}
	ms=ip,p0=p,ip=-1,jp=0,k=p=1;
	while(jp+k<l){
		if(n[ip+k]==n[jp+k]){
			if(k==p)jp +=p,k=1;
			else k++;
		}else if(n[ip+k] < n[jp+k])jp +=k,k=1,p=jp - ip;
		else ip=jp++,k=p=1;
		
	}
	if(ip+1 > ms+1)ms=ip;
	else p=p0;
	if(memcmp(n,n+p,ms+1)){
		mem0=0;
		p=MAX(ms,l-ms-1)+ 1;
	}else mem0=l-p;
	mem=0,z=h;
	for(;;){
		if(z-h <(long)l){
			size_t grow=l | 63;
			const unsigned char *z2=memchr(z,0,grow);
			if(z2){
				z=z2;
				if(z-h <(long)l)return 0;
			}else z +=grow;
		}
		if(BITOP(byteset,h[l-1],&)){
			k=l-shift[h[l-1]];
			if(k){
				if(k < mem)k=mem;
				h +=k,mem=0;
				continue;
			}
		}else{
			h +=l,mem=0;
			continue;
		}
		for(k=MAX(ms+1,mem);n[k] && n[k]==h[k];k++);
		if(n[k]){
			h +=k-ms,mem=0;
			continue;
		}
		for(k=ms+1;k>mem && n[k-1]==h[k-1];k--);
		if(k <=mem)return(char *)h;
		h +=p,mem=mem0;
	}
}
char *strstr(const char *h,const char *n){
	if(!n[0])return(char *)h;
	h=strchr(h,*n);
	if(!h || !n[1])return(char *)h;
	if(!h[1])return 0;
	if(!n[2])return twobyte_strstr((void *)h,(void *)n);
	if(!h[2])return 0;
	if(!n[3])return threebyte_strstr((void *)h,(void *)n);
	if(!h[3])return 0;
	if(!n[4])return fourbyte_strstr((void *)h,(void *)n);
	return twoway_strstr((void *)h,(void *)n);
}
char *strrchr(const char *s, int c){
	return memrchr(s, c, strlen(s) + 1);
}