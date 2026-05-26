#include "ejercicio1.h"

static bool compararPorVuelos(const pair<int, int>& a, const pair<int, int>& b) {
    return a.second < b.second;
}

static void ejecutarBFS(const unordered_map<int, vector<Arista>>& grafo,
    int idOrigen, int maximoVuelos,
    unordered_set<int>& alcanzables,
    unordered_map<int, int>& vuelosMinimos)
{
    queue<pair<int, int>> pendientes;
    pendientes.push({idOrigen, 0});
    vuelosMinimos[idOrigen] = 0;

    while (!pendientes.empty()) {
        int act = pendientes.front().first;
        int v = pendientes.front().second;
        pendientes.pop();
        if (v == maximoVuelos) continue;
        if (grafo.count(act) == 0) continue;
        const vector<Arista>& aristas = grafo.at(act);
        for (int i = 0; i < (int)aristas.size(); i++) {
            int idDestino = aristas[i].idDestino;
            if (vuelosMinimos.count(idDestino) > 0) continue;
            vuelosMinimos[idDestino] = v + 1;
            alcanzables.insert(idDestino);
            pendientes.push({idDestino, v + 1});
        }
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

    unordered_set<int> alcanzables;
    unordered_map<int, int> vuelosMinimos;
    ejecutarBFS(grafo, idOrigen, 4, alcanzables, vuelosMinimos);

    cout << "Aeropuertos distintos alcanzables con maximo 3 escalas: "
         << alcanzables.size() << "\n";

    vector<pair<int, int>> ordenados;
    for (int id : alcanzables) {
        ordenados.push_back({id, vuelosMinimos[id]});
    }
    sort(ordenados.begin(), ordenados.end(), compararPorVuelos);

    cout << "\nPrimeros aeropuertos alcanzables (ordenados por cercania):\n";
    int most = 0;
    for (int i = 0; i < (int)ordenados.size(); i++) {
        if (most == 20) break;
        int id = ordenados[i].first;
        if (aeropuertos.count(id) == 0) continue;
        const Aeropuerto& ap = aeropuertos.at(id);
        cout << "- " << ap.nombre << " (" << ap.iata << ") | "
             << ap.ciudad << ", " << ap.pais << " | Vuelos: " << ordenados[i].second << "\n";
        most++;
    }
    if (alcanzables.size() > 20)
        cout << "... y " << (alcanzables.size() - 20) << " mas.\n";

    int c1 = 0, c2 = 0, c3 = 0;
    for (int i = 0; i < (int)ordenados.size(); i++) {
        int vuelos = ordenados[i].second;
        if (vuelos == 1) c1++;
        else if (vuelos == 2) c2++;
        else if (vuelos > 2) c3++;
    }
    cout << "  1 vuelo (0 escalas): " << c1 << "\n";
    cout << "  2 vuelos (1 escala): " << c2 << "\n";
    cout << "  3-4 vuelos (2-3 escalas): " << c3 << "\n";
}
