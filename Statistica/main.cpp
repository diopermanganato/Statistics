//
//  main.cpp
//  Statistica
//
//  Created by Inartespacco on 09/01/2022.
//

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
/*
 Idee:
 -propagazione incertezze
 -chi quadro //esempio in Estensimetro
 .
 */

using namespace std;

bool isNumber(string str) {
    for (char c: str) {
        if ((isdigit(c) != 0) || c == '.') {
            continue;
        }
        return false;
    }
    return true;
}

int countLines(string fileName) {
    ifstream file(fileName);
    int num = 0;
    for (string line = ""; getline(file, line);) {
        num += 1;
    }
    return num;
}

double convert(string str) {
    if (isNumber(str)) {
        return stod(str);
    } else {
        replace(str.begin(), str.end(), ',', '.');
        if (isNumber(str)) {
            return stod(str);
        } else {
            cout << str << " non è formattabile" << endl;
            abort();
        }
    }
}

vector<double> stringToVector(string line) {
    vector<double> ret;
    string temp;
    string _line = line + ' ';
    for (char c: _line) {
        if ((isdigit(c) != 0) || (c == '.') || (c == ',')) {
            temp.push_back(c);
        } else if ((c == ' ') && (temp == "")) {
            continue;
        } else if (c == ' ') {
            ret.push_back(convert(temp));
            temp = "";
        }
    }
    return ret;
}

string vectorToString(vector<double> input) {
    string output;
    for (double c: input) {
        string elem = to_string(c) + " ";
        output += elem;
    }
    return output;
}

vector<double> removeAllDuplicates(vector<double> vec) {
    long size = vec.size();
    vector<double> ret;
    sort(vec.begin(), vec.end());
    for (int i = 0; i < size; ++i) {
        if ((i == size) && (vec[i] != vec[i-1])) {
            ret.push_back(vec[i]);
        } else if (vec[i] != vec[i+1]) {
            ret.push_back(vec[i]);
        }
    }
    return ret;
}

vector<vector<double>> transpose(vector<vector<double>> matrix) {
    vector<vector<double>> temp(matrix[0].size()); //all columns must have the same size
    for (int i = 0; i < matrix.size(); ++i) {
        for (int j = 0; j < matrix[0].size(); ++j) {
            temp[j].push_back(matrix[i][j]); //in new j-th row of the new matrix we put all the elements of old matrix's i-th row
        }
    }
    return temp;
}

vector<double> loadDataManually() {
    vector<double> output;
    string x;
    cout << "Inserisci i valori e premi x per chiudere l'input" << endl;
    while (cin >> x) {
        if (x == "x") break;
        output.push_back(convert(x));
    }
    return output;
}

vector<vector<double>> readMatrix(string fileName, bool transposed) { 
    ifstream file(fileName);
    vector<vector<double>> matrix;
    if (file.fail()) {
        cout << "Errore nell'apertura di " << fileName << endl;
        abort();
    }
    for (string line; getline(file, line);) {
        matrix.push_back(stringToVector(line));
    }
    if (transposed) { 
        matrix = transpose(matrix);
    }
    return matrix;
}

vector<string> wrapStringArray(vector<string> input, string end) {
    vector<string> output;
    for (string s: input) {
        string cor = end + s + end;
        output.push_back(cor);
    }
    return output;
}

double mean(vector<double> data) {
    long size = data.size();
    double sum = 0;
    for (int i = 0; i<size; ++i) {
        sum += data[i];
    }
    return sum/size;
}

double mode(vector<double> data) { //could be improved
    sort(data.begin(), data.end());
    long counter = 0, champion = 0;
    double saved = 0;
    for (double c: data) {
        counter = count(data.begin(), data.end(), c);
        if (counter >= champion) {
            saved = c;
            champion = counter;
        }
    }
    return saved;
}

double median(vector<double> data) {
    long size = data.size();
    sort(data.begin(), data.end());
    return data[size/2];
}


double stanDev(vector<double> data) {
    double m = mean(data);
    long size = data.size();
    double sum = 0;
    for (int i = 0; i<size; ++i) {
        sum += pow(m-data[i], 2);
    }
    double sd = sqrt(sum/(size-1));
    return sd;
}

double meanStanDev(vector<double> data) {
    double sd = stanDev(data);
    long size = data.size();
    return sd/sqrt(size);
}

double compatibility(double msr1, double inc1, double msr2, double inc2) {
    return abs(msr1-msr2)/sqrt(pow(inc1, 2)+pow(inc2, 2));
}

double chiSquared(int k, vector<double> msr, vector<double> refValues, vector<double> var) {
    double sum = 0;
    for (int i = 0; i < k; ++i) {
        sum += pow((msr[i]-refValues[i])/var[i], 2);
    }
    return sum;
}

void writeDataOnFile(string name, vector<vector<double>> matrix, bool transposed) {
    for (int i = 0; i < matrix.size()-1; ++i) {
        if (matrix[i].size() != matrix[i+1].size()) {
            cout << "Il vettore " << i+1 << "-esimo ha dimensione diversa" << endl;
        }
    }
    ofstream text(name);
    if (!transposed) {
        long rows = matrix.size();
        long columns = matrix[0].size();
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < columns; ++j) {
                text << matrix[i][j] << " ";
            }
            text << endl;
        }
    } else {
        vector<vector<double>> _matrix = transpose(matrix);
        long rows = _matrix.size();
        long columns = _matrix[0].size();
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < columns; ++j) {
                text << _matrix[i][j] << " "; //trasposta della matrice di partenza
            }
            text << endl;
        }
    }
}

