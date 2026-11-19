# Reduce Motion Map

The world map opens and closes without the camera flying. Opening, it fades in at map height instead of flying up from
your character; closing, it fades to black at map height instead of diving back down to you.

Skyrim's map camera zooms between your character and map height each time the map opens or closes. Outdoors the
opening zoom starts at your character and flies up; closing the world map (indoors or out) dives back down while the
map darkens. This SKSE plugin keeps the camera at map height for both: the opening zoom starts where it ends, and the
closing zoom is held at map height until it ends, behind the game's own fade to black. The map shows exactly the same
view as before, and the game's own fades and timing stay as they are. Zooming, panning, the local map and fast travel
are untouched.

No settings, no plugin file, and nothing runs while the map is closed.

## Requirements

- Skyrim Special Edition 1.5.97 or Anniversary Edition 1.6.1170 / 1.7.x (one DLL for both)
- SKSE64
- Address Library for SKSE Plugins

## Compatibility

It changes only the map camera's opening and closing zoom. Mods that change the map's look (map textures, weather,
markers) work alongside it. A mod that hooks the same zoom may conflict.

## Building

`skse\build.bat` (VS Build Tools with CMake, Ninja and vcpkg; see `skse\CMakePresets.json`). The DLL is copied to
`mod\SKSE\Plugins`.

## License

GPL-3.0 (`skse/LICENSE`): CommonLibSSE-NG is GPL-3.0-or-later and statically linked.
