#include "ejercicio2.h"

void Ejercicio2(const unordered_map<int, Aeropuerto>& aeropuertos,
                const unordered_map<int, vector<Arista>>& grafo) {
    cout << "   (Componentes Fuertemente Conexos - SCC)\n";
    cout << "========================================\n";

    vector<vector<int>> componentes = encontrarSCCs(grafo);
    if (componentes.empty()) { cout << "No hay conexiones.\n"; return; }

    int masGrande = componentes[0].size();
    int segundo = componentes.size() > 1 ? componentes[1].size() : 0;

    int totalEnGrafo = 0;
    for (int i = 0; i < (int)componentes.size(); i++)
        totalEnGrafo += componentes[i].size();

    int gruposAislados = 0, nodosSolitarios = 0;
    for (int i = 1; i < (int)componentes.size(); i++) {
        if (componentes[i].size() >= 2) gruposAislados++;
        else nodosSolitarios++;
    }

    cout << "\nRESULTADOS (SCC sobre grafo dirigido):\n";
    cout << "  SCC mas grande: " << masGrande << " aeropuertos\n";
    cout << "  Segundo SCC mas grande: " << segundo << " aeropuertos\n";
    cout << "  Grupos aislados (SCC con >= 2 nodos): " << gruposAislados << "\n";
    cout << "  Nodos sin conexion reciproca (SCC de 1 nodo): " << nodosSolitarios << "\n";
    cout << "  Total nodos en el grafo de rutas: " << totalEnGrafo << "\n";

    if (segundo >= 2) {
        cout << "\nEjemplo de grupo aislado (segundo SCC):\n";
        for (int i = 0; i < (int)componentes[1].size(); i++) {
            int id = componentes[1][i];
            if (aeropuertos.count(id) > 0) {
                const Aeropuerto& ap = aeropuertos.at(id);
                cout << "  - " << ap.nombre << " (" << ap.iata
                     << ") - " << ap.ciudad << ", " << ap.pais << "\n";
            }
        }
    }
}
