#include "ejercicio3.h"

void Ejercicio3(const unordered_map<int, Aeropuerto>& aeropuertos,
                const unordered_map<int, vector<Arista>>& grafo) {
    cout << "\n========================================\n";
    cout << "   RETO 3: LA MAXIMA EFICIENCIA (DIAMETRO EXACTO)\n";
    cout << "========================================\n";

    auto componentes = encontrarSCCs(grafo);
    if (componentes.empty() || componentes[0].size() < 2) {
        cout << "No hay suficientes conexiones.\n"; return;
    }

    const vector<int>& sccGigante = componentes[0];
    int N = sccGigante.size();
    cout << "SCC mas grande: " << N << " aeropuertos\n";
    cout << "Calculando diametro exacto (Dijkstra desde cada nodo)...\n";
    cout << "  Nodos a procesar: " << N << "\n";

    // Mapear node_id -> indice 0..N-1 (evita unordered_map en cada Dijkstra)
    unordered_map<int, int> idAIndice;
    for (int i = 0; i < N; i++)
        idAIndice[sccGigante[i]] = i;

    int nodoA = -1, nodoB = -1;
    double maxDist = 0;
    auto inicio = high_resolution_clock::now();
    int progreso = 0, proxReporte = 1;

    // Dijkstra desde cada nodo del SCC gigante
    for (int fuente : sccGigante) {
        // Distancias indexadas por posicion en sccGigante
        vector<double> dist(N, numeric_limits<double>::max());
        dist[idAIndice[fuente]] = 0;
        using Par = pair<double, int>;
        priority_queue<Par, vector<Par>, greater<Par>> pq;
        pq.push({0, fuente});

        while (!pq.empty()) {
            auto [d, nodo] = pq.top(); pq.pop();
            // Solo nos importan las distancias hacia nodos del SCC
            auto itMap = idAIndice.find(nodo);
            int idxAct = (itMap != idAIndice.end()) ? itMap->second : -1;
            if (idxAct != -1 && d > dist[idxAct]) continue;
            if (idxAct == -1 && d == 0) continue; // nodo fuera del SCC recien empezando

            auto itG = grafo.find(nodo);
            if (itG == grafo.end()) continue;
            for (const Arista& ar : itG->second) {
                double nd = d + ar.distancia;
                auto itDest = idAIndice.find(ar.idDestino);
                if (itDest == idAIndice.end()) continue; // solo importa SCC
                int idxDest = itDest->second;
                if (nd < dist[idxDest]) {
                    dist[idxDest] = nd;
                    pq.push({nd, ar.idDestino});
                }
            }
        }

        // Encontrar nodo mas lejano dentro del SCC desde esta fuente
        for (int i = 0; i < N; i++) {
            if (sccGigante[i] == fuente) continue;
            if (dist[i] > maxDist) {
                maxDist = dist[i];
                nodoA = fuente;
                nodoB = sccGigante[i];
            }
        }

        progreso++;
        int pct = (progreso * 100) / N;
        if (pct >= proxReporte) {
            cout << "\r  Progreso: " << pct << "% (" << progreso << "/" << N << ")" << flush;
            proxReporte = pct + 5;
        }
    }
    cout << "\r  Progreso: 100% (" << N << "/" << N << ")\n";

    auto duracion = duration_cast<seconds>(high_resolution_clock::now() - inicio).count();

    auto apA = aeropuertos.find(nodoA);
    auto apB = aeropuertos.find(nodoB);

    auto mostrarAP = [](const auto& it) {
        cout << it->second.nombre;
        if (!it->second.iata.empty() && it->second.iata != "\\N")
            cout << " (" << it->second.iata << ")";
        cout << " - " << it->second.ciudad << ", " << it->second.pais;
    };

    cout << "\nRESULTADO:\n";
    cout << "  Par mas distante:\n";
    cout << "    A: "; mostrarAP(apA); cout << "\n";
    cout << "    B: "; mostrarAP(apB); cout << "\n";
    cout << "  Distancia: " << fixed << setprecision(0) << maxDist << " km\n";
    if (apA != aeropuertos.end() && apB != aeropuertos.end()) {
        double directa = calcularHaversine(apA->second.latitud, apA->second.longitud,
                                           apB->second.latitud, apB->second.longitud);
        cout << "  Linea recta: " << fixed << setprecision(0) << directa << " km\n";
    }
    cout << "  Tiempo total: " << duracion << " s\n";
}
