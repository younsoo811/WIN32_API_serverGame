#include <windows.h>
#include <TCHAR.H>
#include <stdio.h>

#define WM_ASYNC WM_USER+2
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg,
	WPARAM wParam, LPARAM lParam);
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpszCmdLine, int nCmdShow)
{
	HWND 	 hwnd;
	MSG 	 msg;
	WNDCLASS WndClass;
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = hInstance;
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = _T("Window Class Name");
	RegisterClass(&WndClass);
	hwnd = CreateWindow(
		_T("Window Class Name"),
		_T("Server Window"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		700,
		700,
		NULL,
		NULL,
		hInstance,
		NULL
		);
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}

#include <math.h>
#define  BSIZE 100	//지름

BOOL InRectangle(int mx, int my)
{
	if (mx<BSIZE * 5 && my<BSIZE * 5) return TRUE;	//사각형 안을 클릭한 경우
	else return FALSE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg,
	WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	HPEN hPen;
	PAINTSTRUCT ps;
	static WSADATA wsadata;
	static SOCKET s, cs;
	static TCHAR msg[200];
	static char buffer[100];
	static SOCKADDR_IN addr = { 0 }, c_addr;
	static TCHAR str[100];
	static TCHAR str2[100];
	int size, msgLen;
	TCHAR seps[] = _T(",");
	TCHAR* token;
	TCHAR* nexttoken;
	TCHAR Btk[100];
	static int dx, dy;
	static int	cx, cy;
	static int savex[5][5] = { 0 };
	static int savey[5][5] = { 0 };
	static BOOL isC;
	static BOOL isP;
	static int	iColor;
	static int sendx[5][5] = { 0 };
	static int sendy[5][5] = { 0 };
	static int sx, sy;

	switch (iMsg)
	{
	case WM_CREATE:
		WSAStartup(MAKEWORD(2, 2), &wsadata);
		s = socket(AF_INET, SOCK_STREAM, 0);
		addr.sin_family = AF_INET;
		addr.sin_port = 20;
		addr.sin_addr.s_addr = inet_addr("127.0.0.1");
		bind(s, (LPSOCKADDR)&addr, sizeof(addr));
		WSAAsyncSelect(s, hwnd, WM_ASYNC, FD_ACCEPT);
		if (listen(s, 5) == -1)
			return 0;
		break;
	case WM_ASYNC:
		switch (lParam)
		{
		case FD_ACCEPT:
			size = sizeof(c_addr);
			cs = accept(s, (LPSOCKADDR)&c_addr, &size);
			WSAAsyncSelect(cs, hwnd, WM_ASYNC, FD_READ);
			break;
		case FD_READ:
			msgLen = recv(cs, buffer, 100, 0);
			buffer[msgLen] = NULL;
			isP = true;
#ifdef _UNICODE
			msgLen = MultiByteToWideChar(CP_ACP, 0, buffer, strlen(buffer), NULL, NULL);
			MultiByteToWideChar(CP_ACP, 0, buffer, strlen(buffer), msg, msgLen);
			msg[msgLen] = NULL;
#else
			strcpy_s(msg, buffer);
#endif
			InvalidateRgn(hwnd, NULL, TRUE);
			break;
		default:
			break;
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		if (isP){
			if (_tcscmp(msg, _T(""))){
				token = _tcstok_s(msg, seps, &nexttoken);
				_tcscpy_s(Btk, _countof(Btk), token);
				sx = _wtoi(Btk);
				while (token != NULL){
					//다음 문자열 구하기
					_tcscpy_s(Btk, _countof(Btk), token);
					sy = _wtoi(Btk);
					token = _tcstok_s(NULL, seps, &nexttoken);
				}
				sendx[(sx - (BSIZE / 2)) / BSIZE][(sy - (BSIZE / 2)) / BSIZE] = sx;
				sendy[(sx - (BSIZE / 2)) / BSIZE][(sy - (BSIZE / 2)) / BSIZE] = sy;
			}
		}


		//사각틀 생성
		for (int i = 0; i < 5; i++){
			MoveToEx(hdc, BSIZE*(i + 1), 0, NULL);
			LineTo(hdc, BSIZE*(i + 1), BSIZE * 5);
		}

		for (int i = 0; i < 5; i++){
			MoveToEx(hdc, 0, BSIZE*(i + 1), NULL);
			LineTo(hdc, BSIZE * 5, BSIZE*(i + 1));
		}

		//서버에서 그렸던 원 그리기 (서버 : 붉은 원)
		hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 255));
		(HPEN)SelectObject(hdc, hPen);
		for (int i = 0; i < 5; i++){
			for (int j = 0; j < 5; j++){
				if (sendx[i][j] != 0 && sendy[i][j] != 0){
					Ellipse(hdc, sendx[i][j] - 50, sendy[i][j] - 50, sendx[i][j] + 50, sendy[i][j] + 50);
				}
			}
		}

		//클라이언트에서 그렸던 원 그리기 (클라이언트 : 푸른 원)
		hPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
		(HPEN)SelectObject(hdc, hPen);
		for (int i = 0; i < 5; i++){
			for (int j = 0; j < 5; j++){
				if (savex[i][j] != 0 && savey[i][j] != 0){

					Ellipse(hdc, savex[i][j] - 50, savey[i][j] - 50, savex[i][j] + 50, savey[i][j] + 50);
				}
			}
		}
		EndPaint(hwnd, &ps);
		break;
	case WM_LBUTTONDOWN:
		if (isP){	//클라이언트로부터 전송 받은 다음에 그릴 수 있음
			dx = LOWORD(lParam);	//마우스 좌표
			dy = HIWORD(lParam);

			if (InRectangle(dx, dy)) {	//큰 사각형 내부에 클릭 했는지 확인
				isC = true;
			}
			else isC = false;

			if (isC){
				cx = (dx / BSIZE)*BSIZE + BSIZE / 2;	//원의 중심 좌표 구하기
				cy = (dy / BSIZE)*BSIZE + BSIZE / 2;

				savex[dx / BSIZE][dy / BSIZE] = cx;
				savey[dx / BSIZE][dy / BSIZE] = cy;

				isP = false;	//클라이언트가 전송 보낼때 다시 true로 바뀜
				InvalidateRgn(hwnd, NULL, TRUE);

				wsprintf(str, TEXT("%d"), savex[dx / BSIZE][dy / BSIZE]);
				wsprintf(str2, TEXT("%d"), savey[dx / BSIZE][dy / BSIZE]);
				_tcscat_s(str, _T(","));
				_tcscat_s(str, str2);

				if (cs == INVALID_SOCKET)
					return 0;
				else
				{
#ifdef _UNICODE
					msgLen = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);
					WideCharToMultiByte(CP_ACP, 0, str, -1, buffer, msgLen, NULL, NULL);
#else
					strcpy_s(buffer, str);
#endif
					send(cs, (LPSTR)buffer, strlen(buffer) + 1, 0);
					return 0;
				}
			}
		}
		return 0;
	case WM_DESTROY:
		closesocket(s);
		WSACleanup();
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}
