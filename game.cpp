#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

int* mem = (int*)calloc(131072, 4);

int* traps = (int*)calloc(16384, 4);

int* background = (int*)calloc(1200*800, 4) + 1200*100;

int* visualmap = (int*)calloc(1200*800, 4) + 1200*100;

int* visualbackground = (int*)calloc(1200*800, 4) + 1200*100;

struct Player
{
	int a;
	float x;
	float y;
	float vx;
	float vy;
};

Player player;

MSG msg;

HBITMAP hbm;

RECT window;

unsigned long long maintimervalue = -130000;

HANDLE maintimer = CreateWaitableTimer(0, 1, 0);

void drawrect(int* p, int w, int x, int y, int dx, int dy, int c)
{
	int ax = 0, ay = 0;
	while (ay < dy)
	{
		*(p + (y + ay) * w + (x + ax)) = c;

		ax++;
		if (ax == dx)
		{
			ax = 0;
			ay++;
		}
	}
}

void replacerect(int* p, int w, int x, int y, int dx, int dy, int c, int cr)
{
	int ax = 0, ay = 0;
	while (ay < dy)
	{
		if (*(p + (y + ay) * w + (x + ax)) == cr) *(p + (y + ay) * w + (x + ax)) = c;

		ax++;
		if (ax == dx)
		{
			ax = 0;
			ay++;
		}
	}
}

void copyrect(int* pd, int* ps, int sw, int dw, int x, int y, int dx, int dy)
{
	if (x < 0 || x > 1200 || y < 0 || y > 600) return;

	int ax = 0, ay = 0;
	while (ay < dy)
	{
		if (*(ps + ay * sw + ax) != 0) *(pd + (y + ay) * dw + (x + ax)) = *(ps + ay * sw + ax);

		ax++;
		if (ax == dx)
		{
			ax = 0;
			ay++;
		}
	}
}

void fliprect(int* pd, int* ps, int sw, int dw, int x, int y, int dx, int dy)
{
	if (x < 0 || x > 1200 || y < 0 || y > 600) return;

	int ax = 0, ay = 0;
	while (ay < dy)
	{
		if (*(ps + ay * sw + ax) != 0) *(pd + (y + dy - ay) * dw + (x + ax)) = *(ps + ay * sw + ax);

		ax++;
		if (ax == dx)
		{
			ax = 0;
			ay++;
		}
	}
}

void stretchrect(int* pd, int* ps, int sw, int sh, int dw, int dh)
{
	int ax = 0, ay = 0;
	float xr = (float)sw / (float)dw, yr = (float)sh / (float)dh;

	while (ay < dh)
	{
		*(pd + ay * dw + ax) = *(ps + (int)((float)ay * yr) * sw + (int)((float)ax * xr));

		ax++;
		if (ax == dw)
		{
			ax = 0;
			ay++;
		}
	}
}

void drawline(int* p, int dw, int sx, int sy, int ex, int ey, int c)
{
	float ax = 0, ay = 0;
	float incx = ( sx < ex ? sqrt( (float)((ex - sx) * (ex - sx)) / (float)((ex - sx) * (ex - sx) + (ey - sy) * (ey - sy)) ) : 0 - sqrt( (float)((ex - sx) * (ex - sx)) / (float)((ex - sx) * (ex - sx) + (ey - sy) * (ey - sy)) ) );
	float incy = ( sy < ey ? sqrt( (float)((ey - sy) * (ey - sy)) / (float)((ex - sx) * (ex - sx) + (ey - sy) * (ey - sy)) ) : 0 - sqrt( (float)((ey - sy) * (ey - sy)) / (float)((ex - sx) * (ex - sx) + (ey - sy) * (ey - sy)) ) );

	while (fabsf(ax) <= fabsf(ex - sx) && fabsf(ay) <= fabsf(ey - sy))
	{
		*(p + (sy + (int)ay) * dw + (sx + (int)ax)) = c;

		ax += incx;
		ay += incy;
	}
}

void drawwalls()
{
	hbm = LoadBitmap(GetModuleHandle(0), "wall");
	int* wall = (int*)calloc(60*60, 4);
	GetBitmapBits(hbm, 60*60 * 4, wall);

	int ax = 0, ay = 0;
	while (ay < 600)
	{
		if (*(background + ay * 1200 + ax) == 1) copyrect(visualbackground, wall, 60, 1200, ax, ay, 60, 60);

		ax += 60;
		if (ax == 1200)
		{
			ax = 0;
			ay += 60;
		}
	}

	ax = 0;
	ay = 0;
	while (ay < 600)
	{
		if (*(background + ay * 1200 + ax) == 1)
		{
			if (*(background + (ay - 1) * 1200 + (ax - 1)) == 0 || *(background + (ay - 1) * 1200 + (ax + 1)) == 0 || *(background + (ay + 1) * 1200 + (ax - 1)) == 0 || *(background + (ay + 1) * 1200 + (ax + 1)) == 0)
			{
				*(visualbackground + ay * 1200 + ax) = 0xb4b4b4;
			}
		}

		ax++;
		if (ax == 1200)
		{
			ax = 0;
			ay++;
		}
	}

	free(wall);
}

void retrygame(int* trapssp)
{
	*(mem + 1) = 0;

	CopyMemory(traps, trapssp, 16376 * 4);

	player.x = (float)*(traps + 3);
	player.y = (float)(*(traps + 4) - 7);
	player.vx = 0.2;
	player.vy = 0;

	*(mem + 1) = 2;
}

LRESULT CALLBACK windowcb(HWND handle, unsigned int message, WPARAM wparam, LPARAM lparam)
{
	GetClientRect(handle, &window);

	if (message == WM_CLOSE)
	{
		*mem = 0;
		DestroyWindow(handle);
	}
	else return DefWindowProc(handle, message, wparam, lparam);

	return 0;
}

LRESULT CALLBACK dialogcb(HWND handle, unsigned int message, WPARAM wparam, LPARAM lparam)
{
	if (message == WM_COMMAND && LOWORD(wparam) != 0)
	{
		EndDialog(handle, LOWORD(wparam));
	}
	return 0;
}

