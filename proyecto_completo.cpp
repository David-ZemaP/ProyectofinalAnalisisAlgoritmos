/*
 * PROYECTO FINAL - ANALISIS DE ALGORITMOS
 * Analisis de la Red Mundial de Vuelos
 *
 * Compilacion: g++ -std=c++17 -O2 *.cpp -o proyecto_completo
 *
 * Archivos:
 *   common.h / common.cpp   - Estructuras compartidas y carga de datos
 *   ejercicio1.h/cpp        - RETO 1: Alcance Personalizado (BFS)
 *   ejercicio2.h/cpp        - RETO 2: Grupos y Aislamiento Aereo
 *   ejercicio3.h/cpp        - RETO 3: Diametro del grafo (Dijkstra)
 *   ejercicio4.h/cpp        - RETO 4: Red Minima de Sudamerica (MST)
 *   proyecto_completo.cpp   - Solo el main con menu interactivo
 */

#include "ejercicio1.h"
#include "ejercicio2.h"
#include "ejercicio3.h"
#include "ejercicio4.h"

int main() {
    string archivoAeropuertos = "airports_clean_final.csv";
    string archivoRutas = "routes_graph_validated.csv";

    unordered_map<int, Aeropuerto> aeropuertos;
    unordered_map<int, vector<Arista>> grafoVuelos;
    cout << "[1/2] Cargando aeropuertos...\n";
    try {
        aeropuertos = cargarAeropuertos(archivoAeropuertos);
        cout << "  Aeropuertos cargados: " << aeropuertos.size() << "\n";
    } catch (const exception& e) {
        cout << "ERROR: " << e.what() << "\n";
        return 1;
    }

    cout << "[2/2] Construyendo grafo de rutas...\n";
    try {
        grafoVuelos = cargarRutas(archivoRutas, aeropuertos);
        int total = 0;
        for (const auto& p : grafoVuelos) total += p.second.size();
        cout << "  Nodos con rutas: " << grafoVuelos.size() << "\n";
        cout << "  Rutas cargadas: " << total << "\n";
    } catch (const exception& e) {
        cout << "ERROR: " << e.what() << "\n";
        return 1;
    }

    int opcion = 0;
    do {
        cout << "\n========================================\n";
        cout << "  MENU PRINCIPAL\n";
        cout << "========================================\n";
        cout << "  1. RETO 1: Alcance Personalizado (BFS)\n";
        cout << "  2. RETO 2: Grupos y Aislamiento Aereo\n";
        cout << "  3. RETO 3: La Maxima Eficiencia (Diametro)\n";
        cout << "  4. RETO 4: Red Minima de Sudamerica (MST)\n";
        cout << "  0. Salir\n";
        cout << "========================================\n";
        cout << "Seleccione una opcion: ";

        if (!(cin >> opcion)) {
            if (cin.eof()) break;
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Entrada invalida.\n";
            continue;
        }

        switch (opcion) {
            case 1: Ejercicio1(aeropuertos, grafoVuelos); break;
            case 2: Ejercicio2(aeropuertos, grafoVuelos); break;
            case 3: Ejercicio3(aeropuertos, grafoVuelos); break;
            case 4: Ejercicio4(aeropuertos, grafoVuelos); break;
            case 0: cout << "Saliendo...\n"; break;
            default: cout << "Opcion invalida.\n";
        }
    } while (opcion != 0);

    return 0;
}
