#include "WorldMap.h"

namespace WorldMap
{
	namespace
	{
		// The map camera's zoom begins (once the map has loaded, a few frames after the menu opens): the game sets its start
		// near the player and its end at map height, then this moves the start to the end. Closing the map runs the same
		// zoom the other way, from map height down to the player: that one stays the game's, or the camera drops to the
		// ground at once while the map still shows (measured 2026-11-04: opening outdoors 184069 -> 287522, closing
		// 287522 -> 184069).
		struct BeginHook
		{
			static void thunk(RE::MapCameraStates::Transition* a_this)
			{
				func(a_this);
				if (a_this->zoomDestination.z <= a_this->currentPosition.z) {
					return;
				}
				a_this->zoomOrigin = a_this->zoomDestination;
				a_this->currentPosition = a_this->zoomDestination;
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};
	}

	bool Install()
	{
		REL::Relocation<std::uintptr_t> vtbl{ RE::VTABLE_MapCameraStates__Transition[0] };
		if (vtbl.address() == 0) {
			logger::error("The map camera's Transition isn't known for this game version: the map zooms as vanilla");
			return false;
		}
		BeginHook::func = vtbl.write_vfunc(0x1, BeginHook::thunk);
		logger::info("The world map opens at map height");
		return true;
	}
}
