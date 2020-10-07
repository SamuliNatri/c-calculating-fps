#include <windows.h>
#include <stdint.h>
#include <wchar.h>

typedef uint32_t u32;

int running = 1;
int client_width = 640;
int client_height = 640;

int player_x = 0;
int player_y = 0;
int tile_size = 25;

void* memory;
BITMAPINFO bitmap_info;

void clear_screen(u32 color)
{
    u32 *pixel = (u32 *)memory; 
    
    for(int pixel_number = 0;
        pixel_number < client_width * client_height;
        ++pixel_number)
    {
        *pixel++ = color;
    }
}

void draw_rectangle(int rec_x,
                    int rec_y,
                    int rec_width,
                    int rec_height,
                    u32 color)
{
    u32 *pixel = (u32 *)memory;
    pixel += rec_y * client_width + rec_x;
    
    for(int y = 0;
        y < rec_height;
        ++y)
    {
        for(int x = 0;
            x < rec_width;
            ++x)
        {
            *pixel++ = color;
        }
        
        pixel += client_width - rec_width;
    }
}


LRESULT CALLBACK 
WindowProc(HWND window, 
           UINT message, 
           WPARAM w_param, 
           LPARAM l_param)
{
    LRESULT result;
    switch(message)
    {
        case WM_CLOSE:
        {
            running = 0;
        } break;
        
        case WM_KEYDOWN:
        {
            switch(w_param)
            {
                case VK_RIGHT:
                {
                    player_x += tile_size;
                } break;
                
                case VK_LEFT:
                {
                    player_x -= tile_size;
                } break;
                
                case VK_UP:
                {
                    player_y += tile_size;
                } break;
                
                case VK_DOWN:
                {
                    player_y -= tile_size;
                } break;
            }
        } break;
        
        default:
        {
            result = DefWindowProc(window,
                                   message, 
                                   w_param, 
                                   l_param);
        } break;
    }
    
    return result;
}

int WINAPI 
wWinMain(HINSTANCE instance, 
         HINSTANCE prev_instance, 
         PWSTR cmd_line, 
         int cmd_show)
{
    WNDCLASS window_class = {0};
    
    wchar_t class_name[] = L"GameWindowClass";
    
    window_class.lpfnWndProc = WindowProc;
    window_class.hInstance = instance;
    window_class.lpszClassName = class_name;
    window_class.hCursor = LoadCursor(0,
                                      IDC_CROSS
                                      );
    
    RECT window_rect;
    window_rect.left = 0;
    window_rect.top = 0;
    window_rect.right = client_width;
    window_rect.bottom = client_height;
    
    AdjustWindowRectEx(&window_rect,
                       WS_OVERLAPPEDWINDOW,
                       0,0
                       );
    
    int window_width = window_rect.right - window_rect.left;
    int window_height = window_rect.bottom - window_rect.top;
    
    int screen_width = GetSystemMetrics(SM_CXSCREEN);
    int screen_height = GetSystemMetrics(SM_CYSCREEN);
    
    int window_x = (screen_width / 2) - (window_width / 2);
    int window_y = (screen_height / 2) - (window_height / 2);
    
    RegisterClass(&window_class);
    
    HWND window = CreateWindowEx(0,
                                 class_name,
                                 L"Game",
                                 WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                                 window_x,
                                 window_y,
                                 window_width,
                                 window_height,
                                 0,
                                 0,
                                 instance,
                                 0);
    
    memory = VirtualAlloc(0,
                          client_width * client_height * 4,
                          MEM_RESERVE|MEM_COMMIT,
                          PAGE_READWRITE
                          );
    
    bitmap_info.bmiHeader.biSize = sizeof(bitmap_info.bmiHeader);
    bitmap_info.bmiHeader.biWidth = client_width;
    bitmap_info.bmiHeader.biHeight = client_height;
    bitmap_info.bmiHeader.biPlanes = 1;
    bitmap_info.bmiHeader.biBitCount = 32;
    bitmap_info.bmiHeader.biCompression = BI_RGB;
    
    HDC hdc = GetDC(window);
    
    LARGE_INTEGER start_counter, end_counter, counts, frequency, fps, ms;
    
    QueryPerformanceCounter(&start_counter);
    
    QueryPerformanceFrequency(&frequency);
    
    while(running)
    {
        MSG message;
        while(PeekMessage(&message, window, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
        
        clear_screen(0x111111);
        
        draw_rectangle(100, 100, 50, 50, 0x222222);
        draw_rectangle(150, 150, 25, 25, 0x333333);
        draw_rectangle(175, 175, 15, 15, 0x444444);
        
        draw_rectangle(player_x, 
                       player_y, 
                       tile_size, 
                       tile_size, 
                       0xff00f7);
        
        StretchDIBits(hdc,
                      0,
                      0,
                      client_width,
                      client_height,
                      0,
                      0,
                      client_width,
                      client_height,
                      memory,
                      &bitmap_info,
                      DIB_RGB_COLORS,
                      SRCCOPY
                      );
        
        QueryPerformanceCounter(&end_counter);
        
        counts.QuadPart = end_counter.QuadPart - start_counter.QuadPart;
        start_counter = end_counter;
        
        fps.QuadPart = frequency.QuadPart / counts.QuadPart;
        ms.QuadPart = ((1000 * counts.QuadPart) / frequency.QuadPart);
        
        wchar_t buf[100];
        
        swprintf_s(buf, sizeof(buf),
                   L"counts: %lld\nfrequency: %lld\nfps: %lld\nms: %lld\n\n",
                   counts.QuadPart,
                   frequency.QuadPart,
                   fps.QuadPart,
                   ms.QuadPart);
        OutputDebugString(buf);
    }
    
    return 0;
}
