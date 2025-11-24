#include "./includes/Core.h"

// Enable NVIDIA Optimus high-performance GPU
extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}
