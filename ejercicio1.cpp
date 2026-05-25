#include "ejercicio1.h"

static ResultadoBFS ejecutarBFS(const unordered_map<int, vector<Arista>>& grafo,
                                 int idOrigen, int maximoVuelos) {
    ResultadoBFS res;
    queue<pair<int, int>> pendientes;
    pendientes.push({idOrigen, 0});
    res.vuelosMinimos[idOrigen] = 0;
    res.mapaPadres[idOrigen] = -1;

    while (!pendientes.empty()) {
        int act = pendientes.front().first;
        int v = pendientes.front().second;
        pendientes.pop();
        if (v == maximoVuelos) continue;
        if (grafo.find(act) == grafo.end()) continue;
        for (const Arista& ar : grafo.at(act)) {
            if (res.vuelosMinimos.find(ar.idDestino) != res.vuelosMinimos.end()) continue;
            res.vuelosMinimos[ar.idDestino] = v + 1;
            res.mapaPadres[ar.idDestino] = act;
            res.aeropuertosAlcanzables.insert(ar.idDestino);
            pendientes.push({ar.idDestino, v + 1});
        }
    }
    return res;
}

static void mostrarRutasExactas(const vector<pair<int, int>>& ordenados,
                                 const unordered_map<int, Aeropuerto>& aeropuertos,
                                 const ResultadoBFS& bfs) {
    cout << "\nDesea ver las rutas exactas de los primeros 5 aeropuertos? (s/n): ";
    char r; cin >> r;
    if (r != 's' && r != 'S') return;
    cout << "\n=== RUTAS DE VUELO EXACTAS ===\n";
    int most = 0;
    for (const auto& par : ordenados) {
        if (most == 5) break;
        int id = par.first;
        if (aeropuertos.find(id) == aeropuertos.end()) continue;
        vector<int> camino;
        int act = id;
        while (act != -1) { camino.push_back(act); act = bfs.mapaPadres.at(act); }
        reverse(camino.begin(), camino.end());
        cout << (most + 1) << ". Hacia " << aeropuertos.at(id).ciudad
             << " (" << aeropuertos.at(id).iata << "):\n   Ruta: ";
        for (size_t i = 0; i < camino.size(); ++i) {
            cout << aeropuertos.at(camino[i]).iata;
            if (i < camino.size() - 1) cout << " -> ";
        }
        cout << "\n\n";
        most++;
    }
}

void Ejercicio1(const unordered_map<int, Aeropuerto>& aeropuertos,
                const unordered_map<int, vector<Arista>>& grafo) {
    cin.ignore(10000, '\n');
    string consulta;
    cout << "Ingrese ID, nombre o codigo IATA del aeropuerto: ";
    cout.flush();
    if (!getline(cin, consulta)) { cin.clear(); return; }

    int idOrigen = buscarAeropuerto(aeropuertos, consulta);
    if (idOrigen == -1) { cout << "No se encontro el aeropuerto.\n"; return; }

    const Aeropuerto& orig = aeropuertos.at(idOrigen);
    cout << "\nOrigen: " << orig.nombre << " (" << orig.iata << ") - "
         << orig.ciudad << ", " << orig.pais << "\n";

    ResultadoBFS res = ejecutarBFS(grafo, idOrigen, 4); // 3 escalas = 4 vuelos max

    cout << "Aeropuertos distintos alcanzables con maximo 3 escalas: "
         << res.aeropuertosAlcanzables.size() << "\n";

    vector<pair<int, int>> ordenados;
    for (int id : res.aeropuertosAlcanzables)
        ordenados.push_back({id, res.vuelosMinimos.at(id)});
    sort(ordenados.begin(), ordenados.end(),
         [](const auto& a, const auto& b) { return a.second < b.second; });

    cout << "\nPrimeros aeropuertos alcanzables (ordenados por cercania):\n";
    int most = 0;
    for (const auto& par : ordenados) {
        if (most == 20) break;
        if (aeropuertos.find(par.first) == aeropuertos.end()) continue;
        const auto& ap = aeropuertos.at(par.first);
        cout << "- " << ap.nombre << " (" << ap.iata << ") | "
             << ap.ciudad << ", " << ap.pais << " | Vuelos: " << par.second << "\n";
        most++;
    }
    if (res.aeropuertosAlcanzables.size() > 20)
        cout << "... y " << (res.aeropuertosAlcanzables.size() - 20) << " mas.\n";

    int c1 = 0, c2 = 0, c3 = 0;
    for (const auto& par : ordenados) {
        if (par.second == 1) c1++;
        else if (par.second == 2) c2++;
        else if (par.second > 2) c3++;
    }
    cout << "  1 vuelo (0 escalas): " << c1 << "\n";
    cout << "  2 vuelos (1 escala): " << c2 << "\n";
    cout << "  3-4 vuelos (2-3 escalas): " << c3 << "\n";

    mostrarRutasExactas(ordenados, aeropuertos, res);
}