vector<double> triangError(vector<double> resolutions) {
    vector<double> output;
    for (int i = 0; i < resolutions.size(); ++i) {
        output.push_back(resolutions[i]/(2*sqrt(6)));
    }
    return output;
}

//coefficienti interpolazione
struct Interpolation {
    vector<double> X, Y, SY;
    double term(int discr) {
        long sizeX = X.size(), sizeY = Y.size(), sizeSY = SY.size();
        if ((sizeX != sizeY) || (sizeY != sizeSY) || (sizeX != sizeSY)) {
            cout << "Errore, i dati inseriti hanno dimensioni diverse" << endl;
            abort();
        }
        double sum = 0;
        long size = X.size();
        switch (discr) {
            case 1:
                for (int i = 0; i<size; ++i) { sum += pow(SY[i], -2); }
                break;
            case 2:
                for (int i = 0; i<size; ++i) { sum += pow(X[i]/SY[i], 2); }
                break;
            case 3:
                for (int i = 0; i<size; ++i) { sum += X[i]/pow(SY[i], 2); }
                break;
            case 4:
                for (int i = 0; i<size; ++i) { sum += Y[i]/pow(SY[i], 2); }
                break;
            case 5:
                for (int i = 0; i<size; ++i) { sum += X[i]*Y[i]/pow(SY[i], 2); }
                break;
            default:
                break;
        }
        return sum;
    }
    double delta() {
        return term(1)*term(2)-pow(term(3), 2);
    }
    double a() { //intercept
        return pow(delta(), -1)*(term(2)*term(4)-term(3)*term(5));
    }
    double incA() {
        return sqrt(pow(delta(), -1)*term(2));
    }
    double b() { //slope
        return pow(delta(), -1)*(term(1)*term(5)-term(3)*term(4));
    }
    double incB() {
        return sqrt(pow(delta(), -1)*term(1));
    }
};

//i parametri consistono ordinatamente di titolo, x label, y label. Il grid è settato di default
void multipleLinearFit(string fileName, vector<string> _dataFiles, int columns, vector<string> _parameters) {
    ofstream file(fileName);
    vector<string> dataFiles = wrapStringArray(_dataFiles, "'");
    vector<string> parameters = wrapStringArray(_parameters, "'");
    long numOfFits = dataFiles.size();
    
    vector<string> functions = {"f(x)","g(x)","h(x)","s(x)","t(x)","r(x)","p(x)"};
    vector<string> funcNames = {"f", "g", "h", "s", "t", "r", "p"};
    vector<vector<string>> coupleOfParam = {{"a", "b"},{"c", "d"},{"e", "f"},{"g", "h"},{"i", "j"},{"k", "l"},{"m", "n"}};
    
    if (numOfFits > functions.size()) {
        cout << "Non ci sono abbastanza funzioni" << endl;
        abort();
    }
    
    if (parameters.size() != 3) {
        cout << "Inserire titolo, label di x e di y" << endl;
        abort();
    }
    
    file << "set grid" << endl;
    file << "set title " << parameters[0] << endl;
    file << "set xlabel " << parameters[1] << endl;
    file << "set ylabel " << parameters[2] << endl;
    for (int i = 0; i < numOfFits; ++i) {
        file << functions[i] << " = " << coupleOfParam[i][0] << " + " << coupleOfParam[i][1] << "*x" << endl;
        file << coupleOfParam[i][0] << " = 1" << "\n" << coupleOfParam[i][1] << " = 1" << endl;
    }
    switch (columns) {
        case 3:
            for (int i = 0; i < numOfFits; ++i) {
                file << "fit " << functions[i] << " " << dataFiles[i] << " using 1:2:3 via " << coupleOfParam[i][0] << "," << coupleOfParam[i][1] << endl;
            }
            break;
        default:
            cout << "Numero di colonne non ammesso" << endl;
            break;
    }
    file << "plot ";
    for (int i = 0; i < numOfFits-1; ++i) {
        file << functions[i] << ", " << dataFiles[i] << ", ";
    }
    file << functions[numOfFits-1] << ", " << dataFiles[numOfFits-1];
}

void gnuplotPrint(string fileName, string _dataFile, int columns, bool linearFit) {
    ofstream file(fileName);
    string dataFile = "'" + _dataFile + "'";
    if (linearFit) { //per la scelta dei parametri a,b trovare un valore conveniente
        file << "f(x) = a + b*x" << "\n" << "a = 1" << "\n" << "b = 1" << endl;
        switch (columns) {
            case 2:
                file << "fit f(x) " << dataFile << " using 1:2 via a,b" << endl;
                file << "plot " << dataFile << " using 1:(f($1)) with lines,'' using 1:2" << endl;
                break;
            case 3:
                file << "fit f(x) " << dataFile << " using 1:2:3 via a,b" << endl;
                file << "plot " << dataFile << " using 1:(f($1)) with lines,'' using 1:2:3 with yerrorbars" << endl;
                break;
            default: break;
        }
    } else {
        switch (columns) {
            case 2:
                file << "plot " << dataFile << endl;
                break;
            case 3:
                file << "plot " <<  dataFile << " using 1:2:3 with yerrorbars" << endl;
                break;
            default:
                break;
        }
    }
}

int main() {
    multipleLinearFit("prova", {"file1.txt", "file2.txt"}, 3, {"titolo", "x [s]", "y [m]"});
    return 0;
}
