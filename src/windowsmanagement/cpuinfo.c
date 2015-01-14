//http://www.cnblogs.com/zyl910/archive/2012/08/06/2625347.html

#include <stdio.h>

// intrinsics
#if defined(__GNUC__)    // GCC
#include <cpuid.h>
#elif defined(_MSC_VER)    // MSVC
    #if _MSC_VER >=1400    // VC2005
#include <intrin.h>
    #endif    // #if _MSC_VER >=1400
#else
#error Only supports MSVC or GCC.
#endif    // #if defined(__GNUC__)



#if __cplusplus
extern "C" {
#endif


void getcpuidex(unsigned int CPUInfo[4], unsigned int InfoType, unsigned int ECXValue)
{
#if defined(__GNUC__)    // GCC
    __cpuid_count(InfoType, ECXValue, CPUInfo[0],CPUInfo[1],CPUInfo[2],CPUInfo[3]);
#elif defined(_MSC_VER)    // MSVC
    #if defined(_WIN64) || _MSC_VER>=1600    // 64位下不支持内联汇编. 1600: VS2010, 据说VC2008 SP1之后才支持__cpuidex.
        __cpuidex((int*)(void*)CPUInfo, (int)InfoType, (int)ECXValue);
    #else
        if (NULL==CPUInfo)    return;
        _asm{
            // load. 读取参数到寄存器.
            mov edi, CPUInfo;    // 准备用edi寻址CPUInfo
            mov eax, InfoType;
            mov ecx, ECXValue;
            // CPUID
            cpuid;
            // save. 将寄存器保存到CPUInfo
            mov    [edi], eax;
            mov    [edi+4], ebx;
            mov    [edi+8], ecx;
            mov    [edi+12], edx;
        }
    #endif
#endif    // #if defined(__GNUC__)
}

void getcpuid(unsigned int CPUInfo[4], unsigned int InfoType)
{
#if defined(__GNUC__)    // GCC
    __cpuid(InfoType, CPUInfo[0],CPUInfo[1],CPUInfo[2],CPUInfo[3]);
#elif defined(_MSC_VER)    // MSVC
    #if _MSC_VER>=1400    // VC2005才支持__cpuid
        __cpuid((int*)(void*)CPUInfo, (int)InfoType);
    #else
        getcpuidex(CPUInfo, InfoType, 0);
    #endif
#endif    // #if defined(__GNUC__)
}

// 取得CPU厂商（Vendor）.
//
// result: 成功时返回字符串的长度（一般为12）。失败时返回0.
// pvendor: 接收厂商信息的字符串缓冲区。至少为13字节.
int cpu_getvendor(char* pvendor)
{
    unsigned int dwBuf[4];
    if (NULL==pvendor)    return 0;
    // Function 0: Vendor-ID and Largest Standard Function
    getcpuid(dwBuf, 0);
    // save. 保存到pvendor
    *(unsigned int *)&pvendor[0] = dwBuf[1];    // ebx: 前四个字符.
    *(unsigned int *)&pvendor[4] = dwBuf[3];    // edx: 中间四个字符.
    *(unsigned int *)&pvendor[8] = dwBuf[2];    // ecx: 最后四个字符.
    pvendor[12] = '\0';
    return 12;
}

// 取得CPU商标（Brand）.
//
// result: 成功时返回字符串的长度（一般为48）。失败时返回0.
// pbrand: 接收商标信息的字符串缓冲区。至少为49字节.
int cpu_getName(char* pbrand)
{
    unsigned int dwBuf[4];
    if (NULL==pbrand)    return 0;
    // Function 0x80000000: Largest Extended Function Number
    getcpuid(dwBuf, 0x80000000U);
    if (dwBuf[0] < 0x80000004U)    return 0;
    // Function 80000002h,80000003h,80000004h: Processor Brand String
    getcpuid((unsigned int *)&pbrand[0], 0x80000002U);    // 前16个字符.
    getcpuid((unsigned int *)&pbrand[16], 0x80000003U);    // 中间16个字符.
    getcpuid((unsigned int *)&pbrand[32], 0x80000004U);    // 最后16个字符.
    pbrand[48] = '\0';
    return 48;
}

//获取CPU序列号(PSN)
void cpu_getPSN(char *PSN)
{
    unsigned int varEAX, varEBX, varECX, varEDX;
    char str[256];

    //%eax=1 gives most significant 32 bits in eax
#if defined(__GNUC__)    // GCC
    __asm__ __volatile__ ("cpuid"   : "=a" (varEAX), "=b" (varEBX), "=c" (varECX), "=d" (varEDX) : "a" (1));
#elif defined(_MSC_VER)    // MSVC
    __asm
    {
        mov   eax,01h
        xor   edx,edx
        cpuid
        mov   varEDX,edx
        mov   varEAX,eax
    }
#endif    // #if defined(__GNUC__)
    sprintf(str, "%08X%08X", varEDX, varEAX); //i.e. XXXXXXXX
    sprintf(PSN, "%s", str);

    //sprintf(str, "%08X", varEDX); //i.e. XXXX-XXXX-xxxx-xxxx-xxxx-xxxx
    //sprintf(PSN, "%C%C%C%C-%C%C%C%C", str[0], str[1], str[2], str[3], str[4], str[5], str[6], str[7]);
    //sprintf(str, "%08X", varEAX); //i.e. XXXX-XXXX-xxxx-xxxx-xxxx-xxxx
    //sprintf(PSN, "%s-%C%C%C%C-%C%C%C%C", PSN, str[0], str[1], str[2], str[3], str[4], str[5], str[6], str[7]);


    //%eax=3 gives least significant 64 bits in edx and ecx [if PN is enabled]
#if defined(__GNUC__)    // GCC
    __asm__ __volatile__ ("cpuid"   : "=a" (varEAX), "=b" (varEBX), "=c" (varECX), "=d" (varEDX) : "a" (3));
#elif defined(_MSC_VER)    // MSVC
    __asm
    {
        mov   eax,03h
        xor   ecx,ecx
        xor   edx,edx
        cpuid
        mov   varEDX,edx
        mov   varECX,ecx
    }
#endif    // #if defined(__GNUC__)
    sprintf(str, "%08X%08X", varEDX, varECX); //i.e. XXXXXXXX
    if(strcmp("0000000000000000", str)){
        //如果不全为0则需要复制拷贝
        //strcat(PSN, str);
        sprintf(PSN, "%s%s", PSN, str);
    }

    //sprintf(str, "%08X", varEDX); //i.e. xxxx-xxxx-xxxx-xxxx-XXXX-XXXX
    //sprintf(PSN, "%s-%C%C%C%C-%C%C%C%C", PSN, str[0], str[1], str[2], str[3], str[4], str[5], str[6], str[7]);
    //sprintf(str, "%08X", varECX); //i.e. xxxx-xxxx-xxxx-xxxx-XXXX-XXXX
    //sprintf(PSN, "%s-%C%C%C%C-%C%C%C%C", PSN, str[0], str[1], str[2], str[3], str[4], str[5], str[6], str[7]);


}



/*
//Usage:
int main(int argc, char* argv[])
{

    char szBuf[64];
    //unsigned int dwBuf[4];

    // test
    //getcpuidex(dwBuf, 0,0);
    //getcpuid(dwBuf, 0);
    //printf("%.8X\t%.8X\t%.8X\t%.8X\n", dwBuf[0],dwBuf[1],dwBuf[2],dwBuf[3]);

    cpu_getvendor(szBuf);
    printf("CPU Vendor:%s\n", szBuf);

    cpu_getName(szBuf);
    printf("CPU Name:%s\n", szBuf);

    cpu_getPSN(szBuf);
    printf("CPU SN:%s\n", szBuf);

    getchar();

    return 0;
}
*/


#if defined __cplusplus
}; /* End "C" */
#endif /* __cplusplus */
