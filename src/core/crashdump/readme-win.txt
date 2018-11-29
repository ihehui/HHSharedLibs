https://blog.csdn.net/bytxl/article/details/46722881

*PDB简介*
跟踪提供程序（例如应用程序或驱动程序）的程序数据库 (PDB) 符号文件包含用于对跟踪消息设置格式的指令，以便可以按照用户可读的形式显示这些消息。
跟踪消息格式设置指令属于跟踪提供程序源代码的一部分。 WPP 预处理器从代码中提取这些指令并将其添加到跟踪提供程序的 PDB 符号文件。
当你编译调试（已检验）版本的跟踪提供程序时，编译器生成 PDB 文件。默认情况下，当你使用 BinPlace 生成跟踪提供程序时，生成过程将创建 PDB 文件。
WDK 中的 跟踪使用器、 TraceView 和 Tracefmt 可以从 PDB 文件或 TMF 文件中直接提取跟踪消息格式设置信息。其他的则要求 TMF 文件。 Tracepdb 将 PDB 文件作为输入，提取格式设置信息，然后创建 TMF 文件作为输出。
其他跟踪使用器不使用 PDB 文件或 TMF 文件，例如包含在 Windows 中的工具 Tracerpt。相反，它们使用托管对象格式 (MOF) 文件中的信息为跟踪事件设置格式。这些工具无法对跟踪消息设置格式。
*概述：*
1. 注册生成dump文件的函数。
2. 当程序收到没有捕获的异常时，调用上述函数，生成dump文件。
3. 利用Windbg结合编译程序时生成的pdb和代码来分析dump文件，定位问题。

*如下代码生成dump文件（转）：*

#include 
#include 
#include 
#pragma comment(lib, "dbghelp.lib") 

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

inline BOOL CALLBACK MiniDumpCallback(PVOID pParam, 
const PMINIDUMP_CALLBACK_INPUT pInput, 
PMINIDUMP_CALLBACK_OUTPUT pOutput) 
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
mdei.ThreadId = GetCurrentThreadId(); 
mdei.ExceptionPointers = pep; 
mdei.ClientPointers = FALSE; 

MINIDUMP_CALLBACK_INFORMATION mci; 
mci.CallbackRoutine = (MINIDUMP_CALLBACK_ROUTINE)MiniDumpCallback; 
mci.CallbackParam = NULL; 

::MiniDumpWriteDump(::GetCurrentProcess(), ::GetCurrentProcessId(), hFile, MiniDumpNormal, (pep != NULL) ? &mdei : NULL, NULL, &mci); 

CloseHandle(hFile); 
} 
} 


LONG __stdcall MyUnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo) 
{
CreateMiniDump(pExceptionInfo, "core.dmp"); 

return EXCEPTION_EXECUTE_HANDLER; 
} 


*确保程序开始执行如下代码，然后程序崩溃时会调用上面代码创建dump文件：*
SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);

*分析生成的dump文件需要如下：*
* 编译程序时生成的PDB，如果是release版本： 
* 需要禁用优化 - VS - Project Property - C/C++ Optimization - Release - Optimization - Disabled
* 启用生成调试信息 - VS - Project Property - Linker - Debugging - Generate Debug Info - Yes.
* 代码

Qt工程的MinGW32设置
win32-g++ {
    #加入调试信息
    QMAKE_CFLAGS_RELEASE += -g
    QMAKE_CXXFLAGS_RELEASE += -g
    #禁止优化
    QMAKE_CFLAGS_RELEASE -= -O2
    QMAKE_CXXFLAGS_RELEASE -= -O2
    #release在最后link时默认有"-s”参数，表示"Omit all symbol information from the output file"，因此要去掉该参数
    QMAKE_LFLAGS_RELEASE =
    ####使用cv2pdb的工具从最后编译出来的exe、dll中提取出pdb
}




*分析dump文件步骤如下：*
1. 运行Windbg。
2. 指定PDB文件路径： File - Symbol File Path。多个路径用分号分隔。
3. 指定代码路径：File - Source File Path
4. 载入dump文件。
5. Windbg命令行输入: !analyze -v
6. 等待结果 - 函数调用堆栈，程序崩溃代码。busy状态表示正在生成结果。

*注释：*
* 没有代码，只有PDB，也可以显示函数调用堆栈，但是不会定位到具体代码。
* Windbg中，配置 Symbol File Path: srv*c:\symbols*http://msdl.microsoft.com/download/symbols，可以解决本地找不到symbol问题。定位一般问题，不是必须。c:\symbols为本地缓存PDB目录。只会同步用到的symbol。
* Windbg的附带工具symchk可以用来下载指定dll的pdb文件： 
* 下载特定dll的pdb文件：symchk /r c:\windows\system32\secur32.dll /s SRV*c:\symbols\*http://msdl.microsoft.com/download/symbols
* 下载特定目录下的dll的pdb文件：symchk /r c:\windows\system32 /s SRV*c:\symbols\*http://msdl.microsoft.com/download/symbols
* 如果dump文件被拷贝到编译程序的机器上，无需指定代码路径，只需指定pdb文件，可自动定位代码。 
