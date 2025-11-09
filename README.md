# Pivoine
A data generation utility for Collective Unconscious. It can generate data for the following subsystems:

- Record Player (slot data, max. track count, descriptions)
- Covers (full animation, preview, name and author)
- Menu Themes (preview, name and author)

## Building
Requires `Qt6` and uses CMake for building. `liblcf` will be built as part of the process, unless specified otherwise.

```
git clone https://github.com/lumiscosity/pivoine
cd pivoine
git submodule update --init
cmake -B builddir
cmake --build builddir
```

On Linux, you can build a Flatpak:

```
flathub-builder linux/flatpak-build linux/com.github.lumiscosity.Pivoine.yml --repo linux/flatpak-repo --install-deps-from=flathub
flatpak build-bundle linux/flatpak-repo linux/pivoine.flatpak com.github.lumiscosity.Pivoine --runtime-repo=https://flathub.org/repo/flathub.flatpakrepo
flatpak install linux/pivoine.flatpak
```

## License
Pivoine is free software licensed under GPLv3. The icon is ARR.
