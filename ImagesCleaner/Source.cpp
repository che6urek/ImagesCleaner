#include "Source.h"
#include "DuplicatesSearch.h"

#define FIRST_PATH_BUTTON_ID 1001
#define SECOND_PATH_BUTTON_ID 1002
#define FIND_DUPLICATES_BUTTON_ID 1003
#define EXTENSION_CHECKBOX_ID 1004
#define DELETE_BUTTON_ID 1005
#define MAX_LOAD_STRING 100

#define FONT_SIZE 12
#define MAX_DISPLAYED_PATH_LENGTH 60
#define MAX_PATH_LENGTH 2048

#define WINDOW_WIDTH 700
#define WINDOW_HEIGHT 250
#define WINDOW_X_MARGIN 200
#define WINDOW_Y_MARGIN 200

#define BUTTON_WIDTH 130
#define BUTTON_HEIGHT 30
#define X_MARGIN 15
#define Y_MARGIN 10

const wstring duplicatesFile = L"duplicates.txt";

HINSTANCE hInst;
WCHAR title[MAX_LOAD_STRING] = L"Images Cleaner";
WCHAR windowClass[MAX_LOAD_STRING] = L"ImagesCleanerClass";
RECT rect;

wstring firstSelectedPath;
wstring secondSelectedPath;

wstring reducedFirstPath;
wstring reducedSecondPath;

int duplicatesCount = -1;

vector<vector<path>*>* images = new vector<vector<path>*>;

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

