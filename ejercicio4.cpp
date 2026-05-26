#include "ejercicio4.h"

static unordered_map<int, vector<pair<int, double>>> filtrarSudamerica(
    const unordered_map<int, Aeropuerto>& aeropuertos,
    const unordered_map<int, vector<Arista>>& grafo)
{
    unordered_set<string> paisesSA = {
        "Argentina", "Bolivia", "Brazil", "Chile", "Colombia", "Ecuador",
        "Guyana", "Paraguay", "Peru", "Suriname", "Uruguay",
        "Venezuela", "French Guiana"
    };

    unordered_set<int> esSA;
    esSA.reserve(aeropuertos.size());

    for (const auto& [id, ap] : aeropuertos) {
        if (paisesSA.count(ap.pais)) {
            esSA.insert(id);
        }
    }

    unordered_map<int, vector<pair<int, double>>> grafoSA;

    for (const auto& [idOrigen, aristas] : grafo) {
        if (!esSA.count(idOrigen)) continue;

        for (const Arista& ar : aristas) {
            if (!esSA.count(ar.idDestino)) continue;

            grafoSA[idOrigen].push_back({ar.idDestino, ar.distancia});
            grafoSA[ar.idDestino].push_back({idOrigen, ar.distancia});
        }
    }

    return grafoSA;
}

static vector<int> componenteMasGrande(
    const unordered_map<int, vector<pair<int, double>>>& grafoSA)
{
    unordered_set<int> visit;
    vector<vector<int>> componentes;

    for (const auto& [inicio, _ign] : grafoSA) {
        if (visit.count(inicio)) continue;

        vector<int> comp;
        queue<int> q;
        q.push(inicio);
        visit.insert(inicio);

        while (!q.empty()) {
            int actual = q.front();
            q.pop();
            comp.push_back(actual);

            auto it = grafoSA.find(actual);
            if (it == grafoSA.end()) continue;

            for (const auto& [vecino, _ign2] : it->second) {
                if (!visit.count(vecino)) {
                    visit.insert(vecino);
                    q.push(vecino);
                }
            }
        }

        componentes.push_back(move(comp));
    }

    sort(componentes.begin(), componentes.end(),
         [](const auto& a, const auto& b) { return a.size() > b.size(); });

    if (componentes.empty()) return {};
    return move(componentes[0]);
}

struct ResultadoMST {
    vector<AristaMST> aristas;
    double pesoTotal;
};

static ResultadoMST calcularMST(
    const vector<int>& nodos,
    const unordered_map<int, vector<pair<int, double>>>& grafoSA,
    const unordered_map<int, Aeropuerto>& aeropuertos)
{
    int n = nodos.size();

    unordered_map<int, int> idAIdx;
    idAIdx.reserve(n);
    for (int i = 0; i < n; i++) {
        idAIdx[nodos[i]] = i;
    }

    vector<bool> enMST(n, false);
    vector<double> minDist(n, numeric_limits<double>::max());
    vector<int> padre(n, -1);

    using Par = pair<double, int>;
    priority_queue<Par, vector<Par>, greater<Par>> pq;

    minDist[idAIdx[nodos[0]]] = 0.0;
    pq.push({0.0, nodos[0]});

    while (!pq.empty()) {
        auto [distActual, nodo] = pq.top();
        pq.pop();

        int u = idAIdx[nodo];
        if (enMST[u]) continue;

        enMST[u] = true;

        auto it = grafoSA.find(nodo);
        if (it == grafoSA.end()) continue;

        for (const auto& [vecino, distancia] : it->second) {
            int v = idAIdx[vecino];

            if (!enMST[v] && distancia < minDist[v]) {
                minDist[v] = distancia;
                padre[v] = u;
                pq.push({distancia, vecino});
            }
        }
    }

    ResultadoMST res;
    res.pesoTotal = 0.0;

    for (int v = 0; v < n; v++) {
        if (padre[v] == -1) continue;

        int u = padre[v];
        res.pesoTotal += minDist[v];

        const auto& apU = aeropuertos.at(nodos[u]);
        const auto& apV = aeropuertos.at(nodos[v]);

        res.aristas.push_back({
            nodos[u], nodos[v], minDist[v],
            apU.nombre, apV.nombre, apU.iata, apV.iata
        });
    }

    sort(res.aristas.begin(), res.aristas.end(),
         [](const AristaMST& a, const AristaMST& b) {
             return a.peso < b.peso;
         });

    return res;
}

void Ejercicio4(const unordered_map<int, Aeropuerto>& aeropuertos,
                const unordered_map<int, vector<Arista>>& grafo)
{
    auto grafoSA = filtrarSudamerica(aeropuertos, grafo);

    if (grafoSA.size() < 2) {
        cout << "No hay suficientes rutas entre aeropuertos sudamericanos.\n";
        return;
    }

    auto compGigante = componenteMasGrande(grafoSA);
    int n = compGigante.size();

    if (n < 2) {
        cout << "No hay suficientes aeropuertos conectados.\n";
        return;
    }

    auto mst = calcularMST(compGigante, grafoSA, aeropuertos);

    cout << "=== RETO 4: RED MINIMA DE SUDAMERICA ===\n\n";
    cout << "Aeropuertos conectados: " << n << "\n\n";
    cout << "Red de rutas minima (ordenada por distancia):\n";

    for (size_t i = 0; i < mst.aristas.size(); i++) {
        const auto& ar = mst.aristas[i];
        cout << "  " << (i + 1) << ". "
             << ar.nombreU << " (" << ar.iataU << ")  --  "
             << ar.nombreV << " (" << ar.iataV << ")  |  "
             << fixed << setprecision(0) << ar.peso << " km\n";
    }

    cout << "\nKilometraje total: " << fixed << setprecision(0)
         << mst.pesoTotal << " km\n";
}
