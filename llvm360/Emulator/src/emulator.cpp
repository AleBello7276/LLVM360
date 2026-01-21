#include "Shared.h"
#include "Naive+.h"
#include <locale>
#include <codecvt>


int main(int argc, char *argv[])
{
	std::wstring path;
	if (argc < 2) {
		path = L"./kernel17559.exe";
	} else {
		path = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(argv[1]);
	}
	PBinaryHandle* handle = TranslateBinary(path, false, true);
	printf("YEY");
	return 0;
}
