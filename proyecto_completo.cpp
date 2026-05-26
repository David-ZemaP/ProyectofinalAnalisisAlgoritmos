#include "ejercicio1.h"
#include "ejercicio2.h"
#include "ejercicio3.h"

#include "ejercicio5.h"

int main() {
    string archivoAeropuertos = "airports_clean_final.csv";
    string archivoRutas = "routes_graph_validated.csv";
    string archivoAerolineas = "airlines_clean_final.csv";
    string archivoMultigrafo = "routes_multigraph_v5.csv";

    unordered_map<int, Aeropuerto> aeropuertos;
    unordered_map<int, vector<Arista>> grafoVuelos;
    unordered_map<int, Aerolinea> aerolineas;
    Multigrafo grafoAirline;
    cout << "[1/4] Cargando aeropuertos...\n";

    try {
        aeropuertos = cargarAeropuertos(archivoAeropuertos);
        cout << "  Aeropuertos cargados: " << aeropuertos.size() << "\n";
    } catch (const exception& e) {
        cout << "ERROR: " << e.what() << "\n";
        return 1;
    }

    cout << "[2/4] Construyendo grafo de rutas...\n";
    try {
        grafoVuelos = cargarRutas(archivoRutas, aeropuertos);
        int total = 0;
        for (pair<const int, vector<Arista>> p : grafoVuelos) total += p.second.size();
        cout << "  Nodos con rutas: " << grafoVuelos.size() << "\n";
        cout << "  Rutas cargadas: " << total << "\n";
    } catch (const exception& e) {
        cout << "ERROR: " << e.what() << "\n";
        return 1;
    }

    cout << "[3/4] Cargando catalogo de aerolineas...\n";
    try {
        aerolineas = cargarAerolineas(archivoAerolineas);
        cout << "  Aerolineas cargadas: " << aerolineas.size() << "\n";
    } catch (const exception& e) {
        cout << "ERROR: " << e.what() << "\n";
        cout << "  Reto 4 no estara disponible.\n";
    }

    cout << "[4/4] Cargando multigrafo con aerolineas...\n";
    try {
        grafoAirline = cargarMultigrafoAirline(archivoMultigrafo);
        int total = 0;
        for (pair<const int, vector<AristaAirline>> p : grafoAirline) total += p.second.size();
        cout << "  Nodos con rutas: " << grafoAirline.size() << "\n";
        cout << "  Rutas cargadas: " << total << "\n";
    } catch (const exception& e) {
        cout << "ERROR: " << e.what() << "\n";
        cout << "  Reto 4 no estara disponible.\n";
    }

    int opcion = 0;
    do {
        cout << "\n========================================\n";
        cout << "  MENU PRINCIPAL\n";
        cout << "========================================\n";
        cout << "  1. RETO 1: Alcance Personalizado (BFS)\n";
        cout << "  2. RETO 2: Grupos y Aislamiento Aereo\n";
        cout << "  3. RETO 3: La Maxima Eficiencia (Diametro)\n";
        cout << "  4. \"Ejercicio 5\": El Pasajero Leal\n";
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
            case 4: Ejercicio5(aeropuertos, aerolineas, grafoAirline); break;
            case 0: cout << "Saliendo...\n"; break;
            default: cout << "Opcion invalida.\n";
        }
    } while (opcion != 0);

    return 0;
}