void processmsg()
{
	while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	GetAsyncKeyState(VK_LBUTTON);
	GetAsyncKeyState(VK_SPACE);
	GetAsyncKeyState('S');
	GetAsyncKeyState('R');
	WaitForSingleObject(maintimer, INFINITE);
	SetWaitableTimer(maintimer, (LARGE_INTEGER*)&maintimervalue, 0, 0, 0, 0);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	RECT rect = { 0, 0, 1200, 600 };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, 0);

	HWND handle = CreateWindow("STATIC", "Custom Dungeons", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, 0, 0, 0, 0);
	SetWindowLongPtr(handle, GWLP_WNDPROC, (LONG_PTR)windowcb);

	ShowWindow(handle, SW_SHOWNORMAL);
	UpdateWindow(handle);

	tagBITMAPINFO bmi;
	bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
	bmi.bmiHeader.biWidth = 1200;
	bmi.bmiHeader.biHeight = 600;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	HDC hdc = GetDC(handle);
	SetBkMode(hdc, TRANSPARENT);
	SetStretchBltMode(hdc, COLORONCOLOR);

	hbm = LoadBitmap(hInstance, "door");
	int* door = (int*)calloc(40*50, 4);
	GetBitmapBits(hbm, 40*50 * 4, door);

	hbm = LoadBitmap(hInstance, "finish");
	int* finish = (int*)calloc(50*60, 4);
	GetBitmapBits(hbm, 50*60 * 4, finish);

	hbm = LoadBitmap(hInstance, "stallingleft");
	int* stallingleft = (int*)calloc(169, 4);
	GetBitmapBits(hbm, 169*4, stallingleft);

	hbm = LoadBitmap(hInstance, "stallingright");
	int* stallingright = (int*)calloc(169, 4);
	GetBitmapBits(hbm, 169*4, stallingright);

	hbm = LoadBitmap(hInstance, "movingleft");
	int* movingleft = (int*)calloc(169, 4);
	GetBitmapBits(hbm, 169*4, movingleft);

	hbm = LoadBitmap(hInstance, "movingright");
	int* movingright = (int*)calloc(169, 4);
	GetBitmapBits(hbm, 169*4, movingright);

	hbm = LoadBitmap(hInstance, "slidingleft");
	int* slidingleft = (int*)calloc(169, 4);
	GetBitmapBits(hbm, 169*4, slidingleft);

	hbm = LoadBitmap(hInstance, "slidingright");
	int* slidingright = (int*)calloc(169, 4);
	GetBitmapBits(hbm, 169*4, slidingright);

	hbm = LoadBitmap(hInstance, "saw");
	int* saw = (int*)calloc(900, 4);
	GetBitmapBits(hbm, 900*4, saw);

	hbm = LoadBitmap(hInstance, "redguardleft");
	int* redguardleft = (int*)calloc(900, 4);
	GetBitmapBits(hbm, 900*4, redguardleft);

	hbm = LoadBitmap(hInstance, "redguardright");
	int* redguardright = (int*)calloc(900, 4);
	GetBitmapBits(hbm, 900*4, redguardright);

	hbm = LoadBitmap(hInstance, "spinner");
	int* spinner = (int*)calloc(900, 4);
	GetBitmapBits(hbm, 900*4, spinner);

	hbm = LoadBitmap(hInstance, "guide");
	int* guide = (int*)calloc(900, 4);
	GetBitmapBits(hbm, 900*4, guide);

	hbm = LoadBitmap(hInstance, "wasp");
	int* wasp = (int*)calloc(18*18, 4);
	GetBitmapBits(hbm, 18*18*4, wasp);

	hbm = LoadBitmap(hInstance, "waspwings");
	int* waspwings = (int*)calloc(18*18, 4);
	GetBitmapBits(hbm, 18*18*4, waspwings);

	hbm = LoadBitmap(hInstance, "bullet");
	int* bullet = (int*)calloc(9*9, 4);
	GetBitmapBits(hbm, 9*9 * 4, bullet);

	hbm = LoadBitmap(hInstance, "cannon");
	int* cannon = (int*)calloc(900, 4);
	GetBitmapBits(hbm, 900*4, cannon);

	hbm = LoadBitmap(hInstance, "homingcannon");
	int* homingcannon = (int*)calloc(900, 4);
	GetBitmapBits(hbm, 900*4, homingcannon);

	hbm = LoadBitmap(hInstance, "gravity1");
	int* gravity1 = (int*)calloc(50*50, 4);
	GetBitmapBits(hbm, 50*50 * 4, gravity1);

	hbm = LoadBitmap(hInstance, "gravity2");
	int* gravity2 = (int*)calloc(50*50, 4);
	GetBitmapBits(hbm, 50*50 * 4, gravity2);

	hbm = LoadBitmap(hInstance, "warder");
	int* warder = (int*)calloc(900, 4);
	GetBitmapBits(hbm, 900*4, warder);

	hbm = LoadBitmap(hInstance, "sniper");
	int* sniper = (int*)calloc(900, 4);
	GetBitmapBits(hbm, 900*4, sniper);

	hbm = LoadBitmap(hInstance, "platform");
	int* platform = (int*)calloc(60*6, 4);
	GetBitmapBits(hbm, 60*6 * 4, platform);

	hbm = LoadBitmap(hInstance, "jumppad");
	int* jumppad = (int*)calloc(60*6, 4);
	GetBitmapBits(hbm, 60*6 * 4, jumppad);

	hbm = LoadBitmap(hInstance, "editbutton");
	int* editbutton = (int*)calloc(120*40, 4);
	GetBitmapBits(hbm, 120*40 * 4, editbutton);

	hbm = LoadBitmap(hInstance, "savebutton");
	int* savebutton = (int*)calloc(120*40, 4);
	GetBitmapBits(hbm, 120*40 * 4, savebutton);

	hbm = LoadBitmap(hInstance, "addbutton");
	int* addbutton = (int*)calloc(120*40, 4);
	GetBitmapBits(hbm, 120*40 * 4, addbutton);

	hbm = LoadBitmap(hInstance, "addwallsbutton");
	int* addwallsbutton = (int*)calloc(120*40, 4);
	GetBitmapBits(hbm, 120*40 * 4, addwallsbutton);

	hbm = LoadBitmap(hInstance, "importbutton");
	int* importbutton = (int*)calloc(120*40, 4);
	GetBitmapBits(hbm, 120*40 * 4, importbutton);

	hbm = LoadBitmap(hInstance, "backbutton");
	int* backbutton = (int*)calloc(30*30, 4);
	GetBitmapBits(hbm, 30*30 * 4, backbutton);

	hbm = LoadBitmap(hInstance, "trash");
	int* trash = (int*)calloc(120*40, 4);
	GetBitmapBits(hbm, 120*40 * 4, trash);

	hbm = LoadBitmap(hInstance, "addtraps");
	int* addtraps = (int*)calloc(120*60, 4);
	GetBitmapBits(hbm, 120*60 * 4, addtraps);

	hbm = LoadBitmap(hInstance, "replaying");
	int* replaying = (int*)calloc(120*40, 4);
	GetBitmapBits(hbm, 120*40 * 4, replaying);

	player.a = 6;
	int moveafterslide = 0;
	int timepos = 0;
	int lit = -100;
	float animpos = 9;
	float delanimpos = 0.1;
	int gravity = 1;
	int tgc = 0;
	int trapdeath = 0;
	int deaths = 0;
	int lastplaysound = 0;
	float highjump = 0;

	int* recording = (int*)calloc(8192, 4);
	int jumpcount = 0;

	int* solutionreplay = 0;
	int isreplaying = 0;
	int areplaypos = 0;

	char* jumpsound = (char*)LockResource(LoadResource(hInstance, FindResource(hInstance, "jumpsound", "ARRAY")));
	char* deathsound = (char*)LockResource(LoadResource(hInstance, FindResource(hInstance, "deathsound", "ARRAY")));
	char* bulletsound = (char*)LockResource(LoadResource(hInstance, FindResource(hInstance, "bulletsound", "ARRAY")));
	char* shootsound = (char*)LockResource(LoadResource(hInstance, FindResource(hInstance, "shootsound", "ARRAY")));

	int* rotate30 = (int*)LockResource(LoadResource(hInstance, FindResource(hInstance, "rt30x30", "ARRAY")));
	int* rotate18 = (int*)LockResource(LoadResource(hInstance, FindResource(hInstance, "rt18x18", "ARRAY")));

	int holdjumpblocker = 1;

	int editorpx = 0;
	int editorpy = 0;
	int beforeclickx = 0;
	int beforeclicky = 0;

	*(traps + 2) = 33;
	*(traps + 3) = 90;
	*(traps + 4) = 120;

	*(traps + 14) = 34;
	*(traps + 15) = 150;
	*(traps + 16) = 120;

	drawrect(background, 1200, 0, 0, 1200, 60, 1);
	drawrect(background, 1200, 1140, 0, 60, 600, 1);
	drawrect(background, 1200, 0, 540, 1200, 60, 1);
	drawrect(background, 1200, 0, 0, 60, 600, 1);

	drawrect(visualbackground, 1200, 0, 0, 1200, 600, 0x383838);
	drawwalls();

	int* rtd30 = (int*)calloc(900, 4);
	int* rtd18 = (int*)calloc(324, 4);
	int* movinganim = (int*)calloc(13*17, 4);

	*(mem + 1) = 1;

	int* trapssp = (int*)calloc(16384, 4);
	CopyMemory(trapssp, traps, 16376 * 4);

	retrygame(trapssp);
	
	SetWaitableTimer(maintimer, (LARGE_INTEGER*)&maintimervalue, 0, 0, 0, 0);

	*mem = 1;
	while (*mem == 1)
	{
		processmsg();

	startloop:

		CopyMemory(visualmap, visualbackground, 2880000);

		POINT mp;
		GetCursorPos(&mp);
		ScreenToClient(handle, &mp);
		
		mp.x = mp.x * 1200 / (window.right - window.left);
		mp.y = mp.y * 600 / (window.bottom - window.top);

		if (*(mem + 1) < 4)
		{
			copyrect(visualmap, door, 40, 1200, *(traps + 3) - 20, *(traps + 4) - 50, 40, 50);
			copyrect(visualmap, finish, 50, 1200, *(traps + 15) - 25, *(traps + 16) - 60, 50, 60);

			if (*(mem + 1) == 1 && player.x > *(traps + 15) - 25 && player.x < *(traps + 15) + 25 && player.y > *(traps + 16) - 35 && player.y < *(traps + 16) + 5)
			{
				if (isreplaying)
				{
					isreplaying = 0;
					areplaypos = 0;

					retrygame(trapssp);
					goto startloop;
				}
				
				if (solutionreplay) {
					DialogBox(hInstance, "completed", handle, dialogcb);
					retrygame(trapssp);
					goto startloop;
				}

				int ans = DialogBox(hInstance, "solved", handle, dialogcb);

				if (ans == 2) retrygame(trapssp);
				else if (ans == 3)
				{
					OPENFILENAME ofn;
					ZeroMemory(&ofn, sizeof(OPENFILENAME));
					ofn.lStructSize = sizeof(OPENFILENAME);
					ofn.hwndOwner = handle;
					ofn.lpstrTitle = "Export";
					ofn.lpstrDefExt = "dun";
					ofn.lpstrFilter = "*.dun\0\0";
					char ofnlocation[512] = "mydungeon";
					ofn.lpstrFile = ofnlocation;
					ofn.nMaxFile = 512;

					ofn.Flags = OFN_PATHMUSTEXIST | OFN_EXPLORER;

					GetSaveFileName(&ofn);

					if (!strcmp(ofnlocation, "mydungeon")) goto startloop;

					FILE* toofn = fopen(ofnlocation, "wb");

					int* ofnbg = (int*)calloc(200, 4);
					int wallssavex = 0, wallssavey = 0;
					while (wallssavey < 10)
					{
						if (*(background + wallssavey * 60 * 1200 + wallssavex * 60) == 1) *(ofnbg + wallssavey * 20 + wallssavex) = 1;

						wallssavex++;
						if (wallssavex == 20)
						{
							wallssavex = 0;
							wallssavey++;
						}
					}

					int trapsend = 2;
					while (*(trapssp + trapsend) != 0) trapsend += 12;

					int* data2 = (int*)calloc(200 + 2 + trapsend + jumpcount + 8, 4);

					CopyMemory(data2, ofnbg, 200 * 4);
					*(data2 + 200 + 0) = trapsend;
					*(data2 + 200 + 1) = jumpcount;
					CopyMemory(data2 + 200 + 2, trapssp, trapsend * 4);
					CopyMemory(data2 + 200 + 2 + trapsend, recording, jumpcount * 4);

					int ofnsuccess = 1;
					if (fwrite(data2, (202 + trapsend + jumpcount) * 4, 1, toofn) != 1) if (MessageBox(handle, "Error occurred!", "Warning!", MB_RETRYCANCEL | MB_ICONEXCLAMATION | MB_APPLMODAL) == IDRETRY) ofnsuccess = 0;

					fclose(toofn);
					free(data2);
					free(ofnbg);

					if (ofnsuccess == 0) goto startloop;
					else retrygame(trapssp);
				}
			}
		}

		int fpr = 0;
		while (*(mem + 1) == 1)
		{
			if (isreplaying)
			{
				if (*(solutionreplay + areplaypos) == timepos)
				{
					lit = timepos;
					areplaypos++;
				}
			}
			else
			{
				if (GetAsyncKeyState(VK_LBUTTON) != 0 || GetAsyncKeyState(VK_SPACE) != 0)
				{
					if (holdjumpblocker == 0 && timepos - lit > 35)
					{
						lit = timepos;
						*(recording + jumpcount) = timepos;
						jumpcount++;
					}
					holdjumpblocker = 1;
				}
				else holdjumpblocker = 0;
			}


			int gwall = 0, cwall = 0, lwall = 0, rwall = 0;

			int iwh = 0;
			while (iwh < 13)
			{
				int aifground = *(background + ((int)player.y + player.a + 1) * 1200 + ((int)player.x - player.a + iwh));
				int aifinishground = *(background + ((int)player.y + player.a) * 1200 + ((int)player.x - player.a + iwh));

				if (aifground == 1 || (player.vy >= 0 && (aifground == 2 || (aifground == 3 && timepos - lit < 60)) ) )
				{
					gwall = 1;
					highjump = 0;
				}
				else if ( player.vy >= 0 && (aifground == 4 || (aifground == 5 && timepos - lit < 60)) )
				{
					gwall = 1;
					highjump = 1.45;
				}

				if (*(background + ((int)player.y - player.a - 1) * 1200 + ((int)player.x - player.a + iwh)) == 1)
				{
					cwall = 1;
				}
				if (*(background + ((int)player.y - player.a + iwh) * 1200 + ((int)player.x - player.a - 1)) == 1)
				{
					lwall = 1;
				}
				if (*(background + ((int)player.y - player.a + iwh) * 1200 + ((int)player.x + player.a + 1)) == 1)
				{
					rwall = 1;
				}
				if ((aifinishground == 1 || aifinishground == 2 || aifinishground == 4) && (*(background + (int)player.y * 1200 + ((int)player.x + player.a + 1)) == 0 || *(background + (int)player.y * 1200 + (int)player.x) == 1) )
				{
					gwall = 0;
					lwall = 0;
					rwall = 0;
					player.y--;
					break;
				}

				iwh++;
			}

			if (gravity == 1)
			{
				if (timepos - lit < 60)
				{
					int ifplaysound = 1;

					moveafterslide = 0;

					if (gwall == 1 && lwall == 0 && rwall == 0)
					{
						if (highjump != 0) player.vy = -0.5 * highjump;
						else player.vy = -0.5;
					}
					else if (gwall == 1)
					{
						if (highjump != 0) player.vy = -0.5 * highjump;
						else player.vy = -0.5;
						player.vx = 0;
					}
					else if (lwall == 1 && player.vy > -0.2)
					{
						player.vy = -0.53;
						player.vx = 0.2;
					}
					else if (rwall == 1 && player.vy > -0.2)
					{
						player.vy = -0.53;
						player.vx = -0.2;
					}
					else if ((lwall == 1 && player.vx < 0) || (rwall == 1 && player.vx > 0))
					{
						player.vx = 0;

						ifplaysound = 0;
					}
					else ifplaysound = 0;

					if (ifplaysound == 1 && lastplaysound != lit)
					{
						lastplaysound = lit;
						PlaySound(jumpsound, 0, SND_MEMORY | SND_ASYNC);
					}
				}
				else
				{
					if (gwall == 1) player.vy = 0;
					if (lwall == 1 || rwall == 1) player.vx = 0;
				}

				if (cwall == 1 && player.vy < 0) player.vy = 0;

				if (gwall == 0 && lwall == 0 && rwall == 0)
				{
					if (player.vy < 0.95) player.vy += 0.003;

					if (moveafterslide == 1) player.vx = -0.2;
					else if (moveafterslide == 2) player.vx = 0.2;
					else moveafterslide = 0;
				}
				else if (gwall == 0)
				{
					if (lwall == 1) moveafterslide = 1;
					else moveafterslide = 2;

					if (player.vy < 0) player.vy += 0.0034;
					else player.vy = 0.1;
				}

				if (gwall) player.x += player.vx;
				else player.x += player.vx * 0.96;
				player.y += player.vy;

				fpr++;
				timepos++;

				if (fpr == 8)
				{
					if (player.vx > 0 || (player.vy < 0 && rwall == 1))
					{
						if (gwall == 1)
						{
							if (animpos > 13) delanimpos = -0.2;
							else if (animpos < 12) delanimpos = 0.2;

							animpos += delanimpos;

							stretchrect(movinganim, movingright, 13, 13, 26 - (int)animpos, (int)animpos);
							copyrect(visualmap, movinganim, 26 - (int)animpos, 1200, (int)player.x - 6, (int)player.y + 7 - (int)animpos, 26 - (int)animpos, (int)animpos);
						}
						else
						{
							animpos = 8;
							copyrect(visualmap, movingright, 13, 1200, (int)player.x - 6, (int)player.y - 6, 13, 13);
						}

					}
					else if (player.vx < 0 || (player.vy < 0 && lwall == 1))
					{
						if (gwall == 1)
						{
							if (animpos > 13) delanimpos = -0.2;
							else if (animpos < 12) delanimpos = 0.2;

							animpos += delanimpos;

							stretchrect(movinganim, movingleft, 13, 13, 26 - (int)animpos, (int)animpos);
							copyrect(visualmap, movinganim, 26 - (int)animpos, 1200, (int)player.x - 19 + (int)animpos, (int)player.y + 7 - (int)animpos, 26 - (int)animpos, (int)animpos);
						}
						else
						{
							animpos = 8;
							copyrect(visualmap, movingleft, 13, 1200, (int)player.x - 6, (int)player.y - 6, 13, 13);
						}
					}
					else if (rwall == 1 && player.vy > 0) copyrect(visualmap, slidingleft, 13, 1200, (int)player.x - 6, (int)player.y - 6, 13, 13);
					else if (lwall == 1 && player.vy > 0) copyrect(visualmap, slidingright, 13, 1200, (int)player.x - 6, (int)player.y - 6, 13, 13);
					else if (rwall == 1)
					{
						if (animpos > 17) delanimpos = -0.4;
						else if (animpos < 13) delanimpos = 0.4;

						animpos += delanimpos;

						stretchrect(movinganim, stallingleft, 13, 13, 13, (int)animpos);
						copyrect(visualmap, movinganim, 13, 1200, (int)player.x - 6, (int)player.y + 7 - (int)animpos, 13, (int)animpos);
					}
					else if (lwall == 1)
					{
						if (animpos > 17) delanimpos = -0.4;
						else if (animpos < 13) delanimpos = 0.4;

						animpos += delanimpos;

						stretchrect(movinganim, stallingright, 13, 13, 13, (int)animpos);
						copyrect(visualmap, movinganim, 13, 1200, (int)player.x - 6, (int)player.y + 7 - (int)animpos, 13, (int)animpos);
					}

					break;
				}
			}
			else if (gravity == -1)
			{
				if (timepos - lit < 60)
				{
					int ifplaysound = 1;

					moveafterslide = 0;

					if (cwall == 1 && lwall == 0 && rwall == 0)
					{
						player.vy = 0.5;
					}
					else if (cwall == 1)
					{
						player.vy = 0.5;
						player.vx = 0;
					}
					else if (lwall == 1 && player.vy < 0.2)
					{
						player.vy = 0.53;
						player.vx = 0.2;
					}
					else if (rwall == 1 && player.vy < 0.2)
					{
						player.vy = 0.53;
						player.vx = -0.2;
					}
					else if ((lwall == 1 && player.vx < 0) || (rwall == 1 && player.vx > 0))
					{
						player.vx = 0;

						ifplaysound = 0;
					}
					else ifplaysound = 0;

					if (ifplaysound == 1 && lastplaysound != lit)
					{
						lastplaysound = lit;
						PlaySound(jumpsound, 0, SND_MEMORY | SND_ASYNC);
					}
				}
				else
				{
					if (cwall == 1) player.vy = 0;
					if (lwall == 1 || rwall == 1) player.vx = 0;
				}

				if (gwall == 1 && player.vy > 0) player.vy = 0;

				if (cwall == 0 && lwall == 0 && rwall == 0)
				{
					if (player.vy > -0.95) player.vy -= 0.003;

					if (moveafterslide == 1) player.vx = -0.2;
					else if (moveafterslide == 2) player.vx = 0.2;
					else moveafterslide = 0;
				}
				else if (cwall == 0)
				{
					if (lwall == 1) moveafterslide = 1;
					else moveafterslide = 2;

					if (player.vy > 0) player.vy -= 0.0034;
					else player.vy = -0.1;
				}

				if (cwall) player.x += player.vx;
				else player.x += player.vx * 0.96;
				player.y += player.vy;

				fpr++;
				timepos++;

				if (fpr == 8)
				{
					if (player.vx > 0 || (player.vy > 0 && rwall == 1))
					{
						if (cwall == 1)
						{
							if (animpos > 13) delanimpos = -0.2;
							else if (animpos < 12) delanimpos = 0.2;

							animpos += delanimpos;

							stretchrect(movinganim, movingright, 13, 13, 26 - (int)animpos, (int)animpos);
							fliprect(visualmap, movinganim, 26 - (int)animpos, 1200, (int)player.x - 6, (int)player.y - 6, 26 - (int)animpos, (int)animpos);
						}
						else
						{
							animpos = 8;
							fliprect(visualmap, movingright, 13, 1200, (int)player.x - 6, (int)player.y - 6, 13, 13);
						}

					}
					else if (player.vx < 0 || (player.vy > 0 && lwall == 1))
					{
						if (cwall == 1)
						{
							if (animpos > 13) delanimpos = -0.2;
							else if (animpos < 12) delanimpos = 0.2;

							animpos += delanimpos;

							stretchrect(movinganim, movingleft, 13, 13, 26 - (int)animpos, (int)animpos);
							fliprect(visualmap, movinganim, 26 - (int)animpos, 1200, (int)player.x - 6, (int)player.y - 6, 26 - (int)animpos, (int)animpos);
						}
						else
						{
							animpos = 8;
							fliprect(visualmap, movingleft, 13, 1200, (int)player.x - 6, (int)player.y - 6, 13, 13);
						}
					}
					else if (rwall == 1 && player.vy < 0) fliprect(visualmap, slidingleft, 13, 1200, (int)player.x - 6, (int)player.y - 6, 13, 13);
					else if (lwall == 1 && player.vy < 0) fliprect(visualmap, slidingright, 13, 1200, (int)player.x - 6, (int)player.y - 6, 13, 13);
					else if (rwall == 1)
					{
						if (animpos > 17) delanimpos = -0.4;
						else if (animpos < 13) delanimpos = 0.4;

						animpos += delanimpos;

						stretchrect(movinganim, stallingleft, 13, 13, 13, (int)animpos);
						fliprect(visualmap, movinganim, 13, 1200, (int)player.x - 6, (int)player.y - 6, 13, (int)animpos);
					}
					else if (lwall == 1)
					{
						if (animpos > 17) delanimpos = -0.4;
						else if (animpos < 13) delanimpos = 0.4;

						animpos += delanimpos;

						stretchrect(movinganim, stallingright, 13, 13, 13, (int)animpos);
						fliprect(visualmap, movinganim, 13, 1200, (int)player.x - 6, (int)player.y - 6, 13, (int)animpos);
					}

					break;
				}
			}
		}

		int tm = 2;
		while (*(traps + tm) != 0 && (*(mem + 1) == 1 || *(mem + 1) == 2))
		{
			if (*(traps + tm) == 1)
			{
				if ((player.x - (float)*(traps + tm + 2)) * (player.x - (float)*(traps + tm + 2)) + (player.y - (float)*(traps + tm + 3)) * (player.y - (float)*(traps + tm + 3)) < 200.0)
				{
					trapdeath = 1;
					*(mem + 1) = 2;
					break;
				}

				int ax = 0, ay = 0;
				while (ay < 30)
				{
					if (*(rotate30 + (int)*(double*)(traps + tm + 4) * 900 + ay * 30 + ax) != 0)
					{
						*(rtd30 + ay * 30 + ax) = *(saw + LOWORD(*(rotate30 + (int)*(double*)(traps + tm + 4) * 900 + ay * 30 + ax)) * 30 + HIWORD(*(rotate30 + (int)*(double*)(traps + tm + 4) * 900 + ay * 30 + ax)) );
					}

					ax++;
					if (ax == 30)
					{
						ax = 0;
						ay++;
					}
				}

				copyrect(visualmap, rtd30, 30, 1200, *(traps + tm + 2) - 15, *(traps + tm + 3) - 15, 30, 30);

				if (*(double*)(traps + tm + 4) > 0.9) *(double*)(traps + tm + 4) -= 0.8;
				else *(double*)(traps + tm + 4) = 63;
			}
			else if (*(traps + tm) == 2)
			{
				if ((player.x - *(float*)(traps + tm + 8)) * (player.x - *(float*)(traps + tm + 8)) + (player.y - *(float*)(traps + tm + 9)) * (player.y - *(float*)(traps + tm + 9)) < 260.0)
				{
					trapdeath = 1;
					*(mem + 1) = 2;
					break;
				}

				if (*(traps + tm + 6) == 1)
				{
					if (*(float*)(traps + tm + 8) < *(traps + tm + 4))
					{
						if ( *(float*)(traps + tm + 8) * 2 > (float)*(traps + tm + 2) + (float)*(traps + tm + 4) ) *(float*)(traps + tm + 10) -= 10 / (*(float*)(traps + tm + 8) * 2 - (float)*(traps + tm + 4) - (float)*(traps + tm + 2) + 105);
						else if ( *(float*)(traps + tm + 8) * 2 < (float)*(traps + tm + 2) + (float)*(traps + tm + 4) ) *(float*)(traps + tm + 10) += 10 / ((float)*(traps + tm + 2) + (float)*(traps + tm + 4) - *(float*)(traps + tm + 8) * 2 + 105);

						if (*(float*)(traps + tm + 10) < 0.3) *(float*)(traps + tm + 10) = 0.3;

						*(float*)(traps + tm + 8) += *(float*)(traps + tm + 10);

						copyrect(visualmap, redguardright, 30, 1200, (int)*(float*)(traps + tm + 8) - 15, (int)*(float*)(traps + tm + 9) - 15, 30, 30);
					}
					else
					{
						*(float*)(traps + tm + 10) = 0;
						*(traps + tm + 6) = 2;
					}
				}
				else if (*(traps + tm + 6) == 2)
				{
					if (*(float*)(traps + tm + 8) > *(traps + tm + 2))
					{
						if ( *(float*)(traps + tm + 8) * 2 > (float)*(traps + tm + 2) + (float)*(traps + tm + 4) ) *(float*)(traps + tm + 10) -= 10 / (*(float*)(traps + tm + 8) * 2 - (float)*(traps + tm + 4) - (float)*(traps + tm + 2) + 105);
						else if ( *(float*)(traps + tm + 8) * 2 < (float)*(traps + tm + 2) + (float)*(traps + tm + 4) ) *(float*)(traps + tm + 10) += 10 / ((float)*(traps + tm + 2) + (float)*(traps + tm + 4) - *(float*)(traps + tm + 8) * 2 + 105);

						if (*(float*)(traps + tm + 10) > -0.3) *(float*)(traps + tm + 10) = -0.3;

						*(float*)(traps + tm + 8) += *(float*)(traps + tm + 10);

						copyrect(visualmap, redguardleft, 30, 1200, (int)*(float*)(traps + tm + 8) - 15, (int)*(float*)(traps + tm + 9) - 15, 30, 30);
					}
					else
					{
						*(float*)(traps + tm + 10) = 0;
						*(traps + tm + 6) = 1;
					}
				}

				if (*(traps + tm + 7) == 1)
				{
					if (*(float*)(traps + tm + 9) < *(traps + tm + 5))
					{
						if ( *(float*)(traps + tm + 9) * 2 > (float)*(traps + tm + 3) + (float)*(traps + tm + 5) ) *(float*)(traps + tm + 11) -= 10 / (*(float*)(traps + tm + 9) * 2 - (float)*(traps + tm + 5) - (float)*(traps + tm + 3) + 105);
						else if ( *(float*)(traps + tm + 9) * 2 < (float)*(traps + tm + 3) + (float)*(traps + tm + 5) ) *(float*)(traps + tm + 11) += 10 / ((float)*(traps + tm + 3) + (float)*(traps + tm + 5) - *(float*)(traps + tm + 9) * 2 + 105);

						if (*(float*)(traps + tm + 11) < 0.3) *(float*)(traps + tm + 11) = 0.3;

						*(float*)(traps + tm + 9) += *(float*)(traps + tm + 11);

						copyrect(visualmap, redguardright, 30, 1200, (int)*(float*)(traps + tm + 8) - 15, (int)*(float*)(traps + tm + 9) - 15, 30, 30);
					}
					else
					{
						*(float*)(traps + tm + 11) = 0;
						*(traps + tm + 7) = 2;
					}
				}
				else if (*(traps + tm + 7) == 2)
				{
					if (*(float*)(traps + tm + 9) > *(traps + tm + 3))
					{
						if ( *(float*)(traps + tm + 9) * 2 > (float)*(traps + tm + 3) + (float)*(traps + tm + 5) ) *(float*)(traps + tm + 11) -= 10 / (*(float*)(traps + tm + 9) * 2 - (float)*(traps + tm + 5) - (float)*(traps + tm + 3) + 105);
						else if ( *(float*)(traps + tm + 9) * 2 < (float)*(traps + tm + 3) + (float)*(traps + tm + 5) ) *(float*)(traps + tm + 11) += 10 / ((float)*(traps + tm + 3) + (float)*(traps + tm + 5) - *(float*)(traps + tm + 9) * 2 + 105);

						if (*(float*)(traps + tm + 11) > -0.3) *(float*)(traps + tm + 11) = -0.3;

						*(float*)(traps + tm + 9) += *(float*)(traps + tm + 11);

						copyrect(visualmap, redguardleft, 30, 1200, (int)*(float*)(traps + tm + 8) - 15, (int)*(float*)(traps + tm + 9) - 15, 30, 30);
					}
					else
					{
						*(float*)(traps + tm + 11) = 0;
						*(traps + tm + 7) = 1;
					}
				}

			}
			else if (*(traps + tm) == 3)
			{
				int hca = ( atan2(*(traps + tm + 3) - *(traps + tm + 5), *(traps + tm + 4) - *(traps + tm + 2)) < 0 ? 6.283 + atan2(*(traps + tm + 3) - *(traps + tm + 5), *(traps + tm + 4) - *(traps + tm + 2)) : atan2(*(traps + tm + 3) - *(traps + tm + 5), *(traps + tm + 4) - *(traps + tm + 2)) ) * 10;

				int ax = 0, ay = 0;
				while (ay < 30)
				{
					if (*(rotate30 + hca * 900 + ay * 30 + ax) != 0)
					{
						*(rtd30 + ay * 30 + ax) = *(cannon + LOWORD(*(rotate30 + hca * 900 + ay * 30 + ax)) * 30 + HIWORD(*(rotate30 + hca * 900 + ay * 30 + ax)) );
					}

					ax++;
					if (ax == 30)
					{
						ax = 0;
						ay++;
					}
				}

				copyrect(visualmap, rtd30, 30, 1200, *(traps + tm + 2) - 15, *(traps + tm + 3) - 15, 30, 30);

				if (timepos % 960 == 128)
				{
					PlaySound(shootsound, 0, SND_MEMORY | SND_ASYNC);

					int cb = 2;
					while (*(traps + cb) != 0 && *(traps + cb) != 8) cb += 12;

					*(traps + cb) = 7;

					*(float*)(traps + cb + 2) = (float)*(traps + tm + 2);
					*(float*)(traps + cb + 3) = (float)*(traps + tm + 3);
					*(float*)(traps + cb + 4) = (*(traps + tm + 4) - (float)*(traps + tm + 2)) > 0 ? sqrt( (*(traps + tm + 4) - (float)*(traps + tm + 2)) * (*(traps + tm + 4) - (float)*(traps + tm + 2)) / ((*(traps + tm + 4) - (float)*(traps + tm + 2)) * (*(traps + tm + 4) - (float)*(traps + tm + 2)) + (*(traps + tm + 5) - (float)*(traps + tm + 3)) * (*(traps + tm + 5) - (float)*(traps + tm + 3))) ) : 0 - sqrt( (*(traps + tm + 4) - (float)*(traps + tm + 2)) * (*(traps + tm + 4) - (float)*(traps + tm + 2)) / ((*(traps + tm + 4) - (float)*(traps + tm + 2)) * (*(traps + tm + 4) - (float)*(traps + tm + 2)) + (*(traps + tm + 5) - (float)*(traps + tm + 3)) * (*(traps + tm + 5) - (float)*(traps + tm + 3))) );
					*(float*)(traps + cb + 5) = (*(traps + tm + 5) - (float)*(traps + tm + 3)) > 0 ? sqrt( (*(traps + tm + 5) - (float)*(traps + tm + 3)) * (*(traps + tm + 5) - (float)*(traps + tm + 3)) / ((*(traps + tm + 4) - (float)*(traps + tm + 2)) * (*(traps + tm + 4) - (float)*(traps + tm + 2)) + (*(traps + tm + 5) - (float)*(traps + tm + 3)) * (*(traps + tm + 5) - (float)*(traps + tm + 3))) ) : 0 - sqrt( (*(traps + tm + 5) - (float)*(traps + tm + 3)) * (*(traps + tm + 5) - (float)*(traps + tm + 3)) / ((*(traps + tm + 4) - (float)*(traps + tm + 2)) * (*(traps + tm + 4) - (float)*(traps + tm + 2)) + (*(traps + tm + 5) - (float)*(traps + tm + 3)) * (*(traps + tm + 5) - (float)*(traps + tm + 3))) );

					*(float*)(traps + cb + 2) += *(float*)(traps + cb + 4) * 15;
					*(float*)(traps + cb + 3) += *(float*)(traps + cb + 5) * 15;
				}
			}
			else if (*(traps + tm) == 4)
			{
				if ((player.x - *(float*)(traps + tm + 2)) * (player.x - *(float*)(traps + tm + 2)) + (player.y - *(float*)(traps + tm + 3)) * (player.y - *(float*)(traps + tm + 3)) < 44.0)
				{
					trapdeath = 1;
					*(mem + 1) = 2;
					break;
				}

				int ifcrash = 2, ifcrashed = 0;
				while (*(traps + ifcrash) != 0)
				{
					if (*(traps + ifcrash) == 2 && fabsf(*(float*)(traps + ifcrash + 8) - *(float*)(traps + tm + 2)) < 10 && fabsf(*(float*)(traps + ifcrash + 9) - *(float*)(traps + tm + 3)) < 10)
					{
						ifcrashed = 1;

						*(float*)(traps + tm + 6) = 3 * ( (*(float*)(traps + tm + 2) - *(float*)(traps + ifcrash + 8)) > 0 ? sqrt( (*(float*)(traps + tm + 2) - *(float*)(traps + ifcrash + 8)) * (*(float*)(traps + tm + 2) - *(float*)(traps + ifcrash + 8)) / ((*(float*)(traps + tm + 2) - *(float*)(traps + ifcrash + 8)) * (*(float*)(traps + tm + 2) - *(float*)(traps + ifcrash + 8)) + (*(float*)(traps + tm + 3) - *(float*)(traps + ifcrash + 9)) * (*(float*)(traps + tm + 3) - *(float*)(traps + ifcrash + 9))) ) : 0 - sqrt( (*(float*)(traps + tm + 2) - *(float*)(traps + ifcrash + 8)) * (*(float*)(traps + tm + 2) - *(float*)(traps + ifcrash + 8)) / ((*(float*)(traps + tm + 2) - *(float*)(traps + ifcrash + 8)) * (*(float*)(traps + tm + 2) - *(float*)(traps + ifcrash + 8)) + (*(float*)(traps + tm + 3) - *(float*)(traps + ifcrash + 9)) * (*(float*)(traps + tm + 3) - *(float*)(traps + ifcrash + 9))) ) );
						*(float*)(traps + tm + 7) = 3 * ( (*(float*)(traps + tm + 3) - *(float*)(traps + ifcrash + 9)) > 0 ? sqrt( (*(float*)(traps + tm + 3) - *(float*)(traps + ifcrash + 9)) * (*(float*)(traps + tm + 3) - *(float*)(traps + ifcrash + 9)) / ((*(float*)(traps + tm + 2) - *(float*)(traps + ifcrash + 8)) * (*(float*)(traps + tm + 2) - *(float*)(traps + ifcrash + 8)) + (*(float*)(traps + tm + 3) - *(float*)(traps + ifcrash + 9)) * (*(float*)(traps + tm + 3) - *(float*)(traps + ifcrash + 9))) ) : 0 - sqrt( (*(float*)(traps + tm + 3) - *(float*)(traps + ifcrash + 9)) * (*(float*)(traps + tm + 3) - *(float*)(traps + ifcrash + 9)) / ((*(float*)(traps + tm + 2) - *(float*)(traps + ifcrash + 8)) * (*(float*)(traps + tm + 2) - *(float*)(traps + ifcrash + 8)) + (*(float*)(traps + tm + 3) - *(float*)(traps + ifcrash + 9)) * (*(float*)(traps + tm + 3) - *(float*)(traps + ifcrash + 9))) ) );

						break;
					}
					else if (*(traps + ifcrash) == 5 && fabsf(*(float*)(traps + ifcrash + 2) - *(float*)(traps + tm + 2)) < 10 && fabsf(*(float*)(traps + ifcrash + 3) - *(float*)(traps + tm + 3)) < 10)
					{
						ifcrashed = 1;

						*(float*)(traps + tm + 6) = 3 * ( (*(float*)(traps + tm + 2) - *(float*)(traps + ifcrash + 2)) > 0 ? sqrt( (*(float*)(traps + tm + 2) - *(float*)(traps + ifcrash + 2)) * (*(float*)(traps + tm + 2) - *(float*)(traps + ifcrash + 2)) / ((*(float*)(traps + tm + 2) - *(float*)(traps + ifcrash + 2)) * (*(float*)(traps + tm + 2) - *(float*)(traps + ifcrash + 2)) + (*(float*)(traps + tm + 3) - *(float*)(traps + ifcrash + 3)) * (*(float*)(traps + tm + 3) - *(float*)(traps + ifcrash + 3))) ) : 0 - sqrt( (*(float*)(traps + tm + 2) - *(float*)(traps + ifcrash + 2)) * (*(float*)(traps + tm + 2) - *(float*)(traps + ifcrash + 2)) / ((*(float*)(traps + tm + 2) - *(float*)(traps + ifcrash + 2)) * (*(float*)(traps + tm + 2) - *(float*)(traps + ifcrash + 2)) + (*(float*)(traps + tm + 3) - *(float*)(traps + ifcrash + 3)) * (*(float*)(traps + tm + 3) - *(float*)(traps + ifcrash + 3))) ) );
						*(float*)(traps + tm + 7) = 3 * ( (*(float*)(traps + tm + 3) - *(float*)(traps + ifcrash + 3)) > 0 ? sqrt( (*(float*)(traps + tm + 3) - *(float*)(traps + ifcrash + 3)) * (*(float*)(traps + tm + 3) - *(float*)(traps + ifcrash + 3)) / ((*(float*)(traps + tm + 2) - *(float*)(traps + ifcrash + 2)) * (*(float*)(traps + tm + 2) - *(float*)(traps + ifcrash + 2)) + (*(float*)(traps + tm + 3) - *(float*)(traps + ifcrash + 3)) * (*(float*)(traps + tm + 3) - *(float*)(traps + ifcrash + 3))) ) : 0 - sqrt( (*(float*)(traps + tm + 3) - *(float*)(traps + ifcrash + 3)) * (*(float*)(traps + tm + 3) - *(float*)(traps + ifcrash + 3)) / ((*(float*)(traps + tm + 2) - *(float*)(traps + ifcrash + 2)) * (*(float*)(traps + tm + 2) - *(float*)(traps + ifcrash + 2)) + (*(float*)(traps + tm + 3) - *(float*)(traps + ifcrash + 3)) * (*(float*)(traps + tm + 3) - *(float*)(traps + ifcrash + 3))) ) );

						break;
					}
					else if (*(traps + ifcrash) == 1 && fabsf((float)*(traps + ifcrash + 2) - *(float*)(traps + tm + 2)) < 10 && fabsf((float)*(traps + ifcrash + 3) - *(float*)(traps + tm + 3)) < 10)
					{
						ifcrashed = 1;

						*(float*)(traps + tm + 6) = 3 * ( (*(float*)(traps + tm + 2) - *(traps + ifcrash + 2)) > 0 ? sqrt( (*(float*)(traps + tm + 2) - *(traps + ifcrash + 2)) * (*(float*)(traps + tm + 2) - *(traps + ifcrash + 2)) / ((*(float*)(traps + tm + 2) - *(traps + ifcrash + 2)) * (*(float*)(traps + tm + 2) - *(traps + ifcrash + 2)) + (*(float*)(traps + tm + 3) - *(traps + ifcrash + 3)) * (*(float*)(traps + tm + 3) - *(traps + ifcrash + 3))) ) : 0 - sqrt( (*(float*)(traps + tm + 2) - *(traps + ifcrash + 2)) * (*(float*)(traps + tm + 2) - *(traps + ifcrash + 2)) / ((*(float*)(traps + tm + 2) - *(traps + ifcrash + 2)) * (*(float*)(traps + tm + 2) - *(traps + ifcrash + 2)) + (*(float*)(traps + tm + 3) - *(traps + ifcrash + 3)) * (*(float*)(traps + tm + 3) - *(traps + ifcrash + 3))) ) );
						*(float*)(traps + tm + 7) = 3 * ( (*(float*)(traps + tm + 3) - *(traps + ifcrash + 3)) > 0 ? sqrt( (*(float*)(traps + tm + 3) - *(traps + ifcrash + 3)) * (*(float*)(traps + tm + 3) - *(traps + ifcrash + 3)) / ((*(float*)(traps + tm + 2) - *(traps + ifcrash + 2)) * (*(float*)(traps + tm + 2) - *(traps + ifcrash + 2)) + (*(float*)(traps + tm + 3) - *(traps + ifcrash + 3)) * (*(float*)(traps + tm + 3) - *(traps + ifcrash + 3))) ) : 0 - sqrt( (*(float*)(traps + tm + 3) - *(traps + ifcrash + 3)) * (*(float*)(traps + tm + 3) - *(traps + ifcrash + 3)) / ((*(float*)(traps + tm + 2) - *(traps + ifcrash + 2)) * (*(float*)(traps + tm + 2) - *(traps + ifcrash + 2)) + (*(float*)(traps + tm + 3) - *(traps + ifcrash + 3)) * (*(float*)(traps + tm + 3) - *(traps + ifcrash + 3))) ) );

						break;
					}

					ifcrash += 12;
				}

				if (ifcrashed == 0 && timepos - *(traps + tm + 8) > 140)
				{
					*(float*)(traps + tm + 6) = 1.2 * ( (player.x - *(float*)(traps + tm + 2)) > 0 ? sqrt( (player.x - *(float*)(traps + tm + 2)) * (player.x - *(float*)(traps + tm + 2)) / ((player.x - *(float*)(traps + tm + 2)) * (player.x - *(float*)(traps + tm + 2)) + (player.y - *(float*)(traps + tm + 3)) * (player.y - *(float*)(traps + tm + 3))) ) : 0 - sqrt( (player.x - *(float*)(traps + tm + 2)) * (player.x - *(float*)(traps + tm + 2)) / ((player.x - *(float*)(traps + tm + 2)) * (player.x - *(float*)(traps + tm + 2)) + (player.y - *(float*)(traps + tm + 3)) * (player.y - *(float*)(traps + tm + 3))) ) );
					*(float*)(traps + tm + 7) = 1.2 * ( (player.y - *(float*)(traps + tm + 3)) > 0 ? sqrt( (player.y - *(float*)(traps + tm + 3)) * (player.y - *(float*)(traps + tm + 3)) / ((player.x - *(float*)(traps + tm + 2)) * (player.x - *(float*)(traps + tm + 2)) + (player.y - *(float*)(traps + tm + 3)) * (player.y - *(float*)(traps + tm + 3))) ) : 0 - sqrt( (player.y - *(float*)(traps + tm + 3)) * (player.y - *(float*)(traps + tm + 3)) / ((player.x - *(float*)(traps + tm + 2)) * (player.x - *(float*)(traps + tm + 2)) + (player.y - *(float*)(traps + tm + 3)) * (player.y - *(float*)(traps + tm + 3))) ) );
					*(traps + tm + 8) = timepos;
				}
				else if (timepos < 100) *(traps + tm + 8) = -140;

				int xfree = 0, yfree = 0;
				if (*(background + (int)*(float*)(traps + tm + 3) * 1200 + (int)(*(float*)(traps + tm + 2) + *(float*)(traps + tm + 6) + (*(float*)(traps + tm + 6) > 0 ? 3.0 : -3.0))) != 1) xfree = 1;

				if (*(background + (int)(*(float*)(traps + tm + 3) + *(float*)(traps + tm + 7) + (*(float*)(traps + tm + 7) > 0 ? 3.0 : -3.0)) * 1200 + (int)*(float*)(traps + tm + 2)) != 1) yfree = 1;

				if ((xfree == 1 && yfree == 1) || ifcrashed == 1)
				{
					*(float*)(traps + tm + 2) += *(float*)(traps + tm + 6);
					*(float*)(traps + tm + 3) += *(float*)(traps + tm + 7);
				}
				else if (xfree == 1) *(float*)(traps + tm + 2) += (*(float*)(traps + tm + 6) > 0 ? 1.1 : -1.1);
				else if (yfree == 1) *(float*)(traps + tm + 3) += (*(float*)(traps + tm + 7) > 0 ? 1.1 : -1.1);

				int hca;
				if (*(traps + tm + 4) < 0) hca = 31 + *(traps + tm + 4);
				else hca = *(traps + tm + 4);

				int ax = 0, ay = 0;
				while (ay < 18)
				{
					if (*(rotate18 + hca * 324 + ay * 18 + ax) != 0)
					{
						*(rtd18 + ay * 18 + ax) = *(waspwings + LOWORD(*(rotate18 + hca * 324 + ay * 18 + ax)) * 18 + HIWORD(*(rotate18 + hca * 324 + ay * 18 + ax)) );
					}

					ax++;
					if (ax == 18)
					{
						ax = 0;
						ay++;
					}
				}

				if (hca < 4) hca = 16 - hca;
				else hca = hca - 12;

				ax = 0, ay = 0;
				while (ay < 18)
				{
					if (*(rotate18 + hca * 324 + ay * 18 + ax) != 0 && *(rtd18 + ay * 18 + ax) == 0)
					{
						*(rtd18 + ay * 18 + ax) = *(waspwings + LOWORD(*(rotate18 + hca * 324 + ay * 18 + ax)) * 18 + HIWORD(*(rotate18 + hca * 324 + ay * 18 + ax)) );
					}

					ax++;
					if (ax == 18)
					{
						ax = 0;
						ay++;
					}
				}

				copyrect(rtd18, wasp, 18, 18, 0, 0, 18, 18);

				copyrect(visualmap, rtd18, 18, 1200, (int)*(float*)(traps + tm + 2) - 9, (int)*(float*)(traps + tm + 3) - 9, 18, 18);

				if (*(traps + tm + 4) < 3) *(traps + tm + 4) += 1;
				else *(traps + tm + 4) = -3;
			}
			else if (*(traps + tm) == 5)
			{
				if ((player.x - *(float*)(traps + tm + 2)) * (player.x - *(float*)(traps + tm + 2)) + (player.y - *(float*)(traps + tm + 3)) * (player.y - *(float*)(traps + tm + 3)) < 260.0)
				{
					trapdeath = 1;
					*(mem + 1) = 2;
					break;
				}

				*(float*)(traps + tm + 2) += ((float)*(traps + tm + 5) - *(float*)(traps + tm + 3)) * 0.035;
				*(float*)(traps + tm + 3) += (*(float*)(traps + tm + 2) - (float)*(traps + tm + 4)) * 0.035;

				copyrect(visualmap, spinner, 30, 1200, (int)*(float*)(traps + tm + 2) - 15, (int)*(float*)(traps + tm + 3) - 15, 30, 30);
			}
			else if (*(traps + tm) == 6)
			{
				int hca = ( atan2(*(traps + tm + 3) - player.y, player.x - *(traps + tm + 2)) < 0 ? 6.283 + atan2(*(traps + tm + 3) - player.y, player.x - *(traps + tm + 2)) : atan2(*(traps + tm + 3) - player.y, player.x - *(traps + tm + 2)) ) * 10;

				int ax = 0, ay = 0;
				while (ay < 30)
				{
					if (*(rotate30 + hca * 900 + ay * 30 + ax) != 0)
					{
						*(rtd30 + ay * 30 + ax) = *(homingcannon + LOWORD(*(rotate30 + hca * 900 + ay * 30 + ax)) * 30 + HIWORD(*(rotate30 + hca * 900 + ay * 30 + ax)) );
					}

					ax++;
					if (ax == 30)
					{
						ax = 0;
						ay++;
					}
				}

				copyrect(visualmap, rtd30, 30, 1200, *(traps + tm + 2) - 15, *(traps + tm + 3) - 15, 30, 30);

				if (timepos % 960 == 128)
				{
					PlaySound(shootsound, 0, SND_MEMORY | SND_ASYNC);

					int cb = 2;
					while (*(traps + cb) != 0 && *(traps + cb) != 8) cb += 12;

					*(traps + cb) = 7;

					*(float*)(traps + cb + 2) = (float)*(traps + tm + 2);
					*(float*)(traps + cb + 3) = (float)*(traps + tm + 3);
					*(float*)(traps + cb + 4) = (player.x - (float)*(traps + tm + 2)) > 0 ? sqrt( (player.x - (float)*(traps + tm + 2)) * (player.x - (float)*(traps + tm + 2)) / ((player.x - (float)*(traps + tm + 2)) * (player.x - (float)*(traps + tm + 2)) + (player.y - (float)*(traps + tm + 3)) * (player.y - (float)*(traps + tm + 3))) ) : 0 - sqrt( (player.x - (float)*(traps + tm + 2)) * (player.x - (float)*(traps + tm + 2)) / ((player.x - (float)*(traps + tm + 2)) * (player.x - (float)*(traps + tm + 2)) + (player.y - (float)*(traps + tm + 3)) * (player.y - (float)*(traps + tm + 3))) );
					*(float*)(traps + cb + 5) = (player.y - (float)*(traps + tm + 3)) > 0 ? sqrt( (player.y - (float)*(traps + tm + 3)) * (player.y - (float)*(traps + tm + 3)) / ((player.x - (float)*(traps + tm + 2)) * (player.x - (float)*(traps + tm + 2)) + (player.y - (float)*(traps + tm + 3)) * (player.y - (float)*(traps + tm + 3))) ) : 0 - sqrt( (player.y - (float)*(traps + tm + 3)) * (player.y - (float)*(traps + tm + 3)) / ((player.x - (float)*(traps + tm + 2)) * (player.x - (float)*(traps + tm + 2)) + (player.y - (float)*(traps + tm + 3)) * (player.y - (float)*(traps + tm + 3))) );

					*(float*)(traps + cb + 2) += *(float*)(traps + cb + 4) * 15;
					*(float*)(traps + cb + 3) += *(float*)(traps + cb + 5) * 15;
				}
			}
			else if (*(traps + tm) == 7)
			{
				if ((player.x - *(float*)(traps + tm + 2)) * (player.x - *(float*)(traps + tm + 2)) + (player.y - *(float*)(traps + tm + 3)) * (player.y - *(float*)(traps + tm + 3)) < 84.0)
				{
					trapdeath = 1;
					*(mem + 1) = 2;
					break;
				}

				*(float*)(traps + tm + 2) += *(float*)(traps + tm + 4) * 1.5;
				*(float*)(traps + tm + 3) += *(float*)(traps + tm + 5) * 1.5;

				int aifground = *(background + (int)*(float*)(traps + tm + 3) * 1200 + (int)*(float*)(traps + tm + 2));
				if (aifground > 0 && (aifground == 1 || aifground == 2 || aifground == 4))
				{
					PlaySound(bulletsound, 0, SND_MEMORY | SND_ASYNC);

					ZeroMemory(traps + tm, 12 * 4);
					*(traps + tm) = 8;
				}
				else copyrect(visualmap, bullet, 9, 1200, (int)*(float*)(traps + tm + 2) - 4, (int)*(float*)(traps + tm + 3) - 4, 9, 9);
			}
			else if (*(traps + tm) == 9)
			{
				if ((player.x - (float)*(traps + tm + 2)) * (player.x - (float)*(traps + tm + 2)) + (player.y - (float)*(traps + tm + 3)) * (player.y - (float)*(traps + tm + 3)) < 698.0)
				{
					gravity *= -1;

					ZeroMemory(traps + tm, 48);
					*(traps + tm) = 8;
				}

				if (gravity == 1) copyrect(visualmap, gravity1, 50, 1200, *(traps + tm + 2) - 25, *(traps + tm + 3) - 25, 50, 50);
				else copyrect(visualmap, gravity2, 50, 1200, *(traps + tm + 2) - 25, *(traps + tm + 3) - 25, 50, 50);
			}
			else if (*(traps + tm) == 10)
			{
				if (*(float*)(traps + tm + 8) == 0)
				{
					if (*(traps + tm + 4) < *(traps + tm + 6)) *(float*)(traps + tm + 8) = 0.526;
					else if (*(traps + tm + 4) > *(traps + tm + 6)) *(float*)(traps + tm + 8) = -0.526;
				}
				if (*(float*)(traps + tm + 9) == 0)
				{
					if (*(traps + tm + 5) < *(traps + tm + 7)) *(float*)(traps + tm + 9) = 0.526;
					else if (*(traps + tm + 5) > *(traps + tm + 7)) *(float*)(traps + tm + 9) = -0.526;
				}

				if (timepos % 456 == 8)
				{
					PlaySound(shootsound, 0, SND_MEMORY | SND_ASYNC);

					int cb = 2;
					while (*(traps + cb) != 0 && *(traps + cb) != 8) cb += 12;

					*(traps + cb) = 7;

					*(float*)(traps + cb + 2) = *(float*)(traps + tm + 2);
					*(float*)(traps + cb + 3) = *(float*)(traps + tm + 3);
					if (*(traps + tm + 10) == 0)
					{
						*(float*)(traps + cb + 4) = 0.0;
						*(float*)(traps + cb + 5) = 1.0;
					}
					else if (*(traps + tm + 10) == 1)
					{
						*(float*)(traps + cb + 4) = -1.0;
						*(float*)(traps + cb + 5) = 0.0;
					}
					else if (*(traps + tm + 10) == 2)
					{
						*(float*)(traps + cb + 4) = 0.0;
						*(float*)(traps + cb + 5) = -1.0;
					}
					else if (*(traps + tm + 10) == 3)
					{
						*(float*)(traps + cb + 4) = 1.0;
						*(float*)(traps + cb + 5) = 0.0;
					}

					*(float*)(traps + cb + 2) += *(float*)(traps + cb + 4) * 15;
					*(float*)(traps + cb + 3) += *(float*)(traps + cb + 5) * 15;
				}

				*(float*)(traps + tm + 2) += *(float*)(traps + tm + 8);
				*(float*)(traps + tm + 3) += *(float*)(traps + tm + 9);

				if ((*(float*)(traps + tm + 2) < (float)*(traps + tm + 4) && *(float*)(traps + tm + 2) < (float)*(traps + tm + 6)) || (*(float*)(traps + tm + 2) > (float)*(traps + tm + 4) && *(float*)(traps + tm + 2) > (float)*(traps + tm + 6)))
				{
					*(float*)(traps + tm + 8) *= -1;
					*(float*)(traps + tm + 2) += 2 * *(float*)(traps + tm + 8);
				}
				if ((*(float*)(traps + tm + 3) < (float)*(traps + tm + 5) && *(float*)(traps + tm + 3) < (float)*(traps + tm + 7)) || (*(float*)(traps + tm + 3) > (float)*(traps + tm + 5) && *(float*)(traps + tm + 3) > (float)*(traps + tm + 7)))
				{
					*(float*)(traps + tm + 9) *= -1;
					*(float*)(traps + tm + 3) += 2 * *(float*)(traps + tm + 9);
				}

				int hca = (int)(62.8 - 15.7 * (float)*(traps + tm + 10));

				int ax = 0, ay = 0;
				while (ay < 30)
				{
					if (*(rotate30 + hca * 900 + ay * 30 + ax) != 0)
					{
						*(rtd30 + ay * 30 + ax) = *(warder + LOWORD(*(rotate30 + hca * 900 + ay * 30 + ax)) * 30 + HIWORD(*(rotate30 + hca * 900 + ay * 30 + ax)) );
					}

					ax++;
					if (ax == 30)
					{
						ax = 0;
						ay++;
					}
				}

				copyrect(visualmap, rtd30, 30, 1200, (int)*(float*)(traps + tm + 2) - 15, (int)*(float*)(traps + tm + 3) - 15, 30, 30);
			}
			else if (*(traps + tm) == 11)
			{
				int hca = ( atan2(*(traps + tm + 3) - player.y, player.x - *(traps + tm + 2)) < 0 ? 6.283 + atan2(*(traps + tm + 3) - player.y, player.x - *(traps + tm + 2)) : atan2(*(traps + tm + 3) - player.y, player.x - *(traps + tm + 2)) ) * 10;

				int ax = 0, ay = 0;
				while (ay < 30)
				{
					if (*(rotate30 + hca * 900 + ay * 30 + ax) != 0)
					{
						*(rtd30 + ay * 30 + ax) = *(sniper + LOWORD(*(rotate30 + hca * 900 + ay * 30 + ax)) * 30 + HIWORD(*(rotate30 + hca * 900 + ay * 30 + ax)) );
					}

					ax++;
					if (ax == 30)
					{
						ax = 0;
						ay++;
					}
				}

				copyrect(visualmap, rtd30, 30, 1200, *(traps + tm + 2) - 15, *(traps + tm + 3) - 15, 30, 30);

				if (timepos % 960 == 8)
				{
					PlaySound(shootsound, 0, SND_MEMORY | SND_ASYNC);

					int cb = 2;
					while (*(traps + cb) != 0 && *(traps + cb) != 8) cb += 12;

					*(traps + cb) = 7;

					*(float*)(traps + cb + 2) = (float)*(traps + tm + 2);
					*(float*)(traps + cb + 3) = (float)*(traps + tm + 3);
					*(float*)(traps + cb + 4) = 7.5 * (player.x - (float)*(traps + tm + 2)) * fabsf(player.x - (float)*(traps + tm + 2)) / ( (player.x - (float)*(traps + tm + 2)) * (player.x - (float)*(traps + tm + 2)) + (player.y - (float)*(traps + tm + 3)) * (player.y - (float)*(traps + tm + 3)) );
					*(float*)(traps + cb + 5) = 7.5 * (player.y - (float)*(traps + tm + 3)) * fabsf(player.y - (float)*(traps + tm + 3)) / ( (player.x - (float)*(traps + tm + 2)) * (player.x - (float)*(traps + tm + 2)) + (player.y - (float)*(traps + tm + 3)) * (player.y - (float)*(traps + tm + 3)) );

					*(float*)(traps + cb + 2) += *(float*)(traps + cb + 4) * 3;
					*(float*)(traps + cb + 3) += *(float*)(traps + cb + 5) * 3;
				}
			}
			else if (*(traps + tm) == 12)
			{
				if (*(traps + tm + 4) == 1) copyrect(visualmap, platform, 60, 1200, *(traps + tm + 2), *(traps + tm + 3), 60, 6);
				else if (*(traps + tm + 4) == 2) copyrect(visualmap, jumppad, 60, 1200, *(traps + tm + 2), *(traps + tm + 3), 60, 6);
			}

			tm += 12;
		}

		if (*(mem + 1) == 3)
		{
			while (*(traps + tm) != 0 && *(mem + 1) == 3)
			{
				if (*(traps + tm) == 1) copyrect(visualmap, saw, 30, 1200, *(traps + tm + 2) - 15, *(traps + tm + 3) - 15, 30, 30);
				else if (*(traps + tm) == 2)
				{
					drawline(visualmap, 1200, (int)*(float*)(traps + tm + 8), (int)*(float*)(traps + tm + 9), (*(traps + tm + 4) == (int)*(float*)(traps + tm + 8) ? *(traps + tm + 2) : *(traps + tm + 4)), (*(traps + tm + 5) == (int)*(float*)(traps + tm + 9) ? *(traps + tm + 3) : *(traps + tm + 5)), 0xFFFF00);
					copyrect(visualmap, guide, 30, 1200, (*(traps + tm + 4) == (int)*(float*)(traps + tm + 8) ? *(traps + tm + 2) : *(traps + tm + 4)) - 15, (*(traps + tm + 5) == (int)*(float*)(traps + tm + 9) ? *(traps + tm + 3) : *(traps + tm + 5)) - 15, 30, 30);
					copyrect(visualmap, redguardright, 30, 1200, (int)*(float*)(traps + tm + 8) - 15, (int)*(float*)(traps + tm + 9) - 15, 30, 30);
				}
				else if (*(traps + tm) == 3)
				{
					int hca = ( atan2(*(traps + tm + 3) - *(traps + tm + 5), *(traps + tm + 4) - *(traps + tm + 2)) < 0 ? 6.283 + atan2(*(traps + tm + 3) - *(traps + tm + 5), *(traps + tm + 4) - *(traps + tm + 2)) : atan2(*(traps + tm + 3) - *(traps + tm + 5), *(traps + tm + 4) - *(traps + tm + 2)) ) * 10;

					int ax = 0, ay = 0;
					while (ay < 30)
					{
						if (*(rotate30 + hca * 900 + ay * 30 + ax) != 0)
						{
							*(rtd30 + ay * 30 + ax) = *(cannon + LOWORD(*(rotate30 + hca * 900 + ay * 30 + ax)) * 30 + HIWORD(*(rotate30 + hca * 900 + ay * 30 + ax)) );
						}

						ax++;
						if (ax == 30)
						{
							ax = 0;
							ay++;
						}
					}

					copyrect(visualmap, rtd30, 30, 1200, *(traps + tm + 2) - 15, *(traps + tm + 3) - 15, 30, 30);
				}
				else if (*(traps + tm) == 4) copyrect(visualmap, wasp, 18, 1200, (int)*(float*)(traps + tm + 2) - 9, (int)*(float*)(traps + tm + 3) - 9, 18, 18);
				else if (*(traps + tm) == 5)
				{
					drawline(visualmap, 1200, (int)*(float*)(traps + tm + 2), (int)*(float*)(traps + tm + 3), *(traps + tm + 4), *(traps + tm + 5), 0xFFFF00);
					copyrect(visualmap, guide, 30, 1200, *(traps + tm + 4) - 15, *(traps + tm + 5) - 15, 30, 30);
					copyrect(visualmap, spinner, 30, 1200, (int)*(float*)(traps + tm + 2) - 15, (int)*(float*)(traps + tm + 3) - 15, 30, 30);
				}
				else if (*(traps + tm) == 6) copyrect(visualmap, homingcannon, 30, 1200, *(traps + tm + 2) - 15, *(traps + tm + 3) - 15, 30, 30);
				else if (*(traps + tm) == 9) copyrect(visualmap, gravity1, 50, 1200, *(traps + tm + 2) - 25, *(traps + tm + 3) - 25, 50, 50);
				else if (*(traps + tm) == 10)
				{
					drawline(visualmap, 1200, (int)*(float*)(traps + tm + 2), (int)*(float*)(traps + tm + 3), (int)*(float*)(traps + tm + 2) == *(traps + tm + 4) ? *(traps + tm + 6) : *(traps + tm + 4), (int)*(float*)(traps + tm + 3) == *(traps + tm + 5) ? *(traps + tm + 7) : *(traps + tm + 4), 0xFFFF00);
					copyrect(visualmap, guide, 30, 1200, ((int)*(float*)(traps + tm + 2) == *(traps + tm + 4) ? *(traps + tm + 6) : *(traps + tm + 4)) - 15, ((int)*(float*)(traps + tm + 3) == *(traps + tm + 5) ? *(traps + tm + 7) : *(traps + tm + 4)) - 15, 30, 30);

					int hca = (int)(62.8 - 15.7 * (float)*(traps + tm + 10));

					int ax = 0, ay = 0;
					while (ay < 30)
					{
						if (*(rotate30 + hca * 900 + ay * 30 + ax) != 0)
						{
							*(rtd30 + ay * 30 + ax) = *(warder + LOWORD(*(rotate30 + hca * 900 + ay * 30 + ax)) * 30 + HIWORD(*(rotate30 + hca * 900 + ay * 30 + ax)) );
						}

						ax++;
						if (ax == 30)
						{
							ax = 0;
							ay++;
						}
					}

					copyrect(visualmap, rtd30, 30, 1200, (int)*(float*)(traps + tm + 2) - 15, (int)*(float*)(traps + tm + 3) - 15, 30, 30);
				}
				else if (*(traps + tm) == 11) copyrect(visualmap, sniper, 30, 1200, *(traps + tm + 2) - 15, *(traps + tm + 3) - 15, 30, 30);
				else if (*(traps + tm) == 12)
				{
					if (*(traps + tm + 4) == 1) copyrect(visualmap, platform, 60, 1200, *(traps + tm + 2), *(traps + tm + 3), 60, 6);
					else if (*(traps + tm + 4) == 2) copyrect(visualmap, jumppad, 60, 1200, *(traps + tm + 2), *(traps + tm + 3), 60, 6);
				}

				tm += 12;
			}

			if (GetAsyncKeyState(VK_LBUTTON) != 0)
			{
				if (mp.x > 120 && mp.x < 240 && mp.y > 10 && mp.y < 50)
				{
					CopyMemory(trapssp, traps, 16376 * 4);
					deaths = 0;

					drawrect(visualbackground, 1200, 0, 0, 1200, 600, 0x383838);
					drawwalls();

					int bgextrasplacer = 2;
					while (*(traps + bgextrasplacer) != 0)
					{
						if (*(traps + bgextrasplacer) == 12)
						{
							if (*(traps + bgextrasplacer + 4) == 1)
							{
								replacerect(background, 1200, *(traps + bgextrasplacer + 2) - 5, *(traps + bgextrasplacer + 3), 5, 6, 3, 0);
								replacerect(background, 1200, *(traps + bgextrasplacer + 2), *(traps + bgextrasplacer + 3), 60, 6, 2, 0);
								replacerect(background, 1200, *(traps + bgextrasplacer + 2) + 60, *(traps + bgextrasplacer + 3), 5, 6, 3, 0);
							}
							else if (*(traps + bgextrasplacer + 4) == 2)
							{
								replacerect(background, 1200, *(traps + bgextrasplacer + 2) - 5, *(traps + bgextrasplacer + 3), 5, 6, 5, 0);
								replacerect(background, 1200, *(traps + bgextrasplacer + 2), *(traps + bgextrasplacer + 3), 60, 6, 4, 0);
								replacerect(background, 1200, *(traps + bgextrasplacer + 2) + 60, *(traps + bgextrasplacer + 3), 5, 6, 5, 0);
							}
						}

						bgextrasplacer += 12;
					}

					while (GetAsyncKeyState(VK_LBUTTON) != 0) processmsg();

					retrygame(trapssp);
					goto startloop;
				}
				else if (mp.x > 300 && mp.x < 420 && mp.y > 10 && mp.y < 50)
				{
					while (GetAsyncKeyState(VK_LBUTTON) != 0) processmsg();

					*(mem + 1) = 4;
					goto startloop;
				}
				else if (mp.x > 480 && mp.x < 600 && mp.y > 10 && mp.y < 50)
				{
					deaths = 0;

					drawrect(visualbackground, 1200, 0, 0, 1200, 600, 0x383838);

					int ax = 0, ay = 0;
					while (ay < 600)
					{
						if (ax % 60 == 59 || ax % 60 == 0 || ay % 60 == 59 || ay % 60 == 0) *(visualbackground + ay * 1200 + ax) = 0x009900;

						if (*(background + ay * 1200 + ax) == 1) *(visualbackground + ay * 1200 + ax) = 0x909090;

						ax++;
						if (ax == 1200)
						{
							ax = 0;
							ay++;
						}
					}

					while (GetAsyncKeyState(VK_LBUTTON) != 0) processmsg();

					*(mem + 1) = 5;
					goto startloop;
				}
				else if (mp.x > 15 && mp.x < 45 && mp.y > 495 && mp.y < 525)
				{
					CopyMemory(traps, trapssp, 16376 * 4);

					while (GetAsyncKeyState(VK_LBUTTON) != 0) processmsg();

					retrygame(trapssp);
					goto startloop;
				}

				if (editorpx != (mp.x - mp.x % 30) || editorpy != (mp.y - mp.y % 30))
				{
					int teditor = 2;
					while (*(traps + teditor) != 0)
					{
						if (*(traps + teditor) == 1 && *(traps + teditor + 2) == editorpx + 15 && *(traps + teditor + 3) == editorpy + 15)
						{
							*(traps + teditor + 2) = (mp.x - mp.x % 30) + 15;
							*(traps + teditor + 3) = (mp.y - mp.y % 30) + 15;

							if (mp.x > 1080 && mp.x < 1140 && mp.y > 0 && mp.y < 60)
							{
								ZeroMemory(traps + teditor, 48);
								*(traps + teditor) = 8;
							}

							break;
						}
						else if (*(traps + teditor) == 2 && (int)*(float*)(traps + teditor + 8) == editorpx + 15 && (int)*(float*)(traps + teditor + 9) == editorpy + 15)
						{
							if ( (mp.x - mp.x % 30) + 15 < (*(traps + teditor + 4) == (int)*(float*)(traps + teditor + 8) ? *(traps + teditor + 2) : *(traps + teditor + 4)) )
							{
								if ((int)*(float*)(traps + teditor + 8) == *(traps + teditor + 2))
								{
									*(traps + teditor + 2) = (mp.x - mp.x % 30) + 15;
									*(float*)(traps + teditor + 8) = (float)*(traps + teditor + 2);
								}
								else
								{
									*(traps + teditor + 4) = *(traps + teditor + 2);
									*(traps + teditor + 2) = (mp.x - mp.x % 30) + 15;
									*(float*)(traps + teditor + 8) = (float)*(traps + teditor + 2);
								}

								*(traps + teditor + 6) = 1;
							}
							else if ( (mp.x - mp.x % 30) + 15 > (*(traps + teditor + 4) == (int)*(float*)(traps + teditor + 8) ? *(traps + teditor + 2) : *(traps + teditor + 4)) )
							{
								if ((int)*(float*)(traps + teditor + 8) == *(traps + teditor + 4))
								{
									*(traps + teditor + 4) = (mp.x - mp.x % 30) + 15;
									*(float*)(traps + teditor + 8) = (float)*(traps + teditor + 4);
								}
								else
								{
									*(traps + teditor + 2) = *(traps + teditor + 4);
									*(traps + teditor + 4) = (mp.x - mp.x % 30) + 15;
									*(float*)(traps + teditor + 8) = (float)*(traps + teditor + 4);
								}

								*(traps + teditor + 6) = 2;
							}
							else
							{
								*(traps + teditor + 4) = *(traps + teditor + 2) = (mp.x - mp.x % 30) + 15;
								*(float*)(traps + teditor + 8) = (float)*(traps + teditor + 2);

								*(traps + teditor + 6) = 0;
							}

							if ( (mp.y - mp.y % 30) + 15 < (*(traps + teditor + 5) == (int)*(float*)(traps + teditor + 9) ? *(traps + teditor + 3) : *(traps + teditor + 5)) )
							{
								if ((int)*(float*)(traps + teditor + 9) == *(traps + teditor + 3))
								{
									*(traps + teditor + 3) = (mp.y - mp.y % 30) + 15;
									*(float*)(traps + teditor + 9) = (float)*(traps + teditor + 3);
								}
								else
								{
									*(traps + teditor + 5) = *(traps + teditor + 3);
									*(traps + teditor + 3) = (mp.y - mp.y % 30) + 15;
									*(float*)(traps + teditor + 9) = (float)*(traps + teditor + 3);
								}

								*(traps + teditor + 7) = 1;
							}
							else if ( (mp.y - mp.y % 30) + 15 > (*(traps + teditor + 5) == (int)*(float*)(traps + teditor + 9) ? *(traps + teditor + 3) : *(traps + teditor + 5)) )
							{
								if ((int)*(float*)(traps + teditor + 9) == *(traps + teditor + 5))
								{
									*(traps + teditor + 5) = (mp.y - mp.y % 30) + 15;
									*(float*)(traps + teditor + 9) = (float)*(traps + teditor + 5);
								}
								else
								{
									*(traps + teditor + 3) = *(traps + teditor + 5);
									*(traps + teditor + 5) = (mp.y - mp.y % 30) + 15;
									*(float*)(traps + teditor + 9) = (float)*(traps + teditor + 5);
								}

								*(traps + teditor + 7) = 2;
							}
							else
							{
								*(traps + teditor + 5) = *(traps + teditor + 3) = (mp.y - mp.y % 30) + 15;
								*(float*)(traps + teditor + 9) = (float)*(traps + teditor + 3);

								*(traps + teditor + 7) = 0;
							}


							if (mp.x > 1080 && mp.x < 1140 && mp.y > 0 && mp.y < 60)
							{
								ZeroMemory(traps + teditor, 48);
								*(traps + teditor) = 8;
							}

							break;
						}
						else if (*(traps + teditor) == 2 && (*(traps + teditor + 4) == (int)*(float*)(traps + teditor + 8) ? *(traps + teditor + 2) : *(traps + teditor + 4)) == editorpx + 15 && (*(traps + teditor + 5) == (int)*(float*)(traps + teditor + 9) ? *(traps + teditor + 3) : *(traps + teditor + 5)) == editorpy + 15)
						{
							if ( (mp.x - mp.x % 30) + 15 > (int)*(float*)(traps + teditor + 8) )
							{
								if ((int)*(float*)(traps + teditor + 8) == *(traps + teditor + 2))
								{
									*(traps + teditor + 4) = (mp.x - mp.x % 30) + 15;
								}
								else
								{
									*(traps + teditor + 2) = *(traps + teditor + 4);
									*(traps + teditor + 4) = (mp.x - mp.x % 30) + 15;
								}

								*(traps + teditor + 6) = 1;
							}
							else if ( (mp.x - mp.x % 30) + 15 < (int)*(float*)(traps + teditor + 8) )
							{
								if ((int)*(float*)(traps + teditor + 8) == *(traps + teditor + 4))
								{
									*(traps + teditor + 2) = (mp.x - mp.x % 30) + 15;
								}
								else
								{
									*(traps + teditor + 4) = *(traps + teditor + 2);
									*(traps + teditor + 2) = (mp.x - mp.x % 30) + 15;
								}

								*(traps + teditor + 6) = 2;
							}
							else
							{
								*(traps + teditor + 4) = *(traps + teditor + 2) = (mp.x - mp.x % 30) + 15;
								*(float*)(traps + teditor + 8) = (float)*(traps + teditor + 2);

								*(traps + teditor + 6) = 0;
							}

							if ( (mp.y - mp.y % 30) + 15 > (int)*(float*)(traps + teditor + 9) )
							{
								if ((int)*(float*)(traps + teditor + 9) == *(traps + teditor + 3))
								{
									*(traps + teditor + 5) = (mp.y - mp.y % 30) + 15;
								}
								else
								{
									*(traps + teditor + 3) = *(traps + teditor + 5);
									*(traps + teditor + 5) = (mp.y - mp.y % 30) + 15;
								}

								*(traps + teditor + 7) = 1;
							}
							else if ( (mp.y - mp.y % 30) + 15 < (int)*(float*)(traps + teditor + 9) )
							{
								if ((int)*(float*)(traps + teditor + 9) == *(traps + teditor + 5))
								{
									*(traps + teditor + 3) = (mp.y - mp.y % 30) + 15;
								}
								else
								{
									*(traps + teditor + 5) = *(traps + teditor + 3);
									*(traps + teditor + 3) = (mp.y - mp.y % 30) + 15;
								}

								*(traps + teditor + 7) = 2;
							}
							else
							{
								*(traps + teditor + 5) = *(traps + teditor + 3) = (mp.y - mp.y % 30) + 15;
								*(float*)(traps + teditor + 9) = (float)*(traps + teditor + 3);

								*(traps + teditor + 7) = 0;
							}

							break;
						}
						else if (*(traps + teditor) == 3 && *(traps + teditor + 2) == editorpx + 15 && *(traps + teditor + 3) == editorpy + 15)
						{
							*(traps + teditor + 4) += (mp.x - mp.x % 30) + 15 - *(traps + teditor + 2);
							*(traps + teditor + 5) += (mp.y - mp.y % 30) + 15 - *(traps + teditor + 3);

							*(traps + teditor + 2) = (mp.x - mp.x % 30) + 15;
							*(traps + teditor + 3) = (mp.y - mp.y % 30) + 15;

							if (mp.x > 1080 && mp.x < 1140 && mp.y > 0 && mp.y < 60)
							{
								ZeroMemory(traps + teditor, 48);
								*(traps + teditor) = 8;
							}

							break;
						}
						else if ((*(traps + teditor) == 4 || *(traps + teditor) == 5) && (int)*(float*)(traps + teditor + 2) == editorpx + 15 && (int)*(float*)(traps + teditor + 3) == editorpy + 15)
						{
							*(float*)(traps + teditor + 2) = (float)((mp.x - mp.x % 30) + 15);
							*(float*)(traps + teditor + 3) = (float)((mp.y - mp.y % 30) + 15);

							if (mp.x > 1080 && mp.x < 1140 && mp.y > 0 && mp.y < 60)
							{
								ZeroMemory(traps + teditor, 48);
								*(traps + teditor) = 8;
							}

							break;
						}
						else if (*(traps + teditor) == 5 && *(traps + teditor + 4) == editorpx + 15 && *(traps + teditor + 5) == editorpy + 15)
						{
							*(traps + teditor + 4) = (mp.x - mp.x % 30) + 15;
							*(traps + teditor + 5) = (mp.y - mp.y % 30) + 15;

							break;
						}
						else if (*(traps + teditor) == 6 && *(traps + teditor + 2) == editorpx + 15 && *(traps + teditor + 3) == editorpy + 15)
						{
							*(traps + teditor + 2) = (mp.x - mp.x % 30) + 15;
							*(traps + teditor + 3) = (mp.y - mp.y % 30) + 15;

							if (mp.x > 1080 && mp.x < 1140 && mp.y > 0 && mp.y < 60)
							{
								ZeroMemory(traps + teditor, 48);
								*(traps + teditor) = 8;
							}

							break;
						}
						else if (*(traps + teditor) == 9 && (*(traps + teditor + 2) == editorpx || *(traps + teditor + 2) == editorpx + 30) && (*(traps + teditor + 3) == editorpy || *(traps + teditor + 3) == editorpy + 30) )
						{
							if (mp.x > 1080 && mp.x < 1140 && mp.y > 0 && mp.y < 60)
							{
								ZeroMemory(traps + teditor, 48);
								*(traps + teditor) = 8;
							}

							*(traps + teditor + 2) += (mp.x - mp.x % 30) - editorpx;
							*(traps + teditor + 3) += (mp.y - mp.y % 30) - editorpy;

							break;
						}
						else if (*(traps + teditor) == 10 && (int)*(float*)(traps + teditor + 2) == editorpx + 15 && (int)*(float*)(traps + teditor + 3) == editorpy + 15)
						{
							if ((int)*(float*)(traps + teditor + 2) == *(traps + teditor + 4))
							{
								*(traps + teditor + 4) = (mp.x - mp.x % 30) + 15;
								*(float*)(traps + teditor + 2) = (float)((mp.x - mp.x % 30) + 15);
							}
							else if ((int)*(float*)(traps + teditor + 2) == *(traps + teditor + 6))
							{
								*(traps + teditor + 6) = (mp.x - mp.x % 30) + 15;
								*(float*)(traps + teditor + 2) = (float)((mp.x - mp.x % 30) + 15);
							}

							if ((int)*(float*)(traps + teditor + 3) == *(traps + teditor + 5))
							{
								*(traps + teditor + 5) = (mp.y - mp.y % 30) + 15;
								*(float*)(traps + teditor + 3) = (float)((mp.y - mp.y % 30) + 15);
							}
							else if ((int)*(float*)(traps + teditor + 3) == *(traps + teditor + 7))
							{
								*(traps + teditor + 7) = (mp.y - mp.y % 30) + 15;
								*(float*)(traps + teditor + 3) = (float)((mp.y - mp.y % 30) + 15);
							}

							if (mp.x > 1080 && mp.x < 1140 && mp.y > 0 && mp.y < 60)
							{
								ZeroMemory(traps + teditor, 48);
								*(traps + teditor) = 8;
							}

							break;
						}
						else if (*(traps + teditor) == 10 && ((int)*(float*)(traps + teditor + 2) == *(traps + teditor + 4) ? *(traps + teditor + 6) : *(traps + teditor + 4)) == editorpx + 15 && ((int)*(float*)(traps + teditor + 3) == *(traps + teditor + 5) ? *(traps + teditor + 7) : *(traps + teditor + 5)) == editorpy + 15)
						{
							if ((int)*(float*)(traps + teditor + 2) == *(traps + teditor + 4))
							{
								*(traps + teditor + 6) = (mp.x - mp.x % 30) + 15;
							}
							else if ((int)*(float*)(traps + teditor + 2) == *(traps + teditor + 6))
							{
								*(traps + teditor + 4) = (mp.x - mp.x % 30) + 15;
							}

							if ((int)*(float*)(traps + teditor + 3) == *(traps + teditor + 5))
							{
								*(traps + teditor + 7) = (mp.y - mp.y % 30) + 15;
							}
							else if ((int)*(float*)(traps + teditor + 3) == *(traps + teditor + 7))
							{
								*(traps + teditor + 5) = (mp.y - mp.y % 30) + 15;
							}

							break;
						}
						else if (*(traps + teditor) == 11 && *(traps + teditor + 2) == editorpx + 15 && *(traps + teditor + 3) == editorpy + 15)
						{
							*(traps + teditor + 2) = (mp.x - mp.x % 30) + 15;
							*(traps + teditor + 3) = (mp.y - mp.y % 30) + 15;

							if (mp.x > 1080 && mp.x < 1140 && mp.y > 0 && mp.y < 60)
							{
								ZeroMemory(traps + teditor, 48);
								*(traps + teditor) = 8;
							}

							break;
						}
						else if (*(traps + teditor) == 12 && (*(traps + teditor + 2) == editorpx || *(traps + teditor + 2) == editorpx - 30) && *(traps + teditor + 3) == editorpy)
						{
							*(traps + teditor + 2) += (mp.x - mp.x % 30) - editorpx;
							*(traps + teditor + 3) += (mp.y - mp.y % 30) - editorpy;

							if (mp.x > 1080 && mp.x < 1140 && mp.y > 0 && mp.y < 60)
							{
								ZeroMemory(traps + teditor, 48);
								*(traps + teditor) = 8;
							}

							break;
						}
						else if ((*(traps + teditor) == 33 || *(traps + teditor) == 34) && (*(traps + teditor + 1) == editorpx || *(traps + teditor + 1) == editorpx + 30) && (*(traps + teditor + 2) == editorpy + 30 || *(traps + teditor + 2) == editorpy + 60) )
						{
							*(traps + teditor + 1) += (mp.x - mp.x % 30) - editorpx;
							*(traps + teditor + 2) += (mp.y - mp.y % 30) - editorpy;

							break;
						}

						teditor += 12;
					}

					editorpx = (mp.x - mp.x % 30);
					editorpy = (mp.y - mp.y % 30);
				}

			}
			else
			{
				if (editorpx != 0 && editorpy != 0 && beforeclickx == editorpx && beforeclicky == editorpy)
				{
					int teditor = 2;
					while (*(traps + teditor) != 0)
					{
						if (*(traps + teditor) == 3 && *(traps + teditor + 2) == beforeclickx + 15 && *(traps + teditor + 3) == beforeclicky + 15)
						{
							if (*(traps + teditor + 4) - *(traps + teditor + 2) == 1)
							{
								*(traps + teditor + 4) = *(traps + teditor + 2) + 0;
								*(traps + teditor + 5) = *(traps + teditor + 3) + 1;
							}
							else if (*(traps + teditor + 5) - *(traps + teditor + 3) == 1)
							{
								*(traps + teditor + 4) = *(traps + teditor + 2) + -1;
								*(traps + teditor + 5) = *(traps + teditor + 3) + 0;
							}
							else if (*(traps + teditor + 4) - *(traps + teditor + 2) == -1)
							{
								*(traps + teditor + 4) = *(traps + teditor + 2) + 0;
								*(traps + teditor + 5) = *(traps + teditor + 3) + -1;
							}
							else if (*(traps + teditor + 5) - *(traps + teditor + 3) == -1)
							{
								*(traps + teditor + 4) = *(traps + teditor + 2) + 1;
								*(traps + teditor + 5) = *(traps + teditor + 3) + 0;
							}

							break;
						}
						else if (*(traps + teditor) == 10 && (int)*(float*)(traps + teditor + 2) == beforeclickx + 15 && (int)*(float*)(traps + teditor + 3) == beforeclicky + 15)
						{
							if (*(traps + teditor + 10) < 3) *(traps + teditor + 10) += 1;
							else *(traps + teditor + 10) = 0;

							break;
						}

						teditor += 12;
					}
				}

				if (editorpx != 0 || editorpy != 0)
				{
					beforeclickx = editorpx;
					beforeclicky = editorpy;
				}

				editorpx = 0;
				editorpy = 0;
			}

			copyrect(visualmap, savebutton, 120, 1200, 120, 10, 120, 40);
			copyrect(visualmap, addbutton, 120, 1200, 300, 10, 120, 40);
			copyrect(visualmap, addwallsbutton, 120, 1200, 480, 10, 120, 40);
			copyrect(visualmap, trash, 30, 1200, 1095, 15, 30, 30);
			copyrect(visualmap, backbutton, 30, 1200, 15, 495, 30, 30);
		}
		else if (*(mem + 1) == 4)
		{
			if (GetAsyncKeyState(VK_LBUTTON) != 0)
			{
				if (mp.x > 0 && mp.x < 200 && mp.y > 0 && mp.y < 200)
				{
					int tadder = 2;
					while (*(traps + tadder) != 0 && *(traps + tadder) != 8) tadder += 12;

					*(traps + tadder) = 1;

					*(traps + tadder + 2) = 75;
					*(traps + tadder + 3) = 75;
				}
				else if (mp.x > 200 && mp.x < 400 && mp.y > 0 && mp.y < 200)
				{
					int tadder = 2;
					while (*(traps + tadder) != 0 && *(traps + tadder) != 8) tadder += 12;

					*(traps + tadder) = 2;
					*(traps + tadder + 2) = 75;
					*(traps + tadder + 3) = 75;
					*(traps + tadder + 4) = 195;
					*(traps + tadder + 5) = 75;
					*(traps + tadder + 6) = 1;
					*(float*)(traps + tadder + 8) = 75.0;
					*(float*)(traps + tadder + 9) = 75.0;
				}
				else if (mp.x > 400 && mp.x < 600 && mp.y > 0 && mp.y < 200)
				{
					int tadder = 2;
					while (*(traps + tadder) != 0 && *(traps + tadder) != 8) tadder += 12;

					*(traps + tadder) = 5;

					*(float*)(traps + tadder + 2) = 75.0;
					*(float*)(traps + tadder + 3) = 75.0;
					*(traps + tadder + 4) = 195;
					*(traps + tadder + 5) = 75;
				}
				else if (mp.x > 600 && mp.x < 800 && mp.y > 0 && mp.y < 200)
				{
					int tadder = 2;
					while (*(traps + tadder) != 0 && *(traps + tadder) != 8) tadder += 12;

					*(traps + tadder) = 3;

					*(traps + tadder + 2) = 75;
					*(traps + tadder + 3) = 75;
					*(traps + tadder + 4) = 76;
					*(traps + tadder + 5) = 75;
				}
				else if (mp.x > 800 && mp.x < 1000 && mp.y > 0 && mp.y < 200)
				{
					int tadder = 2;
					while (*(traps + tadder) != 0 && *(traps + tadder) != 8) tadder += 12;

					*(traps + tadder) = 4;

					*(float*)(traps + tadder + 2) = 75.0;
					*(float*)(traps + tadder + 3) = 75.0;
				}
				else if (mp.x > 1000 && mp.x < 1200 && mp.y > 0 && mp.y < 200)
				{
					int tadder = 2;
					while (*(traps + tadder) != 0 && *(traps + tadder) != 8) tadder += 12;

					*(traps + tadder) = 6;

					*(traps + tadder + 2) = 75;
					*(traps + tadder + 3) = 75;
				}
				else if (mp.x > 0 && mp.x < 200 && mp.y > 200 && mp.y < 400)
				{
					int tadder = 2;
					while (*(traps + tadder) != 0 && *(traps + tadder) != 8) tadder += 12;

					*(traps + tadder) = 9;
					*(traps + tadder + 2) = 90;
					*(traps + tadder + 3) = 90;
				}
				else if (mp.x > 200 && mp.x < 400 && mp.y > 200 && mp.y < 400)
				{
					int tadder = 2;
					while (*(traps + tadder) != 0 && *(traps + tadder) != 8) tadder += 12;

					*(traps + tadder) = 10;

					*(float*)(traps + tadder + 2) = 75;
					*(float*)(traps + tadder + 3) = 75;
					*(traps + tadder + 4) = 75;
					*(traps + tadder + 5) = 75;
					*(traps + tadder + 6) = 165;
					*(traps + tadder + 7) = 75;
				}
				else if (mp.x > 400 && mp.x < 600 && mp.y > 200 && mp.y < 400)
				{
					int tadder = 2;
					while (*(traps + tadder) != 0 && *(traps + tadder) != 8) tadder += 12;

					*(traps + tadder) = 11;

					*(traps + tadder + 2) = 75;
					*(traps + tadder + 3) = 75;
				}
				else if (mp.x > 600 && mp.x < 800 && mp.y > 200 && mp.y < 400)
				{
					int tadder = 2;
					while (*(traps + tadder) != 0 && *(traps + tadder) != 8) tadder += 12;

					*(traps + tadder) = 12;

					*(traps + tadder + 2) = 60;
					*(traps + tadder + 3) = 120;
					*(traps + tadder + 4) = 1;
				}
				else if (mp.x > 800 && mp.x < 1000 && mp.y > 200 && mp.y < 400)
				{
					int tadder = 2;
					while (*(traps + tadder) != 0 && *(traps + tadder) != 8) tadder += 12;

					*(traps + tadder) = 12;

					*(traps + tadder + 2) = 60;
					*(traps + tadder + 3) = 120;
					*(traps + tadder + 4) = 2;
				}

				while (GetAsyncKeyState(VK_LBUTTON) != 0) processmsg();

				*(mem + 1) = 3;
				goto startloop;
			}

			stretchrect(visualmap, addtraps, 120, 60, 1200, 600);
		}
		else if (*(mem + 1) == 5)
		{
			if (GetAsyncKeyState(VK_LBUTTON) != 0)
			{
				if (mp.x > 120 && mp.x < 240 && mp.y > 10 && mp.y < 50)
				{
					*(mem + 1) = 3;

					drawwalls();

					while (GetAsyncKeyState(VK_LBUTTON) != 0) processmsg();

					goto startloop;
				}

				int dwx = mp.x - mp.x % 60, dwy = mp.y - mp.y % 60;

				if (dwx > 59 && dwx < 1140 && dwy > 59 && dwy < 540)
				{
					drawrect(background, 1200, dwx, dwy, 60, 60, 1);
					drawrect(visualbackground, 1200, dwx, dwy, 60, 60, 0x909090);
				}
			}
			if (GetAsyncKeyState(VK_RBUTTON) != 0)
			{
				int dwx = mp.x - mp.x % 60, dwy = mp.y - mp.y % 60;

				if (dwx > 59 && dwx < 1140 && dwy > 59 && dwy < 540)
				{
					drawrect(background, 1200, dwx, dwy, 60, 60, 0);
					drawrect(visualbackground, 1200, dwx, dwy, 60, 60, 0x383838);
				}
			}

			copyrect(visualmap, savebutton, 120, 1200, 120, 10, 120, 40);
		}
		else
		{
			copyrect(visualmap, editbutton, 120, 1200, 120, 10, 120, 40);
			copyrect(visualmap, importbutton, 120, 1200, 300, 10, 120, 40);
			if (isreplaying) copyrect(visualmap, replaying, 120, 1200, 480, 10, 120, 40);

			if (GetAsyncKeyState(VK_LBUTTON) != 0)
			{
				if (mp.x > 120 && mp.x < 240 && mp.y > 10 && mp.y < 50)
				{
					while (GetAsyncKeyState(VK_LBUTTON) != 0) processmsg();

					*(mem + 1) = 3;

					if (solutionreplay)
					{
						free(solutionreplay);
						solutionreplay = 0;
					}

					CopyMemory(traps, trapssp, 16376 * 4);

					replacerect(background, 1200, 0, 0, 1200, 600, 0, 2);
					replacerect(background, 1200, 0, 0, 1200, 600, 0, 3);
					replacerect(background, 1200, 0, 0, 1200, 600, 0, 4);
					replacerect(background, 1200, 0, 0, 1200, 600, 0, 5);

					editorpx = 0;
					editorpy = 0;
				}
				else if (mp.x > 300 && mp.x < 420 && mp.y > 10 && mp.y < 50)
				{
					while (GetAsyncKeyState(VK_LBUTTON) != 0) processmsg();

					FILE* toimport;

				startimport:

					OPENFILENAME ofn;
					ZeroMemory(&ofn, sizeof(OPENFILENAME));
					ofn.lStructSize = sizeof(OPENFILENAME);
					ofn.hwndOwner = handle;
					ofn.lpstrTitle = "Import";
					ofn.lpstrDefExt = "dun";
					ofn.lpstrFilter = "Dungeon File\0*.dun\0\0";
					char importlocation[512] = "\0";
					ofn.lpstrFile = importlocation;
					ofn.nMaxFile = 512;

					ofn.Flags = OFN_FILEMUSTEXIST | OFN_EXPLORER;

					GetOpenFileName(&ofn);

					toimport = fopen(importlocation, "rb");
					if (toimport == 0)
					{
						if (MessageBox(handle, "Error occurred!", "Warning!", MB_RETRYCANCEL | MB_ICONEXCLAMATION | MB_APPLMODAL) == IDRETRY) goto startimport;
						else goto startloop;
					}

					int* importbg = (int*)calloc(200, 4);
					fread(importbg, 200 * 4, 1, toimport);

					int wallsimportx = 0, wallsimporty = 0;
					while (wallsimporty < 600)
					{
						if (*(importbg + (wallsimporty / 60) * 20 + (wallsimportx / 60)) == 1) *(background + wallsimporty * 1200 + wallsimportx) = 1;
						else *(background + wallsimporty * 1200 + wallsimportx) = 0;

						wallsimportx++;
						if (wallsimportx == 1200)
						{
							wallsimportx = 0;
							wallsimporty++;
						}
					}

					drawrect(visualbackground, 1200, 0, 0, 1200, 600, 0x383838);
					drawwalls();

					int checkpoint1, checkpoint2;
					fread(&checkpoint1, 4, 1, toimport);
					fread(&checkpoint2, 4, 1, toimport);

					ZeroMemory(trapssp, 16376 * 4);
					fread(trapssp, checkpoint1 * 4, 1, toimport);

					int bgextrasplacer = 2;
					while (*(trapssp + bgextrasplacer) != 0)
					{
						if (*(trapssp + bgextrasplacer) == 12)
						{
							if (*(trapssp + bgextrasplacer + 4) == 1)
							{
								replacerect(background, 1200, *(trapssp + bgextrasplacer + 2) - 5, *(trapssp + bgextrasplacer + 3), 5, 6, 3, 0);
								replacerect(background, 1200, *(trapssp + bgextrasplacer + 2), *(trapssp + bgextrasplacer + 3), 60, 6, 2, 0);
								replacerect(background, 1200, *(trapssp + bgextrasplacer + 2) + 60, *(trapssp + bgextrasplacer + 3), 5, 6, 3, 0);
							}
							else if (*(trapssp + bgextrasplacer + 4) == 2)
							{
								replacerect(background, 1200, *(trapssp + bgextrasplacer + 2) - 5, *(trapssp + bgextrasplacer + 3), 5, 6, 5, 0);
								replacerect(background, 1200, *(trapssp + bgextrasplacer + 2), *(trapssp + bgextrasplacer + 3), 60, 6, 4, 0);
								replacerect(background, 1200, *(trapssp + bgextrasplacer + 2) + 60, *(trapssp + bgextrasplacer + 3), 5, 6, 5, 0);
							}
						}

						bgextrasplacer += 12;
					}

					if (solutionreplay)
					{
						free(solutionreplay);
						solutionreplay = 0;
					}
					solutionreplay = (int*)calloc(8192, 4);
					fread(solutionreplay, checkpoint2 * 4, 1, toimport);

					fclose(toimport);
					free(importbg);

					retrygame(trapssp);
					goto startloop;
				}

			}
		}

		if (*(mem + 1) == 2)
		{
			if (isreplaying)
			{
				isreplaying = 0;
				areplaypos = 0;
			}

			if (trapdeath == 1)
			{
				PlaySound(deathsound, 0, SND_MEMORY | SND_ASYNC);

				trapdeath = 0;
				deaths++;

				CopyMemory(traps, trapssp, 16376 * 4);

				player.x = *(traps + 3);
				player.y = *(traps + 4) - 7;
				player.vx = 0.2;
				player.vy = 0;
			}
			else
			{
				copyrect(visualmap, movingright, 13, 1200, (int)player.x - 6, (int)player.y - 6, 13, 13);
				StretchDIBits(hdc, 0, window.bottom - window.top, window.right - window.left, window.top - window.bottom, 0, 0, 1200, 600, visualmap, &bmi, DIB_RGB_COLORS, SRCCOPY);
			}

			while (GetAsyncKeyState(VK_LBUTTON) == 0 && GetAsyncKeyState(VK_SPACE) == 0)
			{
				if (GetAsyncKeyState('S') != 0 && solutionreplay)
				{
					isreplaying = 1;
					break;
				}
			
				processmsg();
			}

			timepos = 0;
			jumpcount = 0;
			lit = -100;
			animpos = 9;
			delanimpos = 0.1;
			gravity = 1;
			moveafterslide = 0;

			holdjumpblocker = 1;

			CopyMemory(traps, trapssp, 16376 * 4);

			player.x = (float)*(traps + 3);
			player.y = (float)(*(traps + 4) - 7);
			player.vx = 0.2;
			player.vy = 0;

			*(mem + 1) = 1;
		}

		*(mem + 4) += 1;

		if (GetAsyncKeyState('R') != 0 && *(mem + 1) == 1)
		{
			isreplaying = 0;
			deaths++;
			retrygame(trapssp);
		}

		StretchDIBits(hdc, 0, window.bottom - window.top, window.right - window.left, window.top - window.bottom, 0, 0, 1200, 600, visualmap, &bmi, DIB_RGB_COLORS, SRCCOPY);
	}

	return 0;
}
