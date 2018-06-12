#include "instruction.h"
#include "machine.h"

#include <Windows.h>

#include <iomanip>
#include <sstream>

struct console_desc
{
	HANDLE hIn, hOut;
	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
};

void resizeConsole(console_desc &desc, bool fullscreen = false, SHORT xSize = 120, SHORT ySize = 30)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	SMALL_RECT srWindowRect;
	COORD coordScreen;

	GetConsoleScreenBufferInfo(desc.hOut, &csbi);

	coordScreen = GetLargestConsoleWindowSize(desc.hOut);

	srWindowRect.Right = (SHORT)(min(xSize, coordScreen.X) - 1);
	srWindowRect.Bottom = (SHORT)(min(ySize, coordScreen.Y) - 1);
	srWindowRect.Left = srWindowRect.Top = (SHORT)0;

	coordScreen.X = xSize;
	coordScreen.Y = ySize;

	if ((DWORD)csbi.dwSize.X * csbi.dwSize.Y > (DWORD)xSize * ySize)
	{
		SetConsoleWindowInfo(desc.hOut, TRUE, &srWindowRect);
		SetConsoleScreenBufferSize(desc.hOut, coordScreen);
	}

	if ((DWORD)csbi.dwSize.X * csbi.dwSize.Y < (DWORD)xSize * ySize)
	{
		SetConsoleScreenBufferSize(desc.hOut, coordScreen);
		SetConsoleWindowInfo(desc.hOut, TRUE, &srWindowRect);
	}

	if (fullscreen)
		SetConsoleDisplayMode(desc.hOut, CONSOLE_FULLSCREEN_MODE, NULL);

	GetConsoleScreenBufferInfo(desc.hOut, &desc.csbiInfo);
}

DWORD printxy(console_desc desc, SHORT x, SHORT y, const char *str)
{
	COORD pos = { x, y };
	DWORD len = strlen(str);
	DWORD dwBytesWritten = 0;
	WriteConsoleOutputCharacter(desc.hOut, str, len, pos, &dwBytesWritten);

	return dwBytesWritten;
}

DWORD g_fdwSaveOldMode;
void exit_process(HANDLE hIn, bool msgbox = true, LPSTR lpszMessage = "")
{
	if (msgbox)
		MessageBox(NULL, lpszMessage, "Fatal Error!", MB_OK);

	SetConsoleMode(hIn, g_fdwSaveOldMode);

	ExitProcess(0);
}

#define _display_size_width 45
#define _display_size_height 19

void printBuffer(console_desc desc, std::stringstream &s)
{
	if (s.str().size() <= 0)
		return;

	static auto x = 3U, y = 1U;
	static char c = '\0';

	for (char out : s.str())
	{
		c = out;

		// Handle newlines
		if (c == 'n')
		{
			x = 2;
			++y;
		}
		else
			printxy(desc, x, y, &c);


		if (x < _display_size_width &&
			y < _display_size_height)
			x++;

		else if (x > _display_size_width - 1) {
			x = 3;
			++y;
		} 
		else if (y > _display_size_height - 1) {
			y = 1;
			x++;
		}
	}
}

#define _debug_menu_px 51

