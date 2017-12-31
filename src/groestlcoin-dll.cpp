

#define main dallard_main
#include "dallard.cpp"

extern "C" __declspec(dllexport) void DallardEntry() {
	exit(dallard_main(__argc, __argv));
}

#undef main
#define main dallar_cli_main
#include "dallar-cli.cpp"

extern "C" __declspec(dllexport) void DallarCliEntry() {
	exit(dallar_cli_main(__argc, __argv));
}

#undef main
#define main dallar_tx_main
#include "dallar-tx.cpp"

extern "C" __declspec(dllexport) void DallarTxEntry() {
	exit(dallar_tx_main(__argc, __argv));
}

#undef main
#include "qt/bitcoin.cpp"

extern "C" WORD __cdecl __scrt_get_show_window_mode();

extern "C" __declspec(dllexport) void DallarQtEntry() {
	ExitProcess(wWinMain(GetModuleHandle(0), nullptr, _get_wide_winmain_command_line(), __scrt_get_show_window_mode()));
}
