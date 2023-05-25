// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Allocator/Alloca.h"
#include "Container/HashSet.h"
#include "Container/RawStorage.h"
#include "Container/Vector.h"
#include "Diagnostic/Logger.h"
#include "HAL/PlatformMemory.h"
#include "IO/FormatHelpers.h"
#include "IO/FileSystem.h"
#include "VirtualFileSystem.h"
#include "Maths/RandomGenerator.h"

#include <algorithm>

namespace PPE {
namespace Test {
LOG_CATEGORY(, Test_VFS)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void Test_FSTrie() {
    const FFilename a = L"c:/windows/system32/notepad.exe";
    const FFilename b = L"c:/windows/temp/log.txt";
    const FFilename c = L"c:/windows/fonts/arial.ttf";
    const FFilename d = L"c:/windows/zz/bin.out";

    AssertRelease(a < b);
    AssertRelease(c < a);
    AssertRelease(c < b);
    AssertRelease(a < d);
    AssertRelease(b < d);
    AssertRelease(c < d);

    AssertRelease(a.Dirpath().IsAbsolute());
    AssertRelease(b.Dirpath().IsAbsolute());
    AssertRelease(c.Dirpath().IsAbsolute());
    AssertRelease(d.Dirpath().IsAbsolute());

    const FDirpath root = L"c:/windows";

    AssertRelease(a.IsRelativeTo(root));
    AssertRelease(b.IsRelativeTo(root));
    AssertRelease(c.IsRelativeTo(root));
    AssertRelease(d.IsRelativeTo(root));

    const FFilename ra = a.Relative(root);
    const FFilename rb = b.Relative(root);
    const FFilename rc = c.Relative(root);
    const FFilename rd = d.Relative(root);

    AssertRelease(ra < rb);
    AssertRelease(rc < ra);
    AssertRelease(rc < rb);
    AssertRelease(ra < rd);
    AssertRelease(rb < rd);
    AssertRelease(rc < rd);

    AssertRelease(ra.Dirpath().IsRelative());
    AssertRelease(rb.Dirpath().IsRelative());
    AssertRelease(rc.Dirpath().IsRelative());
    AssertRelease(rd.Dirpath().IsRelative());
}
//----------------------------------------------------------------------------
static void Test_VFSFrontend_() {
    const FDirpath& rootpath = L"Saved:/VFS";
    const FDirpath testpath = rootpath / FDirname(L"VFSComponent");
    const FFilename filename = testpath / FBasename(L"test.txt");

    STATIC_CONST_INTEGRAL(size_t, TestCount, 128 << 10);
    STATIC_CONST_INTEGRAL(size_t, TestSizeInBytes, TestCount * sizeof(wchar_t));
    STACKLOCAL_POD_ARRAY(wchar_t, writeData, TestCount);

    FRandomGenerator rand(42);
    rand.Randomize(writeData);

    VerifyRelease(VFS_CreateDirectory(testpath));
    VerifyRelease(VFS_DirectoryExists(testpath));

    VerifyRelease(VFS_WriteAll(filename, writeData.Cast<const u8>(), EAccessPolicy::Truncate_Binary));
    VerifyRelease(VFS_FileExists(filename));

    FFileStat stat;
    VerifyRelease(VFS_FileStats(&stat, filename));
    AssertRelease(stat.SizeInBytes == TestSizeInBytes);

    FRawStorage readData;
    VerifyRelease(VFS_ReadAll(&readData, filename, EAccessPolicy::Binary));
    AssertRelease(readData.SizeInBytes() == TestSizeInBytes);
    AssertRelease(FPlatformMemory::Memcmp(readData.data(), writeData.data(), writeData.SizeInBytes()) == 0);

    VerifyRelease(VFS_RemoveFile(filename));
    VerifyRelease(VFS_FileExists(filename) == false);

    HASHSET(FileSystem, FFilename) txtFiles;
    HASHSET(FileSystem, FFilename) bmpFiles;
    HASHSET(FileSystem, FFilename) generatedFiles;

    const FWStringView subDirs[] = {
        L"a",
        L"a/0",
        L"a/1",
        L"a/1/x/y/z/w",
        L"b/2",
        L"b/2/m",
        L"b/2/m/n",
        L"b/2/m/n/o",
        L"b/2/p/q",
        L"b/3",
        L"c/4",
    };

    HASHSET(FileSystem, FFilename) subDirFilenames[lengthof(subDirs)];

    forrange(s, 0, lengthof(subDirs)) {
        const FWStringView& subDir = subDirs[s];
        FDirpath subPath = testpath;
        subPath.Concat(subDir);
        VerifyRelease(VFS_CreateDirectory(subPath));

        const size_t n = rand.Next(6, 12);

        forrange(i, 0, n) {
            size_t len = rand.Next(3, 9);

            forrange(l, 0, len)
                writeData[l] = checked_cast<wchar_t>(rand.Next(L'a', L'z'));

            writeData[len++] = L'.';

            bool txt = false;

            if (rand.Next() & 1) {
                txt = true;
                writeData[len++] = L't';
                writeData[len++] = L'x';
                writeData[len++] = L't';
            }
            else {
                writeData[len++] = L'b';
                writeData[len++] = L'm';
                writeData[len++] = L'p';
            }

            const FFilename fname{ subPath, writeData.CutBeforeConst(len) };
            generatedFiles.emplace_AssertUnique(fname);
            subDirFilenames[s].emplace_AssertUnique(fname);

            if (txt)
                txtFiles.emplace_AssertUnique(fname);
            else
                bmpFiles.emplace_AssertUnique(fname);

            VerifyRelease(VFS_WriteAll(fname, writeData.Cast<const u8>(), EAccessPolicy::Truncate_Binary));
        }
    }

    {
        HASHSET(FileSystem, FFilename) enumeratedFiles;
        size_t numEnumeratedFiles = VFS_EnumerateFiles(testpath, true/* recursive */, [&enumeratedFiles](const FFilename& fname) {
            enumeratedFiles.emplace_AssertUnique(fname);
        });

        AssertRelease(numEnumeratedFiles == enumeratedFiles.size());
        AssertRelease(numEnumeratedFiles == generatedFiles.size());

        AssertRelease(enumeratedFiles == generatedFiles);
    }

    forrange(s, 0, lengthof(subDirs)) {
        const FWStringView& subDir = subDirs[s];
        FDirpath subPath = testpath;
        subPath.Concat(subDir);
        VerifyRelease(VFS_DirectoryExists(subPath));

        HASHSET(FileSystem, FFilename) enumeratedFiles;
        size_t numEnumeratedFiles = VFS_EnumerateFiles(subPath, false/* non recursive */, [&enumeratedFiles](const FFilename& fname) {
            enumeratedFiles.emplace_AssertUnique(fname);
        });
        AssertRelease(numEnumeratedFiles == enumeratedFiles.size());
        AssertRelease(numEnumeratedFiles == subDirFilenames[s].size());

        Assert(enumeratedFiles == subDirFilenames[s]);
    }

    {
        HASHSET(FileSystem, FFilename) txtGlobs;
        const size_t numGlobTxtFiles = VFS_GlobFiles(testpath, L"*.txt", true/* recursive */, [&txtGlobs](const FFilename& fname) {
            txtGlobs.emplace_AssertUnique(fname);
        });
        AssertRelease(numGlobTxtFiles == txtGlobs.size());
        AssertRelease(numGlobTxtFiles == txtFiles.size());

        AssertRelease(txtGlobs == txtFiles);

        HASHSET(FileSystem, FFilename) bmpGlobs;
        const size_t numGlobBmpFiles = VFS_GlobFiles(testpath, L"*.bmp", true/* recursive */, [&bmpGlobs](const FFilename& fname) {
            bmpGlobs.emplace_AssertUnique(fname);
        });
        AssertRelease(numGlobBmpFiles == bmpGlobs.size());
        AssertRelease(numGlobBmpFiles == bmpFiles.size());

        AssertRelease(bmpGlobs == bmpFiles);

        AssertRelease(generatedFiles.size() == numGlobTxtFiles + numGlobBmpFiles);
    }

    VerifyRelease(VFS_RemoveDirectory(testpath));
    VerifyRelease(VFS_DirectoryExists(testpath) == false);
}
//----------------------------------------------------------------------------
//
//  -- APIs to test --
//
//  * Readable :
//      bool DirectoryExists(const FDirpath& dirpath, EExistPolicy policy) = 0;
//      bool FileExists(const FFilename& filename, EExistPolicy policy) = 0;
//      bool FileStats(FFileStat* pstat, const FFilename& filename) = 0;
//      size_t EnumerateFiles(const FDirpath& dirpath, bool recursive, const TFunction<void(const FFilename&)>& foreach) = 0;
//      size_t GlobFiles(const FDirpath& dirpath, const FWStringView& pattern, bool recursive, const TFunction<void(const FFilename&)>& foreach) = 0;
//      UStreamReader OpenReadable(const FFilename& filename, EAccessPolicy policy) = 0;
//
//  * Writable :
//      bool CreateDirectory(const FDirpath& dirpath) = 0;
//      bool MoveFile(const FFilename& src, const FFilename& dst) = 0;
//      bool RemoveDirectory(const FDirpath& dirpath) = 0;
//      bool RemoveFile(const FFilename& filename) = 0;
//      UStreamWriter OpenWritable(const FFilename& filename, EAccessPolicy policy) = 0;
//
//  * ReadWritable :
//      UStreamReadWriter OpenReadWritable(const FFilename& filename, EAccessPolicy policy) = 0;
//
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Test_VFS() {
    PPE_DEBUG_NAMEDSCOPE("Test_VFS");

    Test_FSTrie();
    Test_VFSFrontend_();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Test
} //!namespace PPE
