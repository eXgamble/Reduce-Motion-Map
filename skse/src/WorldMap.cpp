#include "WorldMap.h"

namespace WorldMap
{
	namespace
	{
		using Transition = RE::MapCameraStates::Transition;
		using Clock = std::chrono::steady_clock;

		// Closing the map runs the same zoom as opening, the other way: from map height down to the player, while the
		// game dims the map to black and then fades the world in (measured 2026-11-19: the dive shows for ~0.35 s while
		// the map darkens). Holding the camera at map height until the zoom ends keeps the game's own fade and timing and
		// removes only the motion; the camera lands on the zoom's end on its last step, when the screen is black.
		// Ending the zoom at once instead dropped the camera to the ground while the map still showed (1aaad33).
		// If the zoom hasn't ended after this long, stop holding it, so a zoom that ends by arriving can't get stuck.
		constexpr auto kMaxHold = std::chrono::seconds(3);

		Transition*       closing = nullptr;  // the close zoom being held; menus and the camera run on one thread
		RE::NiPoint3      closeEnd;           // where the game's close zoom really ends (at the player)
		Clock::time_point closeStart;
		std::uint32_t     heldSteps = 0;

		struct BeginHook
		{
			// The zoom begins (once the map has loaded, a few frames after the menu opens; again when it closes): the
			// game sets its start and end. Opening (measured 2026-11-04: outdoors 184069 -> 287522): the start moves to
			// the end, so the map opens at map height.
			static void thunk(Transition* a_this)
			{
				func(a_this);
				closing = nullptr;
				if (a_this->zoomDestination.z > a_this->currentPosition.z) {
					a_this->zoomOrigin = a_this->zoomDestination;
					a_this->currentPosition = a_this->zoomDestination;
				} else if (a_this->zoomDestination.z < a_this->currentPosition.z) {
					closing = a_this;
					closeEnd = a_this->zoomDestination;
					closeStart = Clock::now();
					heldSteps = 0;
				}
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		struct UpdateHook
		{
			// Each step of the zoom. The game places the camera between the zoom's start and end by its own timer, so
			// while the map closes the end is set to the start (map height) for the game's step: its timer, the fade and
			// the hand-over to the next camera state all run as before, without the camera moving. The step that ends
			// the zoom gets the real end back.
			static void thunk(Transition* a_this, RE::BSTSmartPointer<RE::TESCameraState>& a_nextState)
			{
				if (a_this != closing) {
					func(a_this, a_nextState);
					return;
				}
				const auto held = Clock::now() - closeStart;
				if (held > kMaxHold) {
					logger::warn("Map closing: the zoom didn't end within {} s, so it runs as vanilla from here",
						std::chrono::duration_cast<std::chrono::seconds>(kMaxHold).count());
					a_this->zoomDestination = closeEnd;
					closing = nullptr;
					func(a_this, a_nextState);
					return;
				}
				a_this->zoomDestination = a_this->zoomOrigin;
				func(a_this, a_nextState);
				++heldSteps;
				if (a_nextState) {
					a_this->zoomDestination = closeEnd;
					a_this->currentPosition = closeEnd;
					closing = nullptr;
					logger::info("Map closed: camera held at map height for {} steps, {} ms", heldSteps,
						std::chrono::duration_cast<std::chrono::milliseconds>(held).count());
				}
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
		UpdateHook::func = vtbl.write_vfunc(0x3, UpdateHook::thunk);
		logger::info("The world map opens and closes at map height");
		return true;
	}
}
