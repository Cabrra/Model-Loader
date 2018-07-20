
#include "inc/DirectXLayer.h"
#include "resource.h"
#include "mmsystem.h"
#pragma comment(lib, "WinMM.lib")
#include <fstream>
#include <vector>
#include <map>
#include <string>
using namespace std;
#include <list>

#pragma comment(lib, "lib/DirectXLayer_2013.lib")

//This object is your interface to the DirectX abstraction layer.
DirectXLayer DXLayer;

HINSTANCE g_hInst;
HWND g_hWnd;
HWND g_Dlg;
HWND g_FPS;
HMENU g_dlgMenu;
RECT g_displayPanel, g_oldDisplaySize;
const unsigned int g_displayPanelLeft = 275;
const unsigned int g_displayPanelTop = 0;
HACCEL hacc;
int g_width = 1280;
int g_height = 720;
float proj = 70.0f;
float g_cameraHeight = 30.0f;
float g_cameraZ = 15.0f;

bool loaded = false;
string mainApplicationDirectory;

struct renderstuff{
	string name;
	BufferInfo buffer;
	DirectX::XMMATRIX matrix;
};

//my globals
string texturName;
std::vector<renderstuff> myStruct;

INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void InitializeComponents(void);
void ResizeDisplayPanel(void);
void ResizeDevice(void);
void Update(void);
void Render(void);
void LoadMeshFromFile(const char * path, XMMATRIX* myMatrix);
string GetFilePath(void);
void CreateCameraMatrix(XMVECTOR position, XMVECTOR target);

//--------------------------------------------------------------------------------------
//Entry point to the program. Initializes everything and goes into a message processing 
//loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	UNREFERENCED_PARAMETER( hPrevInstance );
	UNREFERENCED_PARAMETER( lpCmdLine );
	g_hInst = hInstance;
	/*========= Application Initialization =========*/
	g_dlgMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU1));
	g_Dlg = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), 0, DialogProc, 0);
	g_FPS = GetDlgItem(g_Dlg, IDC_STATIC_FPS);
	
	InitializeComponents();
	hacc = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR1));
	ShowWindow(g_Dlg, nCmdShow);
	
	//Set program path.
	HMODULE hModule = GetModuleHandle(NULL);
	char path[MAX_PATH];
	GetModuleFileName(hModule, path, MAX_PATH);
	mainApplicationDirectory = path;
	unsigned int pos = mainApplicationDirectory.find_last_of('\\');
	mainApplicationDirectory.erase(pos);
	SetCurrentDirectory(mainApplicationDirectory.c_str());
	
	HWND renderwindow = GetDlgItem(g_Dlg, IDC_RENDER_WINDOW);
	
	if (!DXLayer.Initialize(renderwindow, g_width, g_height))
	{
		DXLayer.Cleanup();
		return 0;
	}

	//Main message loop
	MSG msg = {0};
	while( WM_QUIT != msg.message )
	{
		if ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{
			if (!TranslateAccelerator(g_Dlg, hacc, &msg) && !IsDialogMessage(g_Dlg, &msg))
			{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
		}
		else
		{
			Update();
			Render();
		}
	}

	DXLayer.Cleanup();
	return ( int )msg.wParam;
}



