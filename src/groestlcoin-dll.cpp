

#define main groestlcoind_main
#include "groestlcoind.cpp"

extern "C" __declspec(dllexport) void DallardEntry() {
	exit(groestlcoind_main(__argc, __argv));
}

#undef main
#define main groestlcoin_cli_main
#include "groestlcoin-cli.cpp"

extern "C" __declspec(dllexport) void DallarCliEntry() {
	exit(groestlcoin_cli_main(__argc, __argv));
}

#undef main
#define main groestlcoin_tx_main
#include "groestlcoin-tx.cpp"

extern "C" __declspec(dllexport) void DallarTxEntry() {
	exit(groestlcoin_tx_main(__argc, __argv));
}

#undef main
#include "qt/bitcoin.cpp"

extern "C" WORD __cdecl __scrt_get_show_window_mode();

extern "C" __declspec(dllexport) void DallarQtEntry() {
	ExitProcess(wWinMain(GetModuleHandle(0), nullptr, _get_wide_winmain_command_line(), __scrt_get_show_window_mode()));
}
