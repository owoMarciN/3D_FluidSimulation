# 3D Fluid Simulation
Na razie w początkowej fazie próba zbudowania symulacji cieczy szczególnie kładąc nacisk na zjawisko [kawitacji](https://pl.wikipedia.org/wiki/Kawitacja). Jak na razie udało mi się stworzyć podstawowy program symulacji mogący załadować plik modelu 3D o rozszerzeniu .obj i wyrenderowanie go w oknie.

Jak na razie w porjekcie zostały użyte następujace biblioteki:
- ```Glad``` (Nagłówki OpenGL)
- ```GLM```  (Matematyka OpenGL)
- ```SDL3``` (Zarządzanie wejściem/wyjściem oraz tworzenie okna)
- ```tiny_object_loader``` (Załadowanie plików modeli 3D o rozszerzeniu .obj)

## RoadMap

Faza 0: Rozpoczecie pracy
- [] Tworzenie okna za pomocą SDL3
- [] Klasa Simulation
- [] Klasa Timer
- [] Załadowanie obiektu 3D z Blendera za pomocą ```tiny_object_loader```
- [] Wyrenderowanie załadowanego obiektu 3D
- [] Ruch kamery w oknie symulacji

