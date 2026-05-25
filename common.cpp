#include "common.h"

vector<string> dividirCSV(const string& linea) {
    vector<string> valores;
    string valor;
    bool dentroDeComillas = false;
    for (char c : linea) {
        if (c == '"') {
            dentroDeComillas = !dentroDeComillas;
        } else if (c == ',' && !dentroDeComillas) {
            valores.push_back(valor);
            valor.clear();
        } else {
            valor += c;
        }
    }
    valores.push_back(valor);
    return valores;
}

string aMinusculas(string texto) {
    for (char& c : texto) {
        c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    }
    return texto;
}

double calcularHaversine(double lat1, double lon1, double lat2, double lon2) {
    const double radioTierra = 6371.0;
    const double pi = 3.141592653589793;
    double phi1 = lat1 * pi / 180.0;
    double phi2 = lat2 * pi / 180.0;
    double deltaPhi = (lat2 - lat1) * pi / 180.0;
    double deltaLam = (lon2 - lon1) * pi / 180.0;
    double a = sin(deltaPhi / 2) * sin(deltaPhi / 2) +
               cos(phi1) * cos(phi2) *
               sin(deltaLam / 2) * sin(deltaLam / 2);
    return radioTierra * 2 * atan2(sqrt(a), sqrt(1 - a));
}

unordered_map<int, Aeropuerto> cargarAeropuertos(const string& rutaArchivo) {
    unordered_map<int, Aeropuerto> aeropuertos;
    ifstream archivo(rutaArchivo);
    if (!archivo.is_open())
        throw runtime_error("No se pudo abrir el archivo de aeropuertos: " + rutaArchivo);

    string linea;
    getline(archivo, linea);
    int omitidos = 0;
    while (getline(archivo, linea)) {
        vector<string> fila = dividirCSV(linea);
        if (fila.size() < 8) { omitidos++; continue; }
        try {
            Aeropuerto a;
            a.id = stoi(fila[0]);
            a.nombre = fila[1];
            a.ciudad = fila[2];
            a.pais = fila[3];
            a.iata = fila[4];
            a.icao = fila[5];
            a.latitud = stod(fila[6]);
            a.longitud = stod(fila[7]);
            aeropuertos[a.id] = a;
        } catch (...) { omitidos++; }
    }
    if (omitidos > 0) cout << "  Aeropuertos omitidos: " << omitidos << "\n";
    return aeropuertos;
}

unordered_map<int, vector<Arista>> cargarRutas(
    const string& rutaArchivo,
    const unordered_map<int, Aeropuerto>& aeropuertos
) {
    unordered_map<int, vector<Arista>> grafo;
    ifstream archivo(rutaArchivo);
    if (!archivo.is_open())
        throw runtime_error("No se pudo abrir el archivo de rutas: " + rutaArchivo);

    string linea;
    getline(archivo, linea);
    int omitidas = 0, invalidas = 0;
    while (getline(archivo, linea)) {
        vector<string> fila = dividirCSV(linea);
        if (fila.size() < 2) { omitidas++; continue; }
        try {
            int idOrigen = stoi(fila[0]);
            int idDestino = stoi(fila[1]);
            auto itO = aeropuertos.find(idOrigen);
            auto itD = aeropuertos.find(idDestino);
            if (itO != aeropuertos.end() && itD != aeropuertos.end()) {
                double dist = calcularHaversine(itO->second.latitud, itO->second.longitud,
                                                itD->second.latitud, itD->second.longitud);
                grafo[idOrigen].push_back({idDestino, dist});
            } else { invalidas++; }
        } catch (...) { omitidas++; }
    }
    if (omitidas > 0 || invalidas > 0)
        cout << "  Rutas omitidas: " << omitidas << " | Invalidas: " << invalidas << "\n";
    return grafo;
}

int buscarAeropuerto(const unordered_map<int, Aeropuerto>& aeropuertos, const string& consulta) {
    string consMin = aMinusculas(consulta);

    int id = -1;
    try { id = stoi(consulta); } catch (...) {}
    if (id != -1 && aeropuertos.find(id) != aeropuertos.end()) return id;


    for (const auto& par : aeropuertos)
        if (aMinusculas(par.second.iata) == consMin) return par.second.id;

    int mejorId = -1;
    size_t menorPos = string::npos;
    for (const auto& par : aeropuertos) {
        size_t pos = aMinusculas(par.second.nombre).find(consMin);
        if (pos != string::npos && (menorPos == string::npos || pos < menorPos)) {
            menorPos = pos;
            mejorId = par.second.id;
        }
    }
    if (mejorId != -1) return mejorId;

    for (const auto& par : aeropuertos)
        if (aMinusculas(par.second.ciudad).find(consMin) != string::npos)
            return par.second.id;

    return -1;
}

vector<vector<int>> encontrarSCCs(const unordered_map<int, vector<Arista>>& grafo) {
    // Convertir a lista de adyacencia simple (sin distancias)
    unordered_map<int, vector<int>> g;
    for (const auto& par : grafo)
        for (const Arista& ar : par.second)
            g[par.first].push_back(ar.idDestino);

    // Tarjan's SCC algorithm
    unordered_map<int, int> idx, low;
    unordered_map<int, bool> enPila;
    stack<int> pila;
    int contador = 0;
    vector<vector<int>> componentes;

    function<void(int)> tarjan = [&](int u) {
        idx[u] = low[u] = ++contador;
        pila.push(u);
        enPila[u] = true;

        auto it = g.find(u);
        if (it != g.end()) {
            for (int v : it->second) {
                if (idx.find(v) == idx.end()) {
                    tarjan(v);
                    low[u] = min(low[u], low[v]);
                } else if (enPila[v]) {
                    low[u] = min(low[u], idx[v]);
                }
            }
        }

        if (low[u] == idx[u]) {
            vector<int> comp;
            int v;
            do {
                v = pila.top(); pila.pop();
                enPila[v] = false;
                comp.push_back(v);
            } while (v != u);
            componentes.push_back(comp);
        }
    };

    for (const auto& par : g)
        if (idx.find(par.first) == idx.end())
            tarjan(par.first);

    sort(componentes.begin(), componentes.end(),
         [](const auto& a, const auto& b) { return a.size() > b.size(); });

    return componentes;
}
