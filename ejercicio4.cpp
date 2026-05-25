#include "ejercicio4.h"

void Ejercicio4(const unordered_map<int, Aeropuerto>& aeropuertos,
                const unordered_map<int, vector<Arista>>& grafo) {
    cout << "\n========================================\n";
    cout << "   RETO 4: RED MINIMA DE SUDAMERICA (MST)\n";
    cout << "========================================\n";
    cout << "   (Usando rutas EXISTENTES entre SA)\n";
    cout << "========================================\n";

    unordered_set<string> paisesSA = {
        "Argentina","Bolivia","Brazil","Chile","Colombia","Ecuador",
        "Guyana","Paraguay","Peru","Suriname","Uruguay","Venezuela","French Guiana"
    };

    // Identificar aeropuertos sudamericanos
    unordered_set<int> esSA;
    for (const auto& par : aeropuertos)
        if (paisesSA.find(par.second.pais) != paisesSA.end())
            esSA.insert(par.first);

    cout << "Aeropuertos en Sudamerica: " << esSA.size() << "\n";

    // Construir grafo NO dirigido con rutas EXISTENTES entre SA
    unordered_map<int, vector<pair<int, double>>> grafoSA;
    for (const auto& par : grafo) {
        if (esSA.find(par.first) == esSA.end()) continue;
        for (const Arista& ar : par.second) {
            if (esSA.find(ar.idDestino) == esSA.end()) continue;
            // Tratar como no dirigido para MST
            grafoSA[par.first].push_back({ar.idDestino, ar.distancia});
            grafoSA[ar.idDestino].push_back({par.first, ar.distancia});
        }
    }

    cout << "Aeropuertos SA con rutas: " << grafoSA.size() << "\n";
    if (grafoSA.size() < 2) { cout << "No hay suficientes rutas entre SA.\n"; return; }

    // Conteo por pais (solo los que estan en el grafo)
    unordered_map<string,int> xPais;
    for (const auto& par : grafoSA) {
        auto it = aeropuertos.find(par.first);
        if (it != aeropuertos.end()) xPais[it->second.pais]++;
    }
    cout << "\nPor pais:\n";
    for (const auto& p : xPais) cout << "  " << p.first << ": " << p.second << "\n";

    // Encontrar la componente conexa mas grande del grafo SA
    unordered_set<int> visit;
    vector<vector<int>> componentes;
    for (const auto& par : grafoSA) {
        if (visit.find(par.first) != visit.end()) continue;
        vector<int> comp;
        queue<int> q;
        q.push(par.first);
        visit.insert(par.first);
        while (!q.empty()) {
            int a = q.front(); q.pop();
            comp.push_back(a);
            auto it = grafoSA.find(a);
            if (it == grafoSA.end()) continue;
            for (const auto& [v, d] : it->second)
                if (visit.find(v) == visit.end()) { visit.insert(v); q.push(v); }
        }
        componentes.push_back(comp);
    }

    sort(componentes.begin(), componentes.end(),
         [](const auto& a, const auto& b) { return a.size() > b.size(); });

    const vector<int>& compGigante = componentes[0];
    int n = compGigante.size();
    cout << "\nComponente mas grande con rutas entre SA: " << n << " aeropuertos\n";

    // Mapear ID -> indice para Prim
    unordered_map<int, int> idAIdx;
    for (int i = 0; i < n; i++) idAIdx[compGigante[i]] = i;

    // Prim con priority queue sobre el grafo SA existente
    cout << "Construyendo MST (Prim con heap)...\n";
    auto inicio = high_resolution_clock::now();

    vector<bool> enMST(n, false);
    vector<double> minDist(n, numeric_limits<double>::max());
    vector<int> padre(n, -1);
    using Par = pair<double, int>;
    priority_queue<Par, vector<Par>, greater<Par>> pq;

    minDist[idAIdx[compGigante[0]]] = 0;
    pq.push({0, compGigante[0]});
    double pesoTotal = 0;

    while (!pq.empty()) {
        auto [d, nodo] = pq.top(); pq.pop();
        int u = idAIdx[nodo];
        if (enMST[u]) continue;
        enMST[u] = true;
        pesoTotal += d;

        auto it = grafoSA.find(nodo);
        if (it == grafoSA.end()) continue;
        for (const auto& [vecino, dist] : it->second) {
            int v = idAIdx[vecino];
            if (!enMST[v] && dist < minDist[v]) {
                minDist[v] = dist;
                padre[v] = u;
                pq.push({dist, vecino});
            }
        }
    }

    auto duracion = duration_cast<milliseconds>(high_resolution_clock::now() - inicio).count();

    // Recolectar aristas del MST
    vector<AristaMST> aristas;
    for (int v = 0; v < n; v++) {
        if (padre[v] == -1) continue;
        int u = padre[v];
        const auto& apU = aeropuertos.at(compGigante[u]);
        const auto& apV = aeropuertos.at(compGigante[v]);
        double distReal = minDist[v];
        aristas.push_back({compGigante[u], compGigante[v], distReal,
                           apU.nombre, apV.nombre, apU.iata, apV.iata});
    }

    cout << "\n=== RESULTADOS ===\n";
    cout << "Kilometraje total: " << fixed << setprecision(0) << pesoTotal << " km\n";
    cout << "Rutas en el arbol: " << aristas.size() << "\n";
    cout << "Aeropuertos conectados: " << n << "\n";
    cout << "Tiempo: " << duracion << " ms\n";

    // Top 15 mas largas
    sort(aristas.begin(), aristas.end(),
         [](const AristaMST& a, const AristaMST& b) { return a.peso > b.peso; });
    cout << "\nRutas mas largas (top 15):\n";
    for (size_t i = 0; i < min(aristas.size(), size_t(15)); i++)
        cout << "  " << (i+1) << ". " << aristas[i].nombreU << " (" << aristas[i].iataU
             << ") <--> " << aristas[i].nombreV << " (" << aristas[i].iataV
             << ")  " << fixed << setprecision(0) << aristas[i].peso << " km\n";

    // Top 10 mas cortas
    sort(aristas.begin(), aristas.end(),
         [](const AristaMST& a, const AristaMST& b) { return a.peso < b.peso; });
    cout << "\nRutas mas cortas (top 10):\n";
    for (size_t i = 0; i < min(aristas.size(), size_t(10)); i++)
        cout << "  " << (i+1) << ". " << aristas[i].nombreU << " (" << aristas[i].iataU
             << ") <--> " << aristas[i].nombreV << " (" << aristas[i].iataV
             << ")  " << fixed << setprecision(0) << aristas[i].peso << " km\n";

    if (aristas.size() >= 2) {
        sort(aristas.begin(), aristas.end(),
             [](const AristaMST& a, const AristaMST& b) { return a.peso < b.peso; });
        double prom = 0;
        for (const auto& a : aristas) prom += a.peso;
        prom /= aristas.size();
        cout << "\n--- ESTADISTICAS ---\n";
        cout << "Ruta mas larga: " << fixed << setprecision(0) << aristas.back().peso << " km\n";
        cout << "Ruta mas corta: " << fixed << setprecision(0) << aristas.front().peso << " km\n";
        cout << "Distancia promedio: " << fixed << setprecision(0) << prom << " km\n";
        cout << "Total: " << fixed << setprecision(0) << pesoTotal << " km\n";
    }

    // Aeropuertos SA que quedaron fuera de la red
    int fuera = 0;
    for (int id : esSA)
        if (grafoSA.find(id) == grafoSA.end()) fuera++;
    if (fuera > 0)
        cout << "\nAeropuertos SA sin rutas a otro SA (aislados): " << fuera << "\n";
}
