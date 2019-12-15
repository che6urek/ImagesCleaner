#include "DuplicatesSearch.h"

int DuplicatesSearch::GetDuplicates(const path& path1, const path& path2,
    vector<vector<path>*>* files, bool checkExtensions)
{
    if (IsSubDir(path1, path2))
    {
        GetFiles(path2, files);
    }
    else
    {
        if (IsSubDir(path2, path1))
        {
            GetFiles(path1, files);
        }
        else
        {
            GetFiles(path1, files);
            GetFiles(path2, files);
        }
    }
    return FindDuplicates(files, checkExtensions);
}

bool DuplicatesSearch::IsSubDir(path path, filesystem::path root)
{
    if (path.empty() || root.empty())
    {
        return false;
    }

    if (!path.compare(root))
    {
        return true;
    }

    int trigger = 0;
    if (!is_directory(path))
    {
        path.remove_filename();
        trigger++;
    }
    if (!is_directory(root))
    {
        root.remove_filename();
        trigger++;
    }
    if (trigger == 2)
    {
        return false;
    }

    return !path.string().find(root.string());
}

string DuplicatesSearch::ToLower(string str)
{
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::tolower(c); });
    return str;
}

bool DuplicatesSearch::CheckExtension(const path& file)
{
    const string extension = ToLower(file.extension().string());
    return extensions.count(extension);
}

void DuplicatesSearch::GetFiles(const path& path, vector<vector<filesystem::path>*>* files)
{
    if (path.empty())
    {
        return;
    }
    if (!is_directory(path))
    {
        files->push_back(new vector<filesystem::path>);
        files->back()->push_back(path);
    }
    else
    {
        for (const auto& p : recursive_directory_iterator(path))
        {
            if (!is_directory(p) && CheckExtension(p.path()))
            {
                files->push_back(new vector<filesystem::path>);
                files->back()->push_back(p.path());
            }
        }
    }
}

bool DuplicatesSearch::CompareFiles(const path& file1, const path& file2, bool checkExtensions)
{
    if ((!checkExtensions || file1.extension() == file2.extension()) && file_size(file1) == file_size(file2))
    {
        const auto image1 = new Image(file1.c_str());
        const auto image2 = new Image(file2.c_str());
        const bool result = !GetDifference(image1, image2);
        delete image1;
        delete image2;
        return result;
    }
    return false;
}

int DuplicatesSearch::FindDuplicates(vector<vector<path>*>* files, bool checkExtensions)
{
    int duplicates = 0;
    if (files->size() > 1)
    {
        for (size_t i = 0; i < files->size(); i++)
        {
            for (auto iterator = files->begin() + i + 1; iterator != files->end(); ++iterator)
            {
                if (files->at(i)->front() == (*iterator)->front())
                {
                    iterator = files->erase(iterator);
                    --iterator;
                    continue;
                }
                if (CompareFiles(files->at(i)->front(), (*iterator)->front(), checkExtensions))
                {
                    files->at(i)->push_back((*iterator)->front());
                    iterator = files->erase(iterator);
                    --iterator;
                    duplicates++;
                }
            }
        }
    }

    return duplicates;
}

int DuplicatesSearch::GetDifference(Image* firstImage, Image* secondImage)
{
    auto firstBitmap = dynamic_cast<Bitmap*>(Resize(firstImage, segmentSize, segmentSize));
    auto secondBitmap = dynamic_cast<Bitmap*>(Resize(secondImage, segmentSize, segmentSize));


    int differences[segmentSize][segmentSize];
    int firstGray[segmentSize][segmentSize];
    int secondGray[segmentSize][segmentSize];

    Color pixelColor;

    for (auto x = 0; x < segmentSize; x++)
    {
        for (auto y = 0; y < segmentSize; y++)
        {
            firstBitmap->GetPixel(x, y, &pixelColor);
            firstGray[x][y] = abs(int(pixelColor.GetValue()));
        }
    }

    for (auto x = 0; x < segmentSize; x++)
    {
        for (auto y = 0; y < segmentSize; y++)
        {
            secondBitmap->GetPixel(x, y, &pixelColor);
            secondGray[x][y] = abs(int(pixelColor.GetValue()));
        }
    }

    delete firstBitmap;
    delete secondBitmap;

    for (auto x = 0; x < segmentSize; x++)
    {
        for (auto y = 0; y < segmentSize; y++)
        {
            differences[x][y] = int(abs(firstGray[x][y] - secondGray[x][y]));
        }
    }

    int differentPixels = 0;

    for (auto i = 0; i < segmentSize; i++)
    {
        for (auto j = 0; j < segmentSize; j++)
        {
            if (differences[i][j] > 0)
            {
                differentPixels++;
            }
        }
    }

    return differentPixels;
}

Image* DuplicatesSearch::Resize(Image* originalImage, int newWidth, int newHeight)
{
    Image* smallVersion = new Bitmap(newWidth, newHeight);

    Graphics graphics(smallVersion);
    graphics.SetSmoothingMode(SmoothingModeHighQuality);
    graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);
    graphics.SetPixelOffsetMode(PixelOffsetModeHighQuality);
    graphics.DrawImage(originalImage, 0, 0, newWidth, newHeight);

    return smallVersion;
}