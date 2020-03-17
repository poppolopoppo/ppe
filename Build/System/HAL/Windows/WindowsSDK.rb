# frozen_string_literal: true

require_once '../../Common.rb'
require_once '../../Utils/Prerequisite.rb'

module Build

    import_glob(:WindowsSDK_10_Glob, 'C:/Program Files (x86)/Windows Kits/10/Lib/10.*').validate_FileExist!
    import_glob(:WindowsSDK_8_1_Glob, 'C:/Program Files (x86)/Windows Kits/8.1/Lib/8.*').validate_FileExist!

    const_memoize(self, :WindowsSDK_10_Info) do
        sdkLib = Build.WindowsSDK_10_Glob()
        return if sdkLib.empty?
        sdkLib = sdkLib.first
        sdkVer = File.basename(sdkLib)
        sdkPath = File.expand_path(File.join(sdkLib, '..', '..'))
        Log.verbose('Windows: found WindowsSDK %s in \'%s\'', sdkVer, sdkPath)
        [ sdkPath, sdkVer ]
    end

    make_facet(:WindowsSDK_10_Includes) do
        sdkPath, sdkVer = *Build.WindowsSDK_10_Info
        export!('WindowsSDKPath', sdkPath)
        includePaths <<
            File.join(sdkPath, 'Include', sdkVer, 'ucrt') <<
            File.join(sdkPath, 'Include', sdkVer, 'um') <<
            File.join(sdkPath, 'Include', sdkVer, 'shared')
        defines <<
            "STRICT"                        << # https://msdn.microsoft.com/en-us/library/windows/desktop/aa383681(v=vs.85).aspx
            "NOMINMAX"                      << # https://support.microsoft.com/en-us/kb/143208
            "VC_EXTRALEAN"                  << # https://support.microsoft.com/en-us/kb/166474
            "WIN32_LEAN_AND_MEAN"           << # https://support.microsoft.com/en-us/kb/166474
            "_NO_W32_PSEUDO_MODIFIERS"      << # Prevent windows from #defining IN or OUT (undocumented)
            "DBGHELP_TRANSLATE_TCHAR"       << # https://msdn.microsoft.com/en-us/library/windows/desktop/ms679294(v=vs.85).aspx
            "_UNICODE"                      << # https://msdn.microsoft.com/fr-fr/library/dybsewaf.aspx
            "UNICODE"                       << #
            "_HAS_EXCEPTIONS=0"             << # Disable STL exceptions
            "OEMRESOURCE"                      # https://docs.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-setsystemcursor
    end
    make_facet(:WindowsSDK_10_X86) do
        sdkPath, sdkVer = *Build.WindowsSDK_10_Info
        self << Build.WindowsSDK_10_Includes
        libraryPaths <<
            File.join(sdkPath, 'Lib', sdkVer, 'ucrt', 'x86') <<
            File.join(sdkPath, 'Lib', sdkVer, 'um', 'x86')
    end
    make_facet(:WindowsSDK_10_X64) do
        sdkPath, sdkVer = *Build.WindowsSDK_10_Info
        self << Build.WindowsSDK_10_Includes
        libraryPaths <<
            File.join(sdkPath, 'Lib', sdkVer, 'ucrt', 'x64') <<
            File.join(sdkPath, 'Lib', sdkVer, 'um', 'x64')
    end

    make_prerequisite(:WindowsSDK_10_RC_exe) do
        sdkPath, sdkVer = *Build.WindowsSDK_10_Info
        need_file!(File.join(sdkPath, 'Bin', sdkVer, 'x64', 'RC.exe'))
    end

    def WindowsSDK_X86() Build.WindowsSDK_10_X86 end
    def WindowsSDK_X64() Build.WindowsSDK_10_X64 end
    def WindowsSDK_RC_exe() Build.WindowsSDK_10_RC_exe end

end #~ Build