void LoadMeshFromFile(const char *path, XMMATRIX* myMatrix)
{
	std::vector <VertexPositionNormalTexture> vertexList;
	std::vector <unsigned int> indexList;

	ifstream bin(path, ios::in | ios::binary);
	if (bin.is_open())
	{
		unsigned int size; //texturename
		bin.read((char*)&size, sizeof (unsigned int));
		char* texture = new char[size + 1];
		bin.read(texture, size);
		texture[size + 1] = '\0';

		//vertex list
		unsigned int vertexsize;
		bin.read((char*)&vertexsize, sizeof (unsigned int));
		for (unsigned int i = 0; i < vertexsize; i++)
		{
			unsigned int myle;
			bin.read((char*)&myle, sizeof (unsigned int));
			char* aux = new char[myle];
			bin.read(aux, myle);
			texturName = aux;
	
		}
		//unique vertex
		unsigned int uniquevertexsize;
		bin.read((char*)&uniquevertexsize, sizeof (unsigned int));
		for (unsigned int i = 0; i < uniquevertexsize; i++)
		{
			float fx, fy, fz, fnx, fny, fnz, fu, fv;
			VertexPositionNormalTexture myVertex;

			//position
			bin.read((char*)&fx, sizeof (float));
			bin.read((char*)&fy, sizeof (float));
			bin.read((char*)&fz, sizeof (float));
			myVertex.position.x = fx;
			myVertex.position.y = fy;
			myVertex.position.z = fz;

			//normals
			bin.read((char*)&fnx, sizeof (float));
			bin.read((char*)&fny, sizeof (float));
			bin.read((char*)&fnz, sizeof (float));
			myVertex.normal.x = fnx;
			myVertex.normal.y = fny;
			myVertex.normal.z = fnz;

			//UV
			bin.read((char*)&fu, sizeof (float));
			bin.read((char*)&fv, sizeof (float));
			myVertex.textureCoordinate.x = fu;
			myVertex.textureCoordinate.y = fv;

			vertexList.push_back(myVertex);
		}

		unsigned int trianglesize;
		bin.read((char*)&trianglesize, sizeof (unsigned int));
		for (unsigned int i = 0; i < trianglesize; i++)
		{
			unsigned int one, two, three;
			bin.read((char*)&one, sizeof (unsigned int));
			bin.read((char*)&two, sizeof (unsigned int));
			bin.read((char*)&three, sizeof (unsigned int));

			indexList.push_back(one);
			indexList.push_back(two);
			indexList.push_back(three);
		}
	}
	bin.close();

	//truncate texture name
	const size_t last_slash = texturName.find_last_of("\\/");
	if (std::string::npos != last_slash)
	{
		texturName.erase(0, last_slash + 1);
	}
	BufferInfo myBuffer;
	DXLayer.LoadVertexAndIndexData_PosNormalTexture(&vertexList[0], vertexList.size(), &indexList[0], indexList.size(), &myBuffer);

	DXLayer.LoadTexture(texturName.c_str());

	renderstuff myPack;
	myPack.buffer = myBuffer;
	myPack.name = texturName;
	myPack.matrix = *myMatrix;

	myStruct.push_back(myPack);
}

//Open a file dialog box and return the selected path.
string GetFilePath(void)
{
	string extension;
	char buffer[256] = "\0";
	OPENFILENAME opFile;
	extension = "Mesh Files (*.mesh)#*.mesh#";
	char szFilters[64];		//buffer of file filters
	memset( szFilters, 0, sizeof( szFilters ) );
	memcpy( szFilters, extension.c_str(), extension.length() );
	for ( int i = 0; i < sizeof(szFilters); i++ ) szFilters[ i ] = ( ( szFilters[ i ] == '#' ) ? '\0' : szFilters[ i ] );

	memcpy(buffer, mainApplicationDirectory.c_str(), mainApplicationDirectory.length());
	ZeroMemory( &opFile, sizeof(opFile) );
	opFile.lStructSize		= sizeof(opFile);
	opFile.hwndOwner		= g_Dlg;
	opFile.lpstrFile		= buffer;
	opFile.lpstrFile[0]	= '\0'; //because szFile is crap.
	opFile.nMaxFile		= sizeof(buffer);
	opFile.lpstrFilter		= szFilters;
	opFile.nFilterIndex	= 0;
	opFile.lpstrFileTitle	= NULL;
	opFile.nMaxFileTitle	= 0;
	opFile.lpstrInitialDir	= mainApplicationDirectory.c_str();
	opFile.Flags			= OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;

	SetCurrentDirectory(mainApplicationDirectory.c_str());
	string path;
	if (GetOpenFileName(&opFile) == TRUE)
		path = opFile.lpstrFile;
	else
		path = "";

	return path;
}

