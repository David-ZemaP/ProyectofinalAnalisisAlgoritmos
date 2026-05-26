#include "ejercicio3.h"

struct ResultadoDiametro {
    int nodoOrigen;
    int nodoDestino;
    double distancia;
};

static ResultadoDiametro calcularDiametroSCC(
    const vector<int>& scc,
    const unordered_map<int, vector<Arista>>& grafo)
{
    int n = scc.size();

    unordered_map<int, int> indice;
    indice.reserve(n);
    for (int i = 0; i < n; i++) {
        indice[scc[i]] = i;
    }

    int nodoA = scc[0];
    int nodoB = scc[0];
    double maxDist = 0.0;

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
    }

    return {nodoA, nodoB, maxDist};
}

static void mostrarAeropuerto(const Aeropuerto& ap) {
    cout << ap.nombre;
    if (!ap.iata.empty() && ap.iata != "\\N") {
        cout << " (" << ap.iata << ")";
    }
    cout << " - " << ap.ciudad << ", " << ap.pais;
}

void Ejercicio3(const unordered_map<int, Aeropuerto>& aeropuertos,
                const unordered_map<int, vector<Arista>>& grafo)
{
    auto componentes = encontrarSCCs(grafo);

    if (componentes.empty()) {
        cout << "No hay conexiones en el grafo.\n";
        return;
    }

    int nodoA = -1, nodoB = -1;
    double maxDist = 0.0;

    for (const auto& scc : componentes) {
        if (scc.size() < 2) continue;

        auto [origen, destino, dist] = calcularDiametroSCC(scc, grafo);

        if (dist > maxDist) {
            maxDist = dist;
            nodoA = origen;
            nodoB = destino;
        }
    }

    if (nodoA == -1 || nodoB == -1 || maxDist == 0.0) {
        cout << "No se encontraron pares de aeropuertos conectados.\n";
        return;
    }

    auto apA = aeropuertos.find(nodoA);
    auto apB = aeropuertos.find(nodoB);

    if (apA != aeropuertos.end() && apB != aeropuertos.end()) {
        cout << "=== RETO 3: LA MAXIMA EFICIENCIA (DIAMETRO) ===\n\n";
        cout << "Aeropuerto A: "; mostrarAeropuerto(apA->second); cout << "\n";
        cout << "Aeropuerto B: "; mostrarAeropuerto(apB->second); cout << "\n";
        cout << "Distancia total por rutas: " << fixed << setprecision(0)
             << maxDist << " km\n";
    } else {
        cout << "Error: no se pudieron recuperar los aeropuertos del resultado.\n";
    }
}
