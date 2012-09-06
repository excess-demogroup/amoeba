#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

#include "resource.h"
#include "win32-config.h"
#include "image/image.h"
#include <math/array.h>

typedef BOOL (CALLBACK *LPDSENUMCALLBACKA)(LPGUID, LPCSTR, LPCSTR, LPVOID);
typedef HRESULT (WINAPI *DIRECTSOUNDENUMERATEA) (LPDSENUMCALLBACKA, LPVOID);
LRESULT CALLBACK config_callback(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

BOOL CALLBACK enum_callback(LPGUID lpGuid, LPCSTR lpcstrDescription, LPCSTR lpcstrModule,
		            LPVOID lpContext)
{
	struct dsenum_pass_struct *dps = (struct dsenum_pass_struct *)lpContext;
	SendMessage(dps->sound_list, CB_ADDSTRING, 0, (LPARAM)lpcstrDescription);
	dps->sound_guids->add_end(lpGuid);
	return true;
}

Win32Config *Win32Config::instance()
{
	static Win32Config *instance = NULL;
	
	if (instance == NULL) {
		instance = new Win32Config();
	}
	return instance;
}

Win32Config::Win32Config()
{
}

void Win32Config::dialog()
{
	HRESULT result = DialogBox(GetModuleHandle(NULL), (LPCTSTR)IDD_CONFIG,
		NULL, (DLGPROC)config_callback);
	if (result != IDC_OK) PostQuitMessage(1);
}

LRESULT CALLBACK config_callback(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int best_mode = 0;
	static Array<int> modes;
	static Array<LPGUID> sound_guids;

	int count = 0;
	int i;
	HWND screenmode_list;

	switch (message) {
	case WM_INITDIALOG:
		DEVMODE mode;
		screenmode_list = GetDlgItem(hDlg, IDC_SCREENMODE);

		while (EnumDisplaySettings(NULL, count, &mode)) {
			char buf[256];
			if (mode.dmBitsPerPel <= 8) {
				count++;
				continue;
			}
			if (mode.dmDisplayFrequency == 0) {
				sprintf(buf, "%ldx%ldx%ldbpp",
					mode.dmPelsWidth,
					mode.dmPelsHeight,
					mode.dmBitsPerPel);
			} else {
				sprintf(buf, "%ldx%ldx%ldbpp@%ldhz",
					mode.dmPelsWidth,
					mode.dmPelsHeight,
					mode.dmBitsPerPel,
					mode.dmDisplayFrequency);
			}

			if ((mode.dmPelsWidth  == 640) && 
			    (mode.dmPelsHeight == 480) &&
			    (mode.dmBitsPerPel > 8)) {
				best_mode = modes.num_elems();
			}

			SendMessage(screenmode_list, CB_ADDSTRING, 0, (LPARAM)buf);
			modes.add_end(count++);
		}
		SendMessage(screenmode_list, CB_SETCURSEL, best_mode, 0);

		{
			HWND zbuffer_list = GetDlgItem(hDlg, IDC_ZBUFFER);
			SendMessage(zbuffer_list, CB_ADDSTRING, 0, (LPARAM)"16bpp");
			SendMessage(zbuffer_list, CB_ADDSTRING, 0, (LPARAM)"24bpp");
			SendMessage(zbuffer_list, CB_ADDSTRING, 0, (LPARAM)"32bpp");
			SendMessage(zbuffer_list, CB_SETCURSEL, 2, 0);
		}

		{
			HWND windowmode_list = GetDlgItem(hDlg, IDC_WINDOWMODE);
			SendMessage(windowmode_list, CB_ADDSTRING, 0, (LPARAM)"Fullscreen");
			SendMessage(windowmode_list, CB_ADDSTRING, 0, (LPARAM)"Windowed");
			SendMessage(windowmode_list, CB_SETCURSEL, 0, 0);
		}
	
		{
			HWND sound_list = GetDlgItem(hDlg, IDC_SOUND);
			
			/* check that we can load DirectSound, and enumerate output devices */
			HMODULE library = (HMODULE)LoadLibrary("dsound.dll");
		        if (library != NULL) {
				DIRECTSOUNDENUMERATEA dsEnum =
					(DIRECTSOUNDENUMERATEA)GetProcAddress(library, "DirectSoundEnumerateA");
				if (dsEnum != NULL) {
					struct dsenum_pass_struct dps;
					dps.sound_list = sound_list;
					dps.sound_guids = &sound_guids;
					dsEnum(enum_callback, &dps);
				}
			}

			/* if the only output device is "Primary Output Device", nuke it :-) */
			if (sound_guids.num_elems() == 1 && sound_guids[0] == NULL) {
				SendMessage(sound_list, CB_DELETESTRING, (WPARAM)0, 0);
				sound_guids[0] = (LPGUID)-1;
			} else {
				sound_guids.add_end((LPGUID)-1);
			}
			SendMessage(sound_list, CB_ADDSTRING, 0, (LPARAM)"No sound");

			SendMessage(sound_list, CB_SETCURSEL, 0, 0);
		}
		
		/* load the logo */
		{
			Image *logo = load_image("launcherlogo.png");
			BITMAPINFOHEADER hdr = {
				sizeof(BITMAPINFOHEADER),
				logo->get_width(),
				-logo->get_height(),
				1,
				logo->get_bpp(),
				BI_RGB,
				0,
				100,
				100,
				0,
				0
			};
			BITMAPINFO bmi = {
				hdr,
				{ 0, 0, 0, 0 }
			};
		
			HWND logo_control = GetDlgItem(hDlg, IDB_LOGO);
			HDC logo_dc = GetDC(logo_control);
			HBITMAP bmp = CreateDIBitmap(logo_dc,
				&hdr, CBM_INIT, logo->get_pixel_data(),
				&bmi, DIB_RGB_COLORS);
			SendMessage(logo_control, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)bmp);
			ReleaseDC(hDlg, logo_dc);
			delete logo;
		}
		break;
		
	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_OK) {
			Win32Config *c = Win32Config::instance();
		
			HWND windowmode_list = GetDlgItem(hDlg, IDC_WINDOWMODE);
			i = SendMessage(windowmode_list, CB_GETCURSEL, 0, 0);
			c->set_fullscreen((i < 0 || i == 0));

			/* handle SOUND! :-P */
			HWND sound_list = GetDlgItem(hDlg, IDC_SOUND);
			i = SendMessage(sound_list, CB_GETCURSEL, 0, 0);
			if (i < 0) {
				c->set_sound((LPGUID)-1);
			} else {
				c->set_sound(sound_guids[i]);
			}

			screenmode_list = GetDlgItem(hDlg, IDC_SCREENMODE);
			i = SendMessage(screenmode_list, CB_GETCURSEL, 0, 0);
			if (i < 0) i = best_mode;
			c->set_mode(modes[i]);

			HWND zbuf_list = GetDlgItem(hDlg, IDC_ZBUFFER);
			i = SendMessage(zbuf_list, CB_GETCURSEL, 0, 0);
			if (i < 0) i = 2;
			c->set_zbuffer(16 + i * 8);

			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		if (LOWORD(wParam) == IDC_CANCEL) {
			EndDialog(hDlg, LOWORD(wParam));
			ExitProcess(0);
			return TRUE;
		}
		break;
		
	case WM_CLOSE:
		EndDialog(hDlg, LOWORD(wParam));
		ExitProcess(0);
		break;
	}
	return FALSE;
}
