#pragma once
#include <objidl.h>
#include <gdiplus.h>
#include <string>
#include <set>
#include <vector>
#include <filesystem>
#include <vector>
#include <set>
#pragma comment (lib, "Gdiplus.lib")

using namespace Gdiplus;
using namespace std;
using namespace filesystem;

class DuplicatesSearch
{
public:
    static int GetDuplicates(const path& path1, const path& path2,
        vector<vector<path>*>* files, bool checkExtensions);
private:
    static const int segmentSize = 32;
    static inline const set<string> extensions{ ".jpg", ".png", ".bmp", ".jpeg", ".icon" };

    static bool IsSubDir(path path, filesystem::path root);
    static string ToLower(string str);
    static bool CheckExtension(const path& file);
    static void GetFiles(const path& path, vector<vector<filesystem::path>*>* files);
    static bool CompareFiles(const path& file1, const path& file2, bool checkExtensions);
    static int FindDuplicates(vector<vector<path>*>* files, bool checkExtensions);
    static int GetDifference(Image* firstImage, Image* secondImage);
    static Image* Resize(Image* originalImage, int newWidth, int newHeight);
};

