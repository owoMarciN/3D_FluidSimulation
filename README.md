# 3D Fluid Simulation
Na razie w początkowej fazie próba zbudowania symulacji cieczy szczególnie kładąc nacisk na zjawisko [kawitacji](https://pl.wikipedia.org/wiki/Kawitacja). Jak na razie udało mi się stworzyć podstawowy program symulacji mogący załadować plik modelu 3D o rozszerzeniu .obj i wyrenderowanie go w oknie.

W projekcie zostały użyte następujace biblioteki:
- ```Glad``` (Nagłówki OpenGL)
- ```GLM```  (Matematyka OpenGL)
- ```SDL3``` (Zarządzanie wejściem/wyjściem oraz tworzenie okna)
- ```tiny_object_loader``` (Załadowanie plików modeli 3D o rozszerzeniu .obj)

## RoadMap

### Faza 0: Rozpoczecie pracy
- [x] Tworzenie okna za pomocą biblioteki SDL3
- [x] Klasa ```Simulation```
- [x] Klasa ```Timer```
- [x] Załadowanie obiektu 3D z Blendera za pomocą ```tiny_object_loader```
- [x] Wyrenderowanie załadowanego obiektu 3D
- [x] Odczytywnie shaderów z plików ```.glsl```
- [x] Kompilacja shaderów
- [x] Ruch kamery w oknie symulacji
- [x] Obrót obiektu wokół osi
- [x] Wskaźnik ```FPS```
- [ ] Komentarze i opis funkcji

### Faza 1: Stworzenie rozwiązania dla symulacji (
Implementcja prostej symulacji [Fluid Simulation For Dummies](https://www.mikeash.com/pyblog/fluid-simulation-for-dummies.html)
- [x] Stworzenie klasy ```FluidCube```
- [ ] Renderowanie gęstości ```barwnika```
- [ ] Renderowanie pól wektoryowych prędkości
- [ ] Zaznaczanie komórek siatki wokół obiektu
- [ ] Symulacja przepływu w rurze
- [ ] Interakcja ```ciecz-model_3D```, ```ciecz-grawitacja```

### Faza 2: Kawitacja
- [ ] Implementacja pola ciśnienia
- [ ] Dodanie ułamka pary alpha
- [ ] Obliczenie lokalnych minimów ciśnienia
- [ ] Identyfikacja stref niskiego ciśnienia
- [ ] Renderowanie stref w wyróżniających się kolorach

### Faza 3: Rayleigh–Plesset i Bąbelki
- [ ] Implementacja klasy ```Bubbles```
- [ ] Integracja równania RP metodą ```Eulera```
- [ ] Interpolacja ciśnienia z siatki do cząstek
- [ ] Test rozrostu bąbelków w strefie niskiego ciśnienia

### Faza 4: Babęlki oddziałujące na przepływ

Ciąg dalszy nastąpi...




