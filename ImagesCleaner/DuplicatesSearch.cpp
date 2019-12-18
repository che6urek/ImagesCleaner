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
    try 
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
                try
                {
                    if (!is_directory(p) && CheckExtension(p.path()))
                    {
                        files->push_back(new vector<filesystem::path>);
                        files->back()->push_back(p.path());
                    }
                }
                catch (filesystem_error & e)
                {

                }
            }
        }
    }
    catch (filesystem_error & e)
    {

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

//Image* DuplicatesSearch::GetGrayVersion(Image* image)
//{
//    Image* newImage = new Bitmap(image->GetWidth(), image->GetHeight());
//    Graphics grayGraphics(newImage);
//    ColorMatrix colorMatrix =
//    {
//        0.3f, 0.3f, 0.3f, 0.0f, 0.0f,
//        0.59f, 0.59f, 0.59f, 0.0f, 0.0f,
//        0.11f, 0.11f, 0.11f, 0.0f, 0.0f,
//        0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
//        0.0f, 0.0f, 0.0f, 0.0f, 1.0f
//    };
//
//    ImageAttributes imageAttributes;
//    imageAttributes.SetColorMatrix(&colorMatrix, ColorMatrixFlagsDefault, ColorAdjustTypeBitmap);
//    grayGraphics.DrawImage(image, Rect(0, 0, image->GetWidth(), image->GetHeight()), 0, 0,
//        image->GetWidth(), image->GetHeight(), UnitPixel, &imageAttributes);
//
//    return newImage;
//}

int DuplicatesSearch::GetDifference(Image* firstImage, Image* secondImage)
{
    //Image* firstImageGray = GetGrayVersion(firstImage);
    //Image* secondImageGray = GetGrayVersion(secondImage);

    //auto firstBitmap = dynamic_cast<Bitmap*>(Resize(firstImageGray, segmentSize, segmentSize));
    //auto secondBitmap = dynamic_cast<Bitmap*>(Resize(secondImageGray, segmentSize, segmentSize));

    auto firstBitmap = dynamic_cast<Bitmap*>(Resize(firstImage, segmentSize, segmentSize));
    auto secondBitmap = dynamic_cast<Bitmap*>(Resize(secondImage, segmentSize, segmentSize));

    Color pixelColor1, pixelColor2;
    int differentPixels = 0;

    for (auto x = 0; x < segmentSize; x++)
    {
        for (auto y = 0; y < segmentSize; y++)
        {
            firstBitmap->GetPixel(x, y, &pixelColor1);
            secondBitmap->GetPixel(x, y, &pixelColor2);
            if(abs(int(pixelColor1.GetValue()) - int(pixelColor2.GetValue())) > 0)
            //if (abs(::byte(pixelColor1.GetBlue()) - ::byte(pixelColor2.GetBlue())) > precision)
            {
                differentPixels++;
            }
        }
    }

    delete firstBitmap;
    delete secondBitmap;

    //delete firstImageGray;
    //delete secondImageGray;

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