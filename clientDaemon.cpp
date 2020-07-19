#include <iostream>
#include <fstream>
#include <windows.h>
#include <comdef.h> 

using namespace std;

#include <gdiplus.h>
using namespace Gdiplus;

int helper_GetEncoderClsid(const WCHAR* format, CLSID* pClsid)  //HELPER FUNCTION
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}


void API_screenshotToBuffer()
{
	int Height = GetSystemMetrics(SM_CYSCREEN);
	int Width = GetSystemMetrics(SM_CXSCREEN);
	POINT a, b;
	a.x = 0;
	a.y = 0;

	b.x = Width;
	b.y = Height;


	// copy screen to bitmap
	HDC     hScreen = GetDC(NULL);
	HDC     hDC = CreateCompatibleDC(hScreen);
	HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, abs(b.x - a.x), abs(b.y - a.y));
	HGDIOBJ old_obj = SelectObject(hDC, hBitmap);
	BOOL    bRet = BitBlt(hDC, 0, 0, abs(b.x - a.x), abs(b.y - a.y), hScreen, a.x, a.y, SRCCOPY);

	// save bitmap to clipboard
	OpenClipboard(NULL);
	EmptyClipboard();
	SetClipboardData(CF_BITMAP, hBitmap);
	CloseClipboard();

	DWORD Junk;
	const char * BmpName = "C:/Users/al/source/repos/clientDaemon/Debug/file.bmp";
	HANDLE FH = CreateFileA(BmpName, GENERIC_WRITE, FILE_SHARE_WRITE, 0, CREATE_ALWAYS, 0, 0);
	WriteFile(FH, hBitmap, b.x*b.y*64, &Junk, 0);
	CloseHandle(FH);

	// clean up
	SelectObject(hDC, old_obj);
	DeleteDC(hDC);
	ReleaseDC(NULL, hScreen);
	DeleteObject(hBitmap);
}