int main()
{
	console_desc console;
	console.hIn = GetStdHandle(STD_INPUT_HANDLE);
	console.hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	resizeConsole(console, false, 86, 20);
	GetConsoleScreenBufferInfo(console.hOut, &console.csbiInfo);
	CONSOLE_CURSOR_INFO cursorInfo;
	GetConsoleCursorInfo(console.hOut, &cursorInfo);
	cursorInfo.bVisible = false;
	SetConsoleCursorInfo(console.hOut, &cursorInfo);
	// Keyboard Events
	DWORD cNumRead, fdwMode, i;
	INPUT_RECORD irInBuf[128];
	int counter = 0;
	// Save the current input mode, to be restored on exit. 
	if (!GetConsoleMode(console.hIn, &g_fdwSaveOldMode))
		exit_process(console.hIn, "GetConsoleMode");
	// Enable the window and mouse input events. 
	fdwMode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
	if (!SetConsoleMode(console.hIn, fdwMode))
		exit_process(console.hIn, "SetConsoleMode");

	machine m;
	std::stringstream s;

	std::string str;

	str = std::string() + char(218);
	printxy(console, 2, 0, str.c_str());
	str = std::string() + char(191);
	printxy(console, _display_size_width, 0, str.c_str());
	str = std::string() + char(192);
	printxy(console, 2, _display_size_height, str.c_str());
	str = std::string() + char(217);
	printxy(console, _display_size_width, _display_size_height, str.c_str());

	for (int x = 3U; x < _display_size_width; x++) {
		str = std::string() + char(196);
		printxy(console, x, 0, str.c_str());
	}
	for (int y = 1U; y < _display_size_height; y++) {
		str = std::string() + char(179);
		printxy(console, 2, y, str.c_str());
	}
	for (int x = 3U; x < _display_size_width; x++) {
		str = std::string() + char(196);
		printxy(console, x, _display_size_height, str.c_str());
	}
	for (int y = 1U; y < _display_size_height; y++) {
		str = std::string() + char(179);
		printxy(console, _display_size_width, y, str.c_str());
	}

	printxy(console, 20, 0, "VM Output");

	bool incrementOneCycle = false;
	while (true)
	{
		// Read input events
		if (!PeekConsoleInput(
			console.hIn,
			irInBuf,
			128,
			&cNumRead))
			exit_process(console.hIn, "PeekConsoleInput");

		// Dispatch the events to the appropriate handler. 
		for (i = 0; i < cNumRead; i++)
		{
			switch (irInBuf[i].EventType)
			{
			case KEY_EVENT: // keyboard input 
			{
				if (irInBuf[i].Event.KeyEvent.bKeyDown)
				{
					// Get keypress
					switch (irInBuf[i].Event.KeyEvent.wVirtualScanCode)
					{
					default:
						break;
					}
				}
				else
				{
					// Get key release
					switch (irInBuf[i].Event.KeyEvent.wVirtualScanCode)
					{
					case 0x1:  // ESC
						exit_process(console.hOut, false);
						break;

					case 0x39: // SPACE
						m.manualHalt(!m.isHalted());
						break;

					case 0x21: // F
						if (m.isHalted())
						{
							m.manualHalt(false);
							incrementOneCycle = true;
						}
						break;

					default:
						break;
					}
				}

				break;
			}

			case WINDOW_BUFFER_SIZE_EVENT:
				break;

			case FOCUS_EVENT:
			case MENU_EVENT:
				break;

			default:
				exit_process(console.hIn, "Unknown input event type");
				break;
			}
		}
		if (!FlushConsoleInputBuffer(console.hIn))
			exit_process(console.hOut, "FlushConsoleInputBuffer");

		m.run();
		if (incrementOneCycle)
		{
			m.manualHalt(true);
			incrementOneCycle = false;
		}
	
		m.getOutputBufferData(s);

		printBuffer(console, s);
		s.str(std::string());

		static std::string dstr;

		dstr = std::string("HL: ") + (m.getMachineDebugData().halt ? "1" : "0");
		printxy(console, _debug_menu_px, 1, dstr.c_str());

		std::stringstream ss;
		ss << std::string("PC: ") << std::setfill('0') << std::setw(3) << std::uppercase << std::setbase(16) <<
			m.getMachineDebugData().pc << std::setbase(10) << " " <<  std::bitset<5>(m.getMachineDebugData().pc).to_string();
		printxy(console, _debug_menu_px, 3, ss.str().c_str());
		ss.str(std::string());
		
		mneumonic mn;
		auto asmcmd = mn.CompareValue(m.getMachineDebugData().rom[m.getMachineDebugData().pc].to_string().substr(_opcode));
		dstr = std::string("OP: ") +
			(asmcmd.size() == 2 ? " " : asmcmd.size() == 3 ? "" : "  ") + asmcmd +
			" " + m.getMachineDebugData().rom[m.getMachineDebugData().pc].to_string().substr(0, 5);
		printxy(console, _debug_menu_px, 5, dstr.c_str());

		ss << std::string("A0: ") << std::setfill('0') << std::setw(3) << std::uppercase << std::setbase(16) <<
			std::bitset<29>(m.getMachineDebugData().rom[m.getMachineDebugData().pc].to_string().substr(_arga)).to_ulong()
			<< std::setbase(10) << " " << m.getMachineDebugData().rom[m.getMachineDebugData().pc].to_string().substr(_arga);
		printxy(console, _debug_menu_px, 6, ss.str().c_str());
		ss.str(std::string());

		ss << std::string("A1: ") << std::setfill('0') << std::setw(3) << std::uppercase << std::setbase(16) <<
			std::bitset<29>(m.getMachineDebugData().rom[m.getMachineDebugData().pc].to_string().substr(_argb)).to_ulong()
			<< std::setbase(10) << " " << m.getMachineDebugData().rom[m.getMachineDebugData().pc].to_string().substr(_argb);
		printxy(console, _debug_menu_px, 7, ss.str().c_str());
		ss.str(std::string());

		for (auto i = 0U; i < 10; i++)
		{
			dstr = std::string("R" + std::to_string(i) + ": ") + m.getMachineDebugData().reg[i].to_string();
			printxy(console, _debug_menu_px, i + 9, dstr.c_str());
		}
	}

	std::string in;
	std::cin >> in;

	return 0;
}

