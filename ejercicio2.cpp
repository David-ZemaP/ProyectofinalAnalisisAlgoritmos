#include "ejercicio2.h"

void Ejercicio2(const unordered_map<int, Aeropuerto>& aeropuertos,
                const unordered_map<int, vector<Arista>>& grafo) {
    cout << "\n========================================\n";
    cout << "   RETO 2: GRUPOS Y AISLAMIENTO AEREO\n";
    cout << "========================================\n";
    cout << "   (Componentes Fuertemente Conexos - SCC)\n";
    cout << "========================================\n";

    auto componentes = encontrarSCCs(grafo);
    if (componentes.empty()) { cout << "No hay conexiones.\n"; return; }

    int masGrande = componentes[0].size();
    int segundo = componentes.size() > 1 ? componentes[1].size() : 0;

    // Contar aeropuertos en el grafo de rutas
    int totalEnGrafo = 0;
    for (const auto& comp : componentes) totalEnGrafo += comp.size();

    int gruposAislados = 0, nodosSolitarios = 0;
    for (size_t i = 1; i < componentes.size(); i++) {
        if (componentes[i].size() >= 2) gruposAislados++;
        else nodosSolitarios++;
    }

    cout << "\nRESULTADOS (SCC sobre grafo dirigido):\n";
    cout << "  SCC mas grande: " << masGrande << " aeropuertos\n";
    cout << "  Segundo SCC mas grande: " << segundo << " aeropuertos\n";
    cout << "  Grupos aislados (SCC con >= 2 nodos): " << gruposAislados << "\n";
    cout << "  Nodos sin conexion reciproca (SCC de 1 nodo): " << nodosSolitarios << "\n";
    cout << "  Total nodos en el grafo de rutas: " << totalEnGrafo << "\n";

    // Mostrar ejemplo del segundo SCC si existe
    if (segundo >= 2) {
        cout << "\nEjemplo de grupo aislado (segundo SCC):\n";
        for (int id : componentes[1]) {
            auto it = aeropuertos.find(id);
            if (it != aeropuertos.end())
                cout << "  - " << it->second.nombre << " (" << it->second.iata
                     << ") - " << it->second.ciudad << ", " << it->second.pais << "\n";
        }
    }
}
