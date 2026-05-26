#include "ejercicio3.h"

void Ejercicio3(const unordered_map<int, Aeropuerto>& aeropuertos,
                const unordered_map<int, vector<Arista>>& grafo) {
 
    auto componentes = encontrarSCCs(grafo);

    if (componentes.empty() || componentes[0].size() < 2) {
        cout << "No hay suficientes conexiones.\n";
        return;
    }

    const vector<int>& scc = componentes[0];
    int n = scc.size();

    cout << "SCC mas grande: " << n << " aeropuertos\n";
    cout << "Calculando diametro exacto (Dijkstra desde cada nodo)...\n";
    cout << "  Nodos a procesar: " << n << "\n";

    unordered_map<int, int> indice;
    for (int i = 0; i < n; i++) {
        indice[scc[i]] = i;
    }

    int nodoA = -1;
    int nodoB = -1;
    double maxDist = 0.0;

    auto inicio = high_resolution_clock::now();
    int procesados = 0;
    int proximoReporte = 5;

    for (int origen : scc) {
        vector<double> dist(n, numeric_limits<double>::max());
        priority_queue<
            pair<double, int>,
            vector<pair<double, int>>,
            greater<pair<double, int>>
        > pq;

        dist[indice[origen]] = 0.0;
        pq.push({0.0, origen});

        while (!pq.empty()) {
            auto [distActual, actual] = pq.top();
            pq.pop();

            auto itActual = indice.find(actual);
            if (itActual == indice.end()) continue;

            int idxActual = itActual->second;
            if (distActual > dist[idxActual]) continue;

            auto itGrafo = grafo.find(actual);
            if (itGrafo == grafo.end()) continue;

            for (const Arista& arista : itGrafo->second) {
                auto itDestino = indice.find(arista.idDestino);
                if (itDestino == indice.end()) continue;

                int idxDestino = itDestino->second;
                double nuevaDist = distActual + arista.distancia;

                if (nuevaDist < dist[idxDestino]) {
                    dist[idxDestino] = nuevaDist;
                    pq.push({nuevaDist, arista.idDestino});
                }
            }
        }

        for (int i = 0; i < n; i++) {
            if (scc[i] != origen && dist[i] > maxDist) {
                maxDist = dist[i];
                nodoA = origen;
                nodoB = scc[i];
            }
        }

        procesados++;
        int porcentaje = (procesados * 100) / n;

        if (porcentaje >= proximoReporte) {
            cout << "\r  Progreso: " << porcentaje
                 << "% (" << procesados << "/" << n << ")" << flush;
            proximoReporte += 5;
        }
    }

    cout << "\r  Progreso: 100% (" << n << "/" << n << ")\n";

    auto duracion = duration_cast<seconds>(
        high_resolution_clock::now() - inicio
    ).count();

    auto apA = aeropuertos.find(nodoA);
    auto apB = aeropuertos.find(nodoB);

    auto mostrarAeropuerto = [](const auto& it) {
        cout << it->second.nombre;

        if (!it->second.iata.empty() && it->second.iata != "\\N") {
            cout << " (" << it->second.iata << ")";
        }

        cout << " - " << it->second.ciudad << ", " << it->second.pais;
    };

    cout << "\nRESULTADO:\n";
    cout << "  Par mas distante:\n";

    if (apA != aeropuertos.end() && apB != aeropuertos.end()) {
        cout << "    A: ";
        mostrarAeropuerto(apA);
        cout << "\n";

        cout << "    B: ";
        mostrarAeropuerto(apB);
        cout << "\n";

        cout << "  Distancia: " << fixed << setprecision(0)
             << maxDist << " km\n";

        double directa = calcularHaversine(
            apA->second.latitud,
            apA->second.longitud,
            apB->second.latitud,
            apB->second.longitud
        );

        cout << "  Linea recta: " << fixed << setprecision(0)
             << directa << " km\n";
    } else {
        cout << "    No se pudieron recuperar los aeropuertos del resultado.\n";
    }

    cout << "  Tiempo total: " << duracion << " s\n";
}