#include "ejercicio4.h"

void Ejercicio4(const unordered_map<int, Aeropuerto>& aeropuertos,
                const unordered_map<int, vector<Arista>>& grafo) {


    unordered_set<string> paisesSA = {
        "Argentina", "Bolivia", "Brazil", "Chile", "Colombia", "Ecuador",
        "Guyana", "Paraguay", "Peru", "Suriname", "Uruguay",
        "Venezuela", "French Guiana"
    };

    unordered_set<int> esSA;

    for (const auto& par : aeropuertos) {
        if (paisesSA.find(par.second.pais) != paisesSA.end()) {
            esSA.insert(par.first);
        }
    }

    cout << "Aeropuertos en Sudamerica: " << esSA.size() << "\n";

    unordered_map<int, vector<pair<int, double>>> grafoSA;

    for (const auto& par : grafo) {
        int idOrigen = par.first;

        if (esSA.find(idOrigen) == esSA.end()) {
            continue;
        }

        for (const Arista& ar : par.second) {
            int idDestino = ar.idDestino;

            if (esSA.find(idDestino) == esSA.end()) {
                continue;
            }

            grafoSA[idOrigen].push_back({idDestino, ar.distancia});
            grafoSA[idDestino].push_back({idOrigen, ar.distancia});
        }
    }

    cout << "Aeropuertos SA con rutas: " << grafoSA.size() << "\n";

    if (grafoSA.size() < 2) {
        cout << "No hay suficientes rutas entre SA.\n";
        return;
    }

    unordered_map<string, int> xPais;

    for (const auto& par : grafoSA) {
        auto it = aeropuertos.find(par.first);

        if (it != aeropuertos.end()) {
            xPais[it->second.pais]++;
        }
    }

    cout << "\nPor pais:\n";

    for (const auto& p : xPais) {
        cout << "  " << p.first << ": " << p.second << "\n";
    }

    unordered_set<int> visit;
    vector<vector<int>> componentes;

    for (const auto& par : grafoSA) {
        int inicio = par.first;

        if (visit.find(inicio) != visit.end()) {
            continue;
        }

        vector<int> comp;
        queue<int> q;

        q.push(inicio);
        visit.insert(inicio);

        while (!q.empty()) {
            int actual = q.front();
            q.pop();

            comp.push_back(actual);

            auto it = grafoSA.find(actual);

            if (it == grafoSA.end()) {
                continue;
            }

            for (const auto& [vecino, distancia] : it->second) {
                if (visit.find(vecino) == visit.end()) {
                    visit.insert(vecino);
                    q.push(vecino);
                }
            }
        }

        componentes.push_back(comp);
    }

    sort(componentes.begin(), componentes.end(),
         [](const auto& a, const auto& b) {
             return a.size() > b.size();
         });

    const vector<int>& compGigante = componentes[0];
    int n = compGigante.size();

    cout << "\nComponente mas grande con rutas entre SA: "
         << n << " aeropuertos\n";

    unordered_map<int, int> idAIdx;

    for (int i = 0; i < n; i++) {
        idAIdx[compGigante[i]] = i;
    }

    cout << "Construyendo MST (Prim con heap)...\n";

    auto inicioTiempo = high_resolution_clock::now();

    vector<bool> enMST(n, false);
    vector<double> minDist(n, numeric_limits<double>::max());
    vector<int> padre(n, -1);

    using Par = pair<double, int>;
    priority_queue<Par, vector<Par>, greater<Par>> pq;

    minDist[idAIdx[compGigante[0]]] = 0.0;
    pq.push({0.0, compGigante[0]});

    double pesoTotal = 0.0;

    while (!pq.empty()) {
        auto [distanciaActual, nodo] = pq.top();
        pq.pop();

        int u = idAIdx[nodo];

        if (enMST[u]) {
            continue;
        }

        enMST[u] = true;
        pesoTotal += distanciaActual;

        auto it = grafoSA.find(nodo);

        if (it == grafoSA.end()) {
            continue;
        }

        for (const auto& [vecino, distancia] : it->second) {
            int v = idAIdx[vecino];

            if (!enMST[v] && distancia < minDist[v]) {
                minDist[v] = distancia;
                padre[v] = u;
                pq.push({distancia, vecino});
            }
        }
    }

    auto duracion = duration_cast<milliseconds>(
        high_resolution_clock::now() - inicioTiempo
    ).count();

    vector<AristaMST> aristas;

    for (int v = 0; v < n; v++) {
        if (padre[v] == -1) {
            continue;
        }

        int u = padre[v];

        const auto& apU = aeropuertos.at(compGigante[u]);
        const auto& apV = aeropuertos.at(compGigante[v]);

        aristas.push_back({
            compGigante[u],
            compGigante[v],
            minDist[v],
            apU.nombre,
            apV.nombre,
            apU.iata,
            apV.iata
        });
    }

    cout << "\n=== RESULTADOS ===\n";
    cout << "Kilometraje total: "
         << fixed << setprecision(0) << pesoTotal << " km\n";
    cout << "Rutas en el arbol: " << aristas.size() << "\n";
    cout << "Aeropuertos conectados: " << n << "\n";
    cout << "Tiempo: " << duracion << " ms\n";
}