void                PathClick(HWND hWnd, wstring& path, wstring& reducedPath);
LPWSTR              GetPath();
wstring             ReducePath(wstring path);
void                SaveToFile(const wstring& fileName, vector<vector<path>*>* files);
void                Reset(HWND hWnd);
void                DeleteFiles(vector<vector<path>*>* files);

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR           gdiplusToken;

    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    GdiplusShutdown(gdiplusToken);
    return int(msg.wParam);
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wcex.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = HBRUSH(COLOR_WINDOW);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = windowClass;

    return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;

    HWND hWnd = CreateWindowW(windowClass, title,
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        WINDOW_X_MARGIN, WINDOW_Y_MARGIN, WINDOW_WIDTH, WINDOW_HEIGHT,
        nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    GetClientRect(hWnd, &rect);
    switch (message)
    {
    case WM_COMMAND:
    {
        const int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case FIRST_PATH_BUTTON_ID:
            PathClick(hWnd, firstSelectedPath, reducedFirstPath);
            break;
        case SECOND_PATH_BUTTON_ID:
            PathClick(hWnd, secondSelectedPath, reducedSecondPath);
            break;
        case FIND_DUPLICATES_BUTTON_ID:
        {
            if (firstSelectedPath.empty() && secondSelectedPath.empty())
            {
                break;
            }
            duplicatesCount = DuplicatesSearch::GetDuplicates(firstSelectedPath, secondSelectedPath,
                images, IsDlgButtonChecked(hWnd, EXTENSION_CHECKBOX_ID));
            SaveToFile(duplicatesFile, images);
            InvalidateRect(hWnd, nullptr, TRUE);
            break;
        }
        case EXTENSION_CHECKBOX_ID:
        {
            const bool checked = IsDlgButtonChecked(hWnd, EXTENSION_CHECKBOX_ID);
            if (checked)
            {
                CheckDlgButton(hWnd, EXTENSION_CHECKBOX_ID, BST_UNCHECKED);
            }
            else
            {
                CheckDlgButton(hWnd, EXTENSION_CHECKBOX_ID, BST_CHECKED);
            }

            Reset(hWnd);
            break;
        }
        case DELETE_BUTTON_ID:
        {
            DeleteFiles(images);
            Reset(hWnd);
            break;
        }
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        SolidBrush brush(Color::Black);
        Font font(L"Arial", FONT_SIZE, FontStyleBold);
        HDC hdc = BeginPaint(hWnd, &ps);
        Graphics graphics(hdc);

        if (!reducedFirstPath.empty())
        {
            const PointF point(X_MARGIN * 2 + BUTTON_WIDTH, Y_MARGIN + FONT_SIZE / 2);
            graphics.DrawString(reducedFirstPath.c_str(), reducedFirstPath.length(), &font, point, &brush);
        }
        if (!reducedSecondPath.empty())
        {
            const PointF point(X_MARGIN * 2 + BUTTON_WIDTH, Y_MARGIN * 2 + BUTTON_HEIGHT + FONT_SIZE / 2);
            graphics.DrawString(reducedSecondPath.c_str(), reducedSecondPath.length(), &font, point, &brush);
        }

        const PointF point(X_MARGIN * 2 + BUTTON_WIDTH, Y_MARGIN * 4 + BUTTON_HEIGHT * 3 + FONT_SIZE / 2);
        const wstring duplicates = L"Duplicates: " + (duplicatesCount == -1 ? L"-" : std::to_wstring(duplicatesCount));
        graphics.DrawString(duplicates.c_str(), duplicates.length(), &font, point, &brush);

        EndPaint(hWnd, &ps);

        break;
    }

    case WM_CREATE:
    {
        CreateWindow(L"button", L"Get first path",
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            X_MARGIN, Y_MARGIN,
            BUTTON_WIDTH, BUTTON_HEIGHT,
            hWnd, HMENU(FIRST_PATH_BUTTON_ID), hInst, NULL);

        CreateWindow(L"button", L"Get second path",
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            X_MARGIN, BUTTON_HEIGHT + Y_MARGIN * 2,
            BUTTON_WIDTH, BUTTON_HEIGHT,
            hWnd, HMENU(SECOND_PATH_BUTTON_ID), hInst, NULL);

        CreateWindow(L"button", L"Check extension",
            WS_CHILD | WS_VISIBLE | BS_CHECKBOX,
            X_MARGIN, BUTTON_HEIGHT * 2 + Y_MARGIN * 3,
            BUTTON_WIDTH, BUTTON_HEIGHT,
            hWnd, HMENU(EXTENSION_CHECKBOX_ID), hInst, NULL);

        CreateWindow(L"button", L"Find duplicates",
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            X_MARGIN, BUTTON_HEIGHT * 3 + Y_MARGIN * 4,
            BUTTON_WIDTH, BUTTON_HEIGHT,
            hWnd, HMENU(FIND_DUPLICATES_BUTTON_ID), hInst, NULL);

        CreateWindow(L"button", L"Delete duplicates",
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            X_MARGIN, BUTTON_HEIGHT * 4 + Y_MARGIN * 5,
            BUTTON_WIDTH, BUTTON_HEIGHT,
            hWnd, HMENU(DELETE_BUTTON_ID), hInst, NULL);

        break;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void Reset(HWND hWnd)
{
    duplicatesCount = -1;
    delete images;
    images = new vector<vector<path>*>;
    InvalidateRect(hWnd, nullptr, TRUE);
}

void PathClick(HWND hWnd, wstring& path, wstring& reducedPath)
{
    LPWSTR selectedPath = GetPath();
    if (selectedPath != nullptr)
    {
        path = selectedPath;
        reducedPath = ReducePath(selectedPath);
        Reset(hWnd);
    }
}

LPWSTR GetPath()
{
    LPWSTR fileName = new WCHAR[MAX_PATH_LENGTH];
    fileName[0] = '\0';

    OPENFILENAME openFile;
    ZeroMemory(&openFile, sizeof openFile);
    openFile.lStructSize = sizeof openFile;
    openFile.hwndOwner = nullptr;
    openFile.nMaxFile = MAX_PATH_LENGTH;
    openFile.lpstrFile = fileName;
    openFile.lpstrFilter = L"Pictures (*.jpg; *.png; *.bmp; *.jpeg; *.icon)\0*.jpg; *.png; *.bmp; *.jpeg; *.icon\0All Files (*.*)\0*.*\0";
    openFile.Flags = OFN_NOVALIDATE;
    if (GetOpenFileName(&openFile))
    {
        return openFile.lpstrFile;
    }
    return nullptr;
}

wstring ReducePath(wstring path)
{
    if (path.length() > MAX_DISPLAYED_PATH_LENGTH)
    {
        const wstring file(path.begin() + path.find_last_of(L'\\') + 1, path.end());
        if (file.length() > MAX_DISPLAYED_PATH_LENGTH - 4)
        {
            return file.substr(0, MAX_DISPLAYED_PATH_LENGTH / 2 - 3).append(L"...")
                .append(file.substr(file.length() - MAX_DISPLAYED_PATH_LENGTH / 2 + 3, file.length()));
        }
        return path.substr(0, MAX_DISPLAYED_PATH_LENGTH - 4 - file.length()).append(L"...\\").append(file);

    }
    return path;
}

void SaveToFile(const wstring& fileName, vector<vector<path>*>* files)
{
    std::ofstream fout(fileName);

    int i = 0;
    fout << "Duplicates" << endl << endl;
    for (auto& file : *files)
    {
        if (file->size() < 2)
        {
            continue;
        }
        fout << ++i << ":" << endl;
        for (auto& iterator2 : *file)
        {
            fout << iterator2.string() << endl;
        }
        fout << endl;
        fout << endl;
    }
    fout.close();
}

void DeleteFiles(vector<vector<path>*>* files)
{
    for (auto& file : *files)
    {
        if (file->size() < 2)
        {
            continue;
        }
        for (auto iterator = file->begin() + 1; iterator != file->end(); ++iterator)
        {
            filesystem::remove(*iterator);
        }
    }
}