//int API_screenToFile(const char * filePath) {
int API_screenToFile(const wchar_t* filePath) {
	HDC hdc = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);
	if (hdc == NULL) {
		printf("Couldn't create device context\n");
		return 1;
	}

	DWORD dwWidth, dwHeight, dwBPP, dwNumColors;
	//dwWidth  = GetDeviceCaps(hdc, HORZRES);
	//dwHeight = GetDeviceCaps(hdc, VERTRES);
	dwWidth = GetSystemMetrics(SM_CXSCREEN);
	dwHeight = GetSystemMetrics(SM_CYSCREEN);

	dwBPP = GetDeviceCaps(hdc, BITSPIXEL);
	if (dwBPP <= 8) {
		dwNumColors = GetDeviceCaps(hdc, NUMCOLORS);
		dwNumColors = 256;
	}
	else {
		dwNumColors = 0;
	}

	// Create compatible DC.
	HDC hdc2 = CreateCompatibleDC(hdc);
	if (hdc2 == NULL) {
		DeleteDC(hdc);
		printf("Couldn't create compatible device context\n");
		return 1;
	}

	// Create bitmap.
	LPVOID pBits;
	HBITMAP bitmap;
	BITMAPINFO bmInfo;

	bmInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmInfo.bmiHeader.biWidth = dwWidth;
	bmInfo.bmiHeader.biHeight = dwHeight;
	bmInfo.bmiHeader.biPlanes = 1;
	bmInfo.bmiHeader.biBitCount = (WORD)dwBPP;
	bmInfo.bmiHeader.biCompression = BI_RGB;
	bmInfo.bmiHeader.biSizeImage = 0;
	bmInfo.bmiHeader.biXPelsPerMeter = 0;
	bmInfo.bmiHeader.biYPelsPerMeter = 0;
	bmInfo.bmiHeader.biClrUsed = dwNumColors;
	bmInfo.bmiHeader.biClrImportant = dwNumColors;

	bitmap = CreateDIBSection(hdc, &bmInfo, DIB_PAL_COLORS, &pBits, NULL, 0);
	if (bitmap == NULL) {
		DeleteDC(hdc);
		DeleteDC(hdc2);
		printf("Couldn't create compatible bitmap\n");
		return 1;
	}

	HGDIOBJ gdiobj = SelectObject(hdc2, (HGDIOBJ)bitmap);
	if ((gdiobj == NULL) || (gdiobj == (void*)(LONG_PTR)GDI_ERROR)) {
		DeleteDC(hdc);
		DeleteDC(hdc2);
		printf("Couldn't select bitmap\n");
		return 1;
	}
	if (!BitBlt(hdc2, 0, 0, dwWidth, dwHeight, hdc, 0, 0, SRCCOPY)) {
		DeleteDC(hdc);
		DeleteDC(hdc2);
		printf("Could not copy bitmap\n");
		return 1;
	}

	RGBQUAD colors[256];
	if (dwNumColors != 0)
		dwNumColors = GetDIBColorTable(hdc2, 0, dwNumColors, colors);

	// Fill in bitmap structures.
	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER bitmapinfoheader;

	bmfh.bfType = 0x04D42;
	bmfh.bfSize = ((dwWidth * dwHeight * dwBPP) / 8) + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (dwNumColors * sizeof(RGBQUAD));
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (dwNumColors * sizeof(RGBQUAD));
	bitmapinfoheader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapinfoheader.biWidth = dwWidth;
	bitmapinfoheader.biHeight = dwHeight;
	bitmapinfoheader.biPlanes = 1;
	bitmapinfoheader.biBitCount = (WORD)dwBPP;
	bitmapinfoheader.biCompression = BI_RGB;
	bitmapinfoheader.biSizeImage = 0;
	bitmapinfoheader.biXPelsPerMeter = 0;
	bitmapinfoheader.biYPelsPerMeter = 0;
	bitmapinfoheader.biClrUsed = dwNumColors;
	bitmapinfoheader.biClrImportant = 0;

	ofstream file;
	file.open(filePath, ios::binary | ios::trunc | ios::out);
	file.write((char*)&bmfh, sizeof(BITMAPFILEHEADER));
	file.write((char*)&bitmapinfoheader, sizeof(BITMAPINFOHEADER));

	if (dwNumColors != 0)
		file.write((char*)colors, sizeof(RGBQUAD)*dwNumColors);
	file.write((char*)pBits, (dwWidth*dwHeight*dwBPP) / 8);

	// Done!
	DeleteObject(bitmap);
	DeleteDC(hdc2);
	DeleteDC(hdc);

	printf("Bitmap captured to disk file\n");
	return 0;
}

void API_BMP_to_PNG(const wchar_t * path_BMP,const wchar_t * path_PNG) {
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	CLSID   encoderClsid;
	Status  stat;
	//Image*   image = new Image(L"file.bmp");
	Image*   image = new Image(path_BMP);

	// Get the CLSID of the PNG encoder.
	helper_GetEncoderClsid(L"image/png", &encoderClsid);

	//stat = image->Save(L"Bird.png", &encoderClsid, NULL);
	stat = image->Save(path_PNG, &encoderClsid, NULL);

	if (stat == Ok)
		printf("Bird.png was saved successfully\n");
	else
		printf("Failure: stat = %d\n", stat);

	delete image;
	GdiplusShutdown(gdiplusToken);
}


void API_screenToPNG(const wchar_t* resultPathPng) {



	DWORD nBufferLength = 300;
	TCHAR lpBuffer[300];
	DWORD size = GetTempPath(
		nBufferLength,  // размер буфера
		lpBuffer// буфер пути
	);

	TCHAR lpTempFileName[300];
	UINT r = GetTempFileName(
		lpBuffer,
		L"del",
		0,
		lpTempFileName
	);
	if (0 == r) {
		cout << "error with GetTempFileName." << endl;
		return;
	}

	//if VS studio set «Use Unicode Character set», type TCHAR is wchar_t. if  «Use Multi-byte character set» then TCHAR is char. 

	API_screenToFile(lpTempFileName);
	API_BMP_to_PNG(lpTempFileName, resultPathPng);

	const WCHAR* wc = lpTempFileName;
	_bstr_t b(wc);
	const char* c = b;
	remove(c);
}

int main()
{
	API_screenToPNG(L"file.png");
}