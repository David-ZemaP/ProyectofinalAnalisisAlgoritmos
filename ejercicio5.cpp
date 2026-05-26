#include "ejercicio5.h"

static bool compararPorCiudad(const pair<int, string>& a, const pair<int, string>& b) {
    return a.second < b.second;
}

static unordered_set<int> alcanzablesPorAerolinea(int idOrigen, int idAerolinea,
                                                   int maxEscalas, const Multigrafo& grafo) {
    unordered_set<int> alcanzables;
    unordered_map<int, int> visitados;
    queue<pair<int, int>> q;

    int maxVuelos = maxEscalas + 1;
    q.push({idOrigen, 0});
    visitados[idOrigen] = 0;

    while (!q.empty()) {
        int actual = q.front().first;
        int vuelos = q.front().second;
        q.pop();

        if (vuelos == maxVuelos) continue;

        if (grafo.count(actual) == 0) continue;

        const vector<AristaAirline>& aristas = grafo.at(actual);
        for (int i = 0; i < (int)aristas.size(); i++) {
            if (aristas[i].idAerolinea != idAerolinea) continue;

            int vecino = aristas[i].idDestino;
            int sigVuelos = vuelos + 1;

            if (visitados.count(vecino) == 0 || sigVuelos < visitados[vecino]) {
                visitados[vecino] = sigVuelos;
                alcanzables.insert(vecino);
                q.push({vecino, sigVuelos});
            }
        }
    }
    return alcanzables;
}

void Ejercicio5(const unordered_map<int, Aeropuerto>& aeropuertos,
                const unordered_map<int, Aerolinea>& aerolineas,
                const Multigrafo& grafo) {

    cin.ignore(10000, '\n');

    string consulta;
    cout << "Ingrese nombre, IATA o ID del aeropuerto de origen: ";
    cout.flush();
    if (!getline(cin, consulta)) { cin.clear(); return; }

    int idOrigen = buscarAeropuerto(aeropuertos, consulta);
    if (idOrigen == -1) { cout << "Aeropuerto de origen no encontrado.\n"; return; }

    const Aeropuerto& orig = aeropuertos.at(idOrigen);
    cout << "\nOrigen: " << orig.nombre << " (" << orig.iata << ") - "
         << orig.ciudad << ", " << orig.pais << "\n";

    cout << "Ingrese codigo IATA de la aerolinea (ej. AA, BA, LH): ";
    cout.flush();
    string codigo;
    if (!getline(cin, codigo)) { cin.clear(); return; }

    string codMin = aMinusculas(codigo);

    int idAerolinea = -1;
    string nombreAero;

    vector<int> idsAerolineas;
    for (pair<const int, Aerolinea> p : aerolineas) idsAerolineas.push_back(p.first);
    for (int i = 0; i < (int)idsAerolineas.size(); i++) {
        int id = idsAerolineas[i];
        const Aerolinea& aero = aerolineas.at(id);
        if (aMinusculas(aero.codigo) == codMin) {
            idAerolinea = aero.id;
            nombreAero = aero.codigo;
            break;
        }
    }

    if (idAerolinea == -1) {
        cout << "Aerolinea '" << codigo << "' no encontrada.\n";
        return;
    }
    cout << "Aerolinea: " << nombreAero << " (ID: " << idAerolinea << ")\n";

    int maxEscalas = 3;
    cout << "Maximo de escalas (default 3): ";
    string input;
    if (getline(cin, input) && !input.empty()) {
        try { maxEscalas = stoi(input); if (maxEscalas < 0) maxEscalas = 3; }
        catch (...) { maxEscalas = 3; }
    }

    unordered_set<int> alcanzables = alcanzablesPorAerolinea(idOrigen, idAerolinea, maxEscalas, grafo);

    cout << "\nAeropuertos alcanzables con maximo " << maxEscalas << " escalas"
         << " usando solo " << nombreAero << ": " << alcanzables.size() << "\n";

    if (!alcanzables.empty()) {
        vector<pair<int, string>> lista;
        for (int id : alcanzables) {
            if (aeropuertos.count(id) > 0) {
                const Aeropuerto& ap = aeropuertos.at(id);
                lista.push_back({id, ap.ciudad + ", " + ap.pais});
            }
        }

        sort(lista.begin(), lista.end(), compararPorCiudad);

        cout << "\nLista de destinos (orden alfabetico):\n";
        int contador = 0;
        for (int i = 0; i < (int)lista.size(); i++) {
            if (contador == 20) break;
            int id = lista[i].first;
            if (aeropuertos.count(id) > 0) {
                const Aeropuerto& ap = aeropuertos.at(id);
                cout << "  - " << ap.nombre << " (" << ap.iata
                     << ") - " << ap.ciudad << ", " << ap.pais << "\n";
                contador++;
            }
        }
        if (lista.size() > 20)
            cout << "  ... y " << (lista.size() - 20) << " mas.\n";
    } else {
        cout << "\nNo hay destinos alcanzables con esa aerolinea desde este origen.\n";
    }
}
