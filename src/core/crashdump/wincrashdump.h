/*
////////////////////
//*确保程序开始执行如下代码，然后程序崩溃时会调用上面代码创建dump文件：*
//SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);

//*分析生成的dump文件需要如下：*
//* 编译程序时生成的PDB，如果是release版本：
//* 需要禁用优化 - VS - Project Property - C/C++ Optimization - Release - Optimization - Disabled
//* 启用生成调试信息 - VS - Project Property - Linker - Debugging - Generate Debug Info - Yes.
//* 代码

//*分析dump文件步骤如下：*
//1. 运行Windbg。
//2. 指定PDB文件路径： File - Symbol File Path。多个路径用分号分隔。
//3. 指定代码路径：File - Source File Path
//4. 载入dump文件。
//5. Windbg命令行输入: !analyze -v
//6. 等待结果 - 函数调用堆栈，程序崩溃代码。busy状态表示正在生成结果。

//*注释：*
//* 没有代码，只有PDB，也可以显示函数调用堆栈，但是不会定位到具体代码。
//* Windbg中，配置 Symbol File Path: srv*c:\symbols*http://msdl.microsoft.com/download/symbols，可以解决本地找不到symbol问题。定位一般问题，不是必须。c:\symbols为本地缓存PDB目录。只会同步用到的symbol。
//* Windbg的附带工具symchk可以用来下载指定dll的pdb文件：
//* 下载特定dll的pdb文件：symchk /r c:\windows\system32\secur32.dll /s SRV*c:\symbols\*http://msdl.microsoft.com/download/symbols
//* 下载特定目录下的dll的pdb文件：symchk /r c:\windows\system32 /s SRV*c:\symbols\*http://msdl.microsoft.com/download/symbols
//* 如果dump文件被拷贝到编译程序的机器上，无需指定代码路径，只需指定pdb文件，可自动定位代码。
////////////////////
*/

#ifndef WINCRASHDUMP_H
#define WINCRASHDUMP_H


#include <Windows.h>
#include <stdio.h>
#include <DbgHelp.h>
#pragma comment(lib, "dbghelp.lib")


/////// Usage:
/// SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);
//////



inline BOOL IsDataSectionNeeded(const WCHAR* pModuleName)
{
    if(pModuleName == NULL)
    {
        return FALSE;
    }

    WCHAR szFileName[_MAX_FNAME] = L"";
    _wsplitpath(pModuleName, NULL, NULL, szFileName, NULL);

    if(wcsicmp(szFileName, L"ntdll") == 0)
        return TRUE;

    return FALSE;
}

inline BOOL CALLBACK MiniDumpCallback(PVOID                            pParam,
                                      const PMINIDUMP_CALLBACK_INPUT   pInput,
                                      PMINIDUMP_CALLBACK_OUTPUT        pOutput)
{
    if(pInput == 0 || pOutput == 0)
        return FALSE;

    switch(pInput->CallbackType)
    {
    case ModuleCallback:
        if(pOutput->ModuleWriteFlags & ModuleWriteDataSeg)
            if(!IsDataSectionNeeded(pInput->Module.FullPath))
                pOutput->ModuleWriteFlags &= (~ModuleWriteDataSeg);
    case IncludeModuleCallback:
    case IncludeThreadCallback:
    case ThreadCallback:
    case ThreadExCallback:
        return TRUE;
    default:;
    }

    return FALSE;
}

inline void CreateMiniDump(PEXCEPTION_POINTERS pep, LPCTSTR strFileName)
{
    HANDLE hFile = CreateFile(strFileName, GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if(hFile != INVALID_HANDLE_VALUE)
    {
      MINIDUMP_EXCEPTION_INFORMATION mdei;
      mdei.ThreadId           = GetCurrentThreadId();
      mdei.ExceptionPointers  = pep;
      mdei.ClientPointers     = FALSE;

      MINIDUMP_CALLBACK_INFORMATION mci;
      mci.CallbackRoutine     = (MINIDUMP_CALLBACK_ROUTINE)MiniDumpCallback;
      mci.CallbackParam       = NULL;

      ::MiniDumpWriteDump(::GetCurrentProcess(), ::GetCurrentProcessId(), hFile, MiniDumpNormal, (pep != NULL) ? &mdei : NULL, NULL, &mci);

      CloseHandle(hFile);
    }
}


LONG __stdcall MyUnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo)
{
  CreateMiniDump(pExceptionInfo, L"core.dmp");

  return EXCEPTION_EXECUTE_HANDLER;
}


#endif // WINCRASHDUMP_H