//#include <iostream>
//#include <string>
//#include <stdint.h>
//
//#include <Windows.h>
//
//struct console_desc
//{
//	HANDLE hIn, hOut;
//	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
//};
//
//void scrollBuffer(console_desc &desc, DWORD lines)
//{
//	SMALL_RECT srctScrollRect, srctClipRect;
//	CHAR_INFO chiFill;
//	COORD coordDest;
//
//	srctScrollRect.Left = 0;
//	srctScrollRect.Top = 1;
//	srctScrollRect.Right = desc.csbiInfo.dwSize.X - (SHORT)lines;
//	srctScrollRect.Bottom = desc.csbiInfo.dwSize.Y - (SHORT)lines;
//
//	coordDest.X = 0;
//	coordDest.Y = 0;
//
//	srctClipRect = srctScrollRect;
//
//	chiFill.Attributes = FOREGROUND_INTENSITY;
//	chiFill.Char.AsciiChar = (char)' ';
//
//	ScrollConsoleScreenBuffer(
//		desc.hOut,
//		&srctScrollRect,
//		&srctClipRect,
//		coordDest,
//		&chiFill);
//}
//
//void newline(console_desc &desc)
//{
//	desc.csbiInfo.dwCursorPosition.X = 0;
//
//	if ((desc.csbiInfo.dwSize.Y - 1) == desc.csbiInfo.dwCursorPosition.Y)
//		scrollBuffer(desc, 1);
//	else
//		desc.csbiInfo.dwCursorPosition.Y += 1;
//
//	SetConsoleCursorPosition(desc.hOut, desc.csbiInfo.dwCursorPosition);
//}
//
//void setCursorPos(console_desc desc, SHORT x, SHORT y)
//{
//	COORD c = { x, y };
//	SetConsoleCursorPosition(desc.hOut, c);
//}
//
//DWORD printxy(console_desc desc, SHORT x, SHORT y, const char *str)
//{
//	COORD pos = { x, y };
//	DWORD len = strlen(str);
//	DWORD dwBytesWritten = 0;
//	WriteConsoleOutputCharacter(desc.hOut, str, len, pos, &dwBytesWritten);
//
//	return dwBytesWritten;
//}
//
//void resizeConsole(console_desc &desc, bool fullscreen = false, SHORT xSize = 120, SHORT ySize = 30)
//{
//	CONSOLE_SCREEN_BUFFER_INFO csbi;
//	SMALL_RECT srWindowRect;
//	COORD coordScreen;
//
//	GetConsoleScreenBufferInfo(desc.hOut, &csbi);
//
//	coordScreen = GetLargestConsoleWindowSize(desc.hOut);
//
//	srWindowRect.Right = (SHORT)(min(xSize, coordScreen.X) - 1);
//	srWindowRect.Bottom = (SHORT)(min(ySize, coordScreen.Y) - 1);
//	srWindowRect.Left = srWindowRect.Top = (SHORT)0;
//
//	coordScreen.X = xSize;
//	coordScreen.Y = ySize;
//
//	if ((DWORD)csbi.dwSize.X * csbi.dwSize.Y > (DWORD)xSize * ySize)
//	{
//		SetConsoleWindowInfo(desc.hOut, TRUE, &srWindowRect);
//		SetConsoleScreenBufferSize(desc.hOut, coordScreen);
//	}
//
//	if ((DWORD)csbi.dwSize.X * csbi.dwSize.Y < (DWORD)xSize * ySize)
//	{
//		SetConsoleScreenBufferSize(desc.hOut, coordScreen);
//		SetConsoleWindowInfo(desc.hOut, TRUE, &srWindowRect);
//	}
//
//	if (fullscreen)
//		SetConsoleDisplayMode(desc.hOut, CONSOLE_FULLSCREEN_MODE, NULL);
//
//	GetConsoleScreenBufferInfo(desc.hOut, &desc.csbiInfo);
//}
//
//typedef struct {
//	char *data;
//	uint_least32_t  used;
//	uint_least32_t  size;
//} img_buffer;
//
//void init_img_buffer(img_buffer *buf, uint_least32_t initialSize)
//{
//	buf->data = (char *)malloc(initialSize * sizeof(char));
//	buf->used = 0;
//	buf->size = initialSize;
//
//	memset(buf->data, 0, buf->size);
//}
//
//void insert_img_buffer(img_buffer *buf, uint_least32_t element)
//{
//	if (buf->used == buf->size)
//	{
//		buf->size *= 2;
//		buf->data = (char *)realloc(buf->data, buf->size * sizeof(char));
//	}
//
//	buf->data[buf->used++] = element;
//}
//
//void free_img_buffer(img_buffer *buf)
//{
//	free(buf->data);
//	buf->data = nullptr;
//	buf->used = buf->size = 0;
//}
//
//struct render_desc
//{
//	img_buffer buffer;
//};
//
//DWORD g_fdwSaveOldMode;
//
//void exit_process(HANDLE hIn, LPSTR lpszMessage = "", bool msgbox = true)
//{
//	if (msgbox)
//		MessageBox(NULL, lpszMessage, "Fatal Error!", MB_OK);
//
//	SetConsoleMode(hIn, g_fdwSaveOldMode);
//
//	ExitProcess(0);
//}
//
//int main()
//{
//	// Create the console
//	console_desc console;
//	console.hIn = GetStdHandle(STD_INPUT_HANDLE);
//	console.hOut = GetStdHandle(STD_OUTPUT_HANDLE);
//	resizeConsole(console);
//	GetConsoleScreenBufferInfo(console.hOut, &console.csbiInfo);
//
//	// Hide the cursor
//	CONSOLE_CURSOR_INFO cursorInfo;
//	GetConsoleCursorInfo(console.hOut, &cursorInfo);
//	cursorInfo.bVisible = false;
//	SetConsoleCursorInfo(console.hOut, &cursorInfo);
//
//	// Keyboard Events
//	DWORD cNumRead, fdwMode, i;
//	INPUT_RECORD irInBuf[128];
//	int counter = 0;
//	// Save the current input mode, to be restored on exit. 
//	if (!GetConsoleMode(console.hIn, &g_fdwSaveOldMode))
//		exit_process(console.hIn, "GetConsoleMode");
//	// Enable the window and mouse input events. 
//	fdwMode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
//	if (!SetConsoleMode(console.hIn, fdwMode))
//		exit_process(console.hIn, "SetConsoleMode");
//
//	// Create the 'renderer'
//	render_desc renderer;
//	init_img_buffer(&renderer.buffer, static_cast<uint_least32_t>(
//		console.csbiInfo.dwSize.X * console.csbiInfo.dwSize.Y));
//
//	while (true)
//	{
//		// Read input events
//		if (!PeekConsoleInput(
//			console.hIn,
//			irInBuf,
//			128,
//			&cNumRead))
//			exit_process(console.hIn, "PeekConsoleInput");
//
//		// Dispatch the events to the appropriate handler. 
//		for (i = 0; i < cNumRead; i++)
//		{
//			switch (irInBuf[i].EventType)
//			{
//			case KEY_EVENT: // keyboard input 
//			{
//				if (irInBuf[i].Event.KeyEvent.bKeyDown)
//				{
//					// Get keypress
//					switch (irInBuf[i].Event.KeyEvent.wVirtualKeyCode)
//					{
//					case VK_LEFT:
//
//						break;
//					case VK_RIGHT:
//
//						break;
//					case VK_UP:
//
//						break;
//					case VK_DOWN:
//
//						break;
//
//					default:
//						break;
//					}
//				}
//				else
//				{
//					// Get key release
//					switch (irInBuf[i].Event.KeyEvent.wVirtualKeyCode)
//					{
//					case VK_ESCAPE:
//						exit_process(console.hOut, "", false);
//
//					default:
//						break;
//					}
//				}
//
//				break;
//			}
//
//			case WINDOW_BUFFER_SIZE_EVENT:
//				printf("Console screen buffer is %d columns by %d rows.\n",
//					irInBuf[0].Event.WindowBufferSizeEvent.dwSize.X,
//					irInBuf[0].Event.WindowBufferSizeEvent.dwSize.Y);
//				break;
//
//			case FOCUS_EVENT:
//			case MENU_EVENT:
//				break;
//
//			default:
//				exit_process(console.hIn, "Unknown input event type");
//				break;
//			}
//		}
//		if (!FlushConsoleInputBuffer(console.hIn))
//			exit_process(console.hOut, "FlushConsoleInputBuffer");
//
//		Sleep(100);
//		
//		memset(renderer.buffer.data, 0, renderer.buffer.size);
//		//renderer.buffer.data[pos.X + 120 * pos.Y] = '0';
//
//		DWORD bytesWritten = 0;
//		if (!WriteConsoleOutputCharacter(
//			console.hOut,
//			renderer.buffer.data,
//			renderer.buffer.size,
//			{ 0, 0 },
//			&bytesWritten))
//			exit_process(console.hOut, "WriteConsoleOutputCharacter");
//	}
//
//	exit_process(console.hIn, "", false);
//	return 0;
//}