/*=================== Message Handler ========================*/
INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int wmId = 0;
	
	string path;
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			HICON bigIcon = NULL;
			bigIcon = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), 0);
			SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)bigIcon);
			loaded = true;
			break;
		}
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		switch(wmId)
		{
		case ID_ACCELERATOR_OPEN:
		case ID_FILE_OPEN_MENU:
		{
								  MessageBox(NULL, "I'm a placeholder for now!", "Task!", MB_OK);
								  break;
		}
		case ID_FILE_EXIT:
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			return TRUE;
		case IDC_BUTTON1:
			{
				//MessageBox(NULL, "Load the bench and draw it on the left", "Task!", MB_OK);
							string path = GetFilePath();
							if (path.length() > 0)
							{
								XMMATRIX myMatrix = XMMatrixIdentity();
								XMMATRIX offset = XMMatrixTranslation(-15.0f, 0.0f, 0.0f);
								myMatrix = myMatrix * offset;
								LoadMeshFromFile(path.c_str(), &myMatrix);
							}
				break;
			}
		case IDC_BUTTON2:
			{
				//MessageBox(NULL, "Load Major N and draw him in the middle", "Task!", MB_OK);
							string path = GetFilePath();
							if (path.length() > 0)
							{
								XMMATRIX myMatrix = XMMatrixIdentity();
								
								LoadMeshFromFile(path.c_str(), &myMatrix);
							}
				break;
			}
		case IDC_BUTTON3:
			{
				MessageBox(NULL, "Load the model with the hierarchy, and draw it on the right", "Task!", MB_OK);
				break;
			}
		}

	case WM_SIZE:
		{
			int width = LOWORD(lParam);
			int height = HIWORD(lParam);
			//Only resize if necessary... mostly to keep from breaking DirectX debugging.
			if (width != 0 && height != 0)
				ResizeDisplayPanel();

			break;
		}
	case WM_CLOSE:
			DestroyWindow(hDlg);
		
		return TRUE;
	case WM_GETMINMAXINFO:
		{
			LPMINMAXINFO lpInfo = (LPMINMAXINFO)lParam;
			if(lpInfo)
				lpInfo->ptMinTrackSize.x = 1024, lpInfo->ptMinTrackSize.y = 768;
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		return TRUE;
	}

	return FALSE;
}

void InitializeComponents(void)
{
	g_oldDisplaySize.bottom = 0;
	g_oldDisplaySize.top = 0;
	g_oldDisplaySize.left = 0;
	g_oldDisplaySize.right = 0;
	ResizeDisplayPanel();
}

void ResizeDisplayPanel(void)
{
	RECT mainwindow;
	GetWindowRect(g_Dlg, &mainwindow);
	g_displayPanel.top = mainwindow.top;
	g_displayPanel.left = mainwindow.left + 275;
	g_displayPanel.bottom = mainwindow.bottom;
	g_displayPanel.right = mainwindow.right;

	g_width = g_displayPanel.right - g_displayPanel.left - 25;
	g_height = g_displayPanel.bottom - g_displayPanel.top - 75;

	HWND pict = GetDlgItem(g_Dlg, IDC_RENDER_WINDOW);
	SetWindowPos(pict, HWND_TOP, g_displayPanelLeft, g_displayPanelTop, g_width, g_height, SWP_SHOWWINDOW);
	InvalidateRect(g_Dlg, NULL, true);

	DXLayer.ResizeDevice(g_width, g_height);

	HWND div = GetDlgItem(g_Dlg, IDC_TOOL_DIVIDER);
	RECT rdiv;
	GetWindowRect(div, &rdiv);
	SetWindowPos(div, HWND_TOP, 265, 0, 10, mainwindow.bottom - mainwindow.top, SWP_SHOWWINDOW);
	DXLayer.CreateProjectionMatrix(proj);
}

void Update(void)
{
	//BASIC camera controls.
	if (GetAsyncKeyState(VK_UP))
			g_cameraHeight += 0.01f;
	if (GetAsyncKeyState(VK_DOWN))
			g_cameraHeight -= 0.01f;
	if (GetAsyncKeyState(VK_RIGHT))
			g_cameraZ += 0.01f;
	if (GetAsyncKeyState(VK_LEFT))
			g_cameraZ -= 0.01f;
	DXLayer.CreateCameraMatrix(XMVectorSet(0.0f, g_cameraHeight, g_cameraZ, 0.0f), XMVectorSet(0.0f, 5.0f, 0.0f, 0.0f));
}

void Render()
{
	float clearcolor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	DXLayer.Clear(clearcolor);
	for (unsigned int i = 0; i < myStruct.size(); i++)
	{
		DXLayer.SetWorldMatrix(&myStruct[i].matrix);
		DXLayer.SetTexture(myStruct[i].name.c_str());
		DXLayer.DrawMesh(&myStruct[i].buffer);
		
	}
	DXLayer.Present();
}

void CreateCameraMatrix(XMVECTOR position, XMVECTOR target)
{

}