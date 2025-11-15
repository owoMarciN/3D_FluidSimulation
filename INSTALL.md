## Instalacja

Ten projekt wymaga wielu bibliotek, z których część dla wygody postarałem się uwzględnić już w samym projekcie m.in:
- ```Glad``` (Nagłówków OpenGL),
- ```GLM``` (Matematyka OpenGL),
- ```tiny_object_loader.h``` (Załadowywanie plików modeli 3D o rozszerzeniu .obj).

Jedank pozostaje jedna biblioteka, która sprawia niemałe trudności -- SDL3. \
Poniżej spróbuję opisać jak przeszedłem przez proces instalacji dla systemów Windows i Linux.

## Windows -- MSYS2 UCRT64

1. Pobierz ```MSYS2```: https://www.msys2.org/
	- **Głównie dla wygody oraz by móc kompilować za pomocą gcc/g++ na Windowsie.**
	- **Miej na uwadze, że na Windowsie ścieżki do bibliotek mogą różnić się w zależności od instalacji MSYS2.**

3. W swoim systemie wyszukaj ```UCRT64``` i uruchom w trybie administratora.

4. Zaktualizuj za pomocą:

```bash
	pacman -Syu
```

4. Po restarcie zainstaluj potrzebne narzędzia i ```SDL3```:

```bash
	pacman -S mingw-w64-ucrt-x86_64-toolchain mingw-w64-ucrt-x86_64-sdl3
```

5. Skompiluj projekt:

```bash
	#Sklonuj repo
	git clone https://github.com/owoMarciN/3D_FluidSimulation.git

	#Przejdź do folderu
	cd 3D_FluidSimulation

	#Skompiluj projekt
	g++ src/*.cpp src/*c -Iinclude -L/ucrt64/lib -o turbine -lSDL3 -lopengl32

	#Przekopiuj SDL3.dll obok pliku .exe
	cp /ucrt64/bin/SDL3.dll .

	#Uruchom
	./turbine.exe
```

## Linux

1. Pobierz i rozpakuj ```SourceCode.zip```: https://github.com/libsdl-org/SDL/releases

2. Wejdź do folderu ```SDL3-*``` i utwórz folder build:

```bash
	mkdir build
	cd build

	#Skompiluj i zainstaluj:
	cmake .. -DCMAKE_BUILD_TYPE=Release
	cmake --build . --config Release
	sudo cmake --install . --config Release
```

3. Dodaj ścieżkę do bibliotek do zmiennej ```LD_LIBRARY_PATH```:

```bash
	nano ~/.bashrc
```
Wpisz na końcu pliku:

```bash
	export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
```

Zastosuj zmiany:
```bash
	source ~/.bashrc

	#Aby się upewnić
	sudo ldconfig
```

4. Skompiluj projekt (przykład dla pulpitu):

```bash
	#Sklonuj repo
	git clone https://github.com/owoMarciN/3D_FluidSimulation.git

	#Przejdź do folderu
	cd 3D_FluidSimulation

	#Skompiluj projekt
	g++ src/*.cpp src/*.c -Iinclude -L/usr/local/lib -o turbine -lSDL3 -lGL

	#Uruchom
	./turbine
```

	






