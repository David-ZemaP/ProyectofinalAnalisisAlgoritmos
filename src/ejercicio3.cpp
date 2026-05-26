#include "ejercicio3.h"

typedef pair<double, int> ParDist;

static void calcularDiametroSCC(
    const vector<int>& scc,
    const unordered_map<int, vector<Arista>>& grafo,
    int& nodoA, int& nodoB, double& maxDist)
{
    int n = (int)scc.size();

    unordered_map<int, int> indice;
    for (int i = 0; i < n; i++) {
        indice[scc[i]] = i;
    }

    nodoA = scc[0];
    nodoB = scc[0];
    maxDist = 0.0;

    for (int iOrigen = 0; iOrigen < n; iOrigen++) {
        int origen = scc[iOrigen];
        vector<double> dist(n, numeric_limits<double>::max());
        priority_queue<ParDist, vector<ParDist>, greater<ParDist>> pq;

        dist[indice[origen]] = 0.0;
        pq.push({0.0, origen});

        while (!pq.empty()) {
            double distActual = pq.top().first;
            int actual = pq.top().second;
            pq.pop();

            if (indice.count(actual) == 0) continue;
            int idxActual = indice[actual];
            if (distActual > dist[idxActual]) continue;

            if (grafo.count(actual) == 0) continue;
            const vector<Arista>& aristas = grafo.at(actual);
            for (int i = 0; i < (int)aristas.size(); i++) {
                int idDestino = aristas[i].idDestino;
                if (indice.count(idDestino) == 0) continue;

                int idxDestino = indice[idDestino];
                double nuevaDist = distActual + aristas[i].distancia;

                if (nuevaDist < dist[idxDestino]) {
                    dist[idxDestino] = nuevaDist;
                    pq.push({nuevaDist, idDestino});
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
    vector<vector<int>> componentes = encontrarSCCs(grafo);

    if (componentes.empty()) {
        cout << "No hay conexiones en el grafo.\n";
        return;
    }

    int nodoA = -1, nodoB = -1;
    double maxDist = 0.0;

    for (int i = 0; i < (int)componentes.size(); i++) {
        const vector<int>& scc = componentes[i];
        if ((int)scc.size() < 2) continue;

        int auxNodoA, auxNodoB;
        double auxDist;
        calcularDiametroSCC(scc, grafo, auxNodoA, auxNodoB, auxDist);

        if (auxDist > maxDist) {
            maxDist = auxDist;
            nodoA = auxNodoA;
            nodoB = auxNodoB;
        }
    }

    if (nodoA == -1 || nodoB == -1 || maxDist == 0.0) {
        cout << "No se encontraron pares de aeropuertos conectados.\n";
        return;
    }

    if (aeropuertos.count(nodoA) > 0 && aeropuertos.count(nodoB) > 0) {
        const Aeropuerto& apA = aeropuertos.at(nodoA);
        const Aeropuerto& apB = aeropuertos.at(nodoB);
        cout << "=== RETO 3: LA MAXIMA EFICIENCIA (DIAMETRO) ===\n\n";
        cout << "Aeropuerto A: "; mostrarAeropuerto(apA); cout << "\n";
        cout << "Aeropuerto B: "; mostrarAeropuerto(apB); cout << "\n";
        cout << "Distancia total por rutas: " << fixed << setprecision(0)
             << maxDist << " km\n";
    } else {
        cout << "Error: no se pudieron recuperar los aeropuertos del resultado.\n";
    }
}
