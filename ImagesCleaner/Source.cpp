#include "Source.h"
#include "DuplicatesSearch.h"

const auto firstPathButtonId = 1001;
const auto secondPathButtonId = 1002;
const auto findDuplicatesButtonId = 1003;
const auto extensionCheckBoxId = 1004;
const auto deleteButtonId = 1005;
const auto maxLoadString = 100;

const auto fontSize = 12;
const auto maxDisplayedPathLength = 60;
const auto maxPathLength = 2048;

const auto windowWidth = 700;
const auto windowHeight = 250;
const auto windowXMargin = 200;
const auto windowYMargin = 200;

const auto buttonWidth = 130;
const auto buttonHeight = 30;
const auto xMargin = 15;
const auto yMargin = 10;

const auto segmentSize = 32;

const wstring duplicatesFile = L"duplicates.txt";

HINSTANCE hInst;
WCHAR title[maxLoadString] = L"Images Cleaner";
WCHAR windowClass[maxLoadString] = L"ImagesCleanerClass";
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

void				PathClick(HWND hWnd, wstring& path, wstring& reducedPath);
LPWSTR				GetPath();
wstring				ReducePath(wstring path);
void				SaveToFile(const wstring& fileName, vector<vector<path>*>* files);
void				Reset(HWND hWnd);
void				DeleteFiles(vector<vector<path>*>* files);

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
		windowXMargin, windowYMargin, windowWidth, windowHeight,
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
		case firstPathButtonId:
			PathClick(hWnd, firstSelectedPath, reducedFirstPath);
			break;
		case secondPathButtonId:
			PathClick(hWnd, secondSelectedPath, reducedSecondPath);
			break;
		case findDuplicatesButtonId:
		{
			if (firstSelectedPath.empty() && secondSelectedPath.empty())
			{
				break;
			}
			duplicatesCount = DuplicatesSearch::GetDuplicates(firstSelectedPath, secondSelectedPath,
				images, IsDlgButtonChecked(hWnd, extensionCheckBoxId));
			SaveToFile(duplicatesFile, images);
			InvalidateRect(hWnd, nullptr, TRUE);
			break;
		}
		case extensionCheckBoxId:
		{
			const bool checked = IsDlgButtonChecked(hWnd, extensionCheckBoxId);
			if (checked)
			{
				CheckDlgButton(hWnd, extensionCheckBoxId, BST_UNCHECKED);
			}
			else
			{
				CheckDlgButton(hWnd, extensionCheckBoxId, BST_CHECKED);
			}

			Reset(hWnd);
			break;
		}
		case deleteButtonId:
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
		Font font(L"Arial", fontSize, FontStyleBold);
		HDC hdc = BeginPaint(hWnd, &ps);
		Graphics graphics(hdc);

		if (!reducedFirstPath.empty())
		{
			const PointF point(xMargin * 2 + buttonWidth, yMargin + fontSize / 2);
			graphics.DrawString(reducedFirstPath.c_str(), reducedFirstPath.length(), &font, point, &brush);
		}
		if (!reducedSecondPath.empty())
		{
			const PointF point(xMargin * 2 + buttonWidth, yMargin * 2 + buttonHeight + fontSize / 2);
			graphics.DrawString(reducedSecondPath.c_str(), reducedSecondPath.length(), &font, point, &brush);
		}

		const PointF point(xMargin * 2 + buttonWidth, yMargin * 4 + buttonHeight * 3 + fontSize / 2);
		const wstring duplicates = L"Duplicates: " + (duplicatesCount == -1 ? L"-" : std::to_wstring(duplicatesCount));
		graphics.DrawString(duplicates.c_str(), duplicates.length(), &font, point, &brush);

		EndPaint(hWnd, &ps);

		break;
	}

	case WM_CREATE:
	{
		CreateWindow(L"button", L"Get first path",
			WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
			xMargin, yMargin,
			buttonWidth, buttonHeight,
			hWnd, HMENU(firstPathButtonId), hInst, NULL);

		CreateWindow(L"button", L"Get second path",
			WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
			xMargin, buttonHeight + yMargin * 2,
			buttonWidth, buttonHeight,
			hWnd, HMENU(secondPathButtonId), hInst, NULL);

		CreateWindow(L"button", L"Check extension",
			WS_CHILD | WS_VISIBLE | BS_CHECKBOX,
			xMargin, buttonHeight * 2 + yMargin * 3,
			buttonWidth, buttonHeight,
			hWnd, HMENU(extensionCheckBoxId), hInst, NULL);

		CreateWindow(L"button", L"Find duplicates",
			WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
			xMargin, buttonHeight * 3 + yMargin * 4,
			buttonWidth, buttonHeight,
			hWnd, HMENU(findDuplicatesButtonId), hInst, NULL);

		CreateWindow(L"button", L"Delete duplicates",
			WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
			xMargin, buttonHeight * 4 + yMargin * 5,
			buttonWidth, buttonHeight,
			hWnd, HMENU(deleteButtonId), hInst, NULL);

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
	LPWSTR fileName = new WCHAR[maxPathLength];
	fileName[0] = '\0';

	OPENFILENAME openFile;
	ZeroMemory(&openFile, sizeof openFile);
	openFile.lStructSize = sizeof openFile;
	openFile.hwndOwner = nullptr;
	openFile.nMaxFile = maxPathLength;
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
	if (path.length() > maxDisplayedPathLength)
	{
		const wstring file(path.begin() + path.find_last_of(L'\\') + 1, path.end());
		if (file.length() > maxDisplayedPathLength - 4)
		{
			return file.substr(0, maxDisplayedPathLength / 2 - 3).append(L"...")
				.append(file.substr(file.length() - maxDisplayedPathLength / 2 + 3, file.length()));
		}
		return path.substr(0, maxDisplayedPathLength - 4 - file.length()).append(L"...\\").append(file);

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