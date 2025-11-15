## Instalacja

Ten projekt wymaga wielu bibliotek, z których część dla wygody postarałem się uwzględnić już w samym projekcie m.in:
 - Glad -- (Załadować nagłówków OpenGL)
 - GLM -- (Biblioteka matematyczna OpenGL)
 - tiny_object_loader.h -- (Załadowywaniu modeli o rozszerzeniu .obj)

Jedank pozostaje jedna biblioteka, która sprawia niemałe trudności SDL3. Poniżej spróbuję opisać jak przeszedłem przez proces instalacji dla systemów Windows i Linux:

## Windows -- MSYS2 UCRT64

1. Pobierz MSYS2: https://www.msys2.org/ \
   Miej na uwadze, że na Windowsie ścieżki do bibliotek mogą różnić się w zależności od instalacji MSYS2

2. W swoim systemie wyszukaj „UCRT64” i uruchom w trybie administratora.

3. Zaktualizuj za pomocą:

```bash
	pacman -Syu
```

4. Po restarcie zainstaluj potrzebne narzędzia i SDL3:

```bash
	pacman -S mingw-w64-ucrt-x86_64-toolchain mingw-w64-ucrt-x86_64-sdl3
```

5. Skompiluj projektu (przykład dla pulpitu):

```bash
	#Przejdź do folderu
	cd ~/Desktop/3D_FluidSimulation-main

	#Skompiluj projekt
	g++ src/*.cpp src/*c -Iinclude -L/ucrt64/lib -o turbine -lSDL3 -lopengl32

	#Przekopiuj SDL3.dll obok pliku .exe
	cp /ucrt64/bin/SDL3.dll build/

	#Uruchom
	./build/turbine.exe
```

## Linux

1. Pobierz i rozpakuj SDL3: https://github.com/libsdl-org/SDL/releases

2. Wejdź do folderu SDL3 i utwórz folder build:

```bash
	mkdir build
	cd build
```

3. Skompiluj i zainstaluj:

```bash
	cmake .. -DCMAKE_BUILD_TYPE=Release
	cmake --build . --config Release
	sudo cmake --install . --config Release
```

4. Dodaj ścieżkę do bibliotek do zmiennej LD_LIBRARY_PATH:

```bash
	nano ~/.bashrc
```
Wpisz na końcu:

```bash
	export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
```

Zastosuj zmiany:
```bash
	source ~/.bashrc

	#By się upewnić
	sudo ldconfig
```

5. Skompiluj projekt (przykład dla pulpitu):

```bash
	cd ~/Desktop/3D_FluidSimulation-main

	g++ src/*.cpp src/*.c -Iinclude -L/usr/local/lib -o turbine -lSDL3 -lGL

	./turbine
```







