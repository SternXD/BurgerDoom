#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL_main.h>

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Core.h>

#include "Base/CmdLine.h"
#include "Game/DoomMain.h"

static int bootstrap(int argc, char **argv) {
  // On Xbox One/Xbox Series consoles, pressing B generates a BackRequested
  // navigation event. If unhandled the app is suspended and control returns to
  // the shell. Mark it handled here so the game processes B through its normal
  // input path.
  {
    using namespace winrt::Windows::UI::Core;
    auto navigation = SystemNavigationManager::GetForCurrentView();
    navigation.BackRequested(
        [](const winrt::Windows::Foundation::IInspectable &,
           const BackRequestedEventArgs &args) { args.Handled(true); });
  }

  CmdLine::init(argc, argv);
  D_DoomMain();
  return 0;
}

int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
  return SDL_WinRTRunApp(bootstrap, NULL);
}
