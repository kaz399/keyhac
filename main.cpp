#include <string>
#include <vector>
#include <list>

#include <wchar.h>

#include <windows.h>
#include <shellapi.h>

#if defined(_DEBUG)
#undef _DEBUG
#include "python.h"
#define _DEBUG
#else
#include "python.h"
#endif

#define PYTHON_INSTALL_PATH L"c:\\opt\\python311"

//--------------------------------------

int AppMain()
{
	std::wstring exe_dir;
	PyConfig py_conf;
	PyConfig_InitIsolatedConfig(&py_conf);

	// Get exe's directory
	{
		wchar_t exe_path_buf[MAX_PATH + 1];
		GetModuleFileName(NULL, exe_path_buf, MAX_PATH);

		exe_dir = exe_path_buf;

		size_t last_backslash_pos = exe_dir.find_last_of(L"\\/");
		if (last_backslash_pos >= 0)
		{
			exe_dir = exe_dir.substr(0, last_backslash_pos);
		}
		else
		{
			exe_dir = L"";
		}
	}

	// Setup environment variable "PATH"
	{
		std::wstring env_path;

		wchar_t tmp_buf[1];
		DWORD ret = GetEnvironmentVariable(L"PATH", tmp_buf, 0);
		if (ret > 0)
		{
			DWORD len = ret;
			wchar_t * buf = (wchar_t*)malloc((len + 1) * sizeof(wchar_t));

			GetEnvironmentVariable(L"PATH", buf, (len + 1) * sizeof(wchar_t));

			env_path = buf;

			free(buf);
		}

		env_path = exe_dir + L"/lib;" + env_path;

		SetEnvironmentVariable(L"PATH", env_path.c_str());
	}

	// Python home
	{
#if defined(_DEBUG)
		//PyConfig_SetString(&py_conf, &py_conf.home, PYTHON_INSTALL_PATH);
		PyConfig_SetString(&py_conf, &py_conf.home, const_cast<wchar_t*>(exe_dir.c_str()));
#else
		PyConfig_SetString(&py_conf, &py_conf.home, const_cast<wchar_t*>(exe_dir.c_str()));
#endif //_DEBUG
	}

	// Python module search path
	{
		std::wstring sys_path;
		py_conf.module_search_paths_set = 1;
		PyWideStringList_Append(&py_conf.module_search_paths, L"/extension");

		#if defined(_DEBUG)
		PyWideStringList_Append(&py_conf.module_search_paths, (exe_dir + L"\\..\\..\\dist\\keyhac\\lib").c_str());
		PyWideStringList_Append(&py_conf.module_search_paths, (exe_dir + L"\\..\\..\\dist\\keyhac\\packages").c_str());
		//PyConfig_SetString(&py_conf, &py_conf.platlibdir, (exe_dir + L"\\..\\..\\dist\\keyhac\\lib").c_str());
        #else
		PyWideStringList_Append(&py_conf.module_search_paths, (exe_dir + L"\\lib").c_str());
		PyWideStringList_Append(&py_conf.module_search_paths, (exe_dir + L"\\packages").c_str());
		#endif
	}
	
	// Setup sys.argv
	{
		wchar_t * cmdline = GetCommandLine();

		int argc;
		wchar_t ** argv = CommandLineToArgvW(cmdline, &argc);

		PyConfig_SetArgv(&py_conf, argc, argv);

		LocalFree(argv);
	}

	// Initialization
	Py_InitializeFromConfig(&py_conf);
	PyConfig_Clear(&py_conf);

	// enable DPI handling
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	// Execute python side main script
	{
		PyObject * module = PyImport_ImportModule("keyhac_main");
		if (module == NULL)
		{
			OutputDebugStringA("ERROR\n");
			PyErr_Print();
		}

		Py_XDECREF(module);
		module = NULL;
	}

	// Termination
	Py_Finalize();

	return 0;
}

#if ! defined(_DEBUG)

int WINAPI WinMain(
	HINSTANCE hInstance,      /* handle to current instance */
	HINSTANCE hPrevInstance,  /* handle to previous instance */
	LPSTR lpCmdLine,          /* pointer to command line */
	int nCmdShow              /* show state of window */
	)
{
	return AppMain();
}

#else

int main(int argc, const char * argv[])
{
	return AppMain();
}

#endif
