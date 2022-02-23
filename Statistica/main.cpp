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
 -chi quadro
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
    if (transposed) { //broken
        matrix = transpose(matrix);
    }
    return matrix;
}

double mean(vector<double> data) {
    long size = data.size();
    double sum = 0;
    for (int i = 0; i<size; ++i) {
        sum += data[i];
    }
    return sum/size;
}

double mode(vector<double> data) {
    long counter = 0, champion = 0;
    double saved = 0;
    for (double c: data) {
        counter = count(data.begin(), data.end(), c);
        if (counter >= champion) {
            saved = c;
            champion = counter;
        }
        remove(data.begin(), data.end(), c);
    }
    return saved;
}

double median(vector<double>) {
    return 0;
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
    long rows = matrix.size();
    //controllo se ogni riga ha stesso numero di colonne
    for (int i = 0; i < rows-1; ++i) {
        if (matrix[i].size() != matrix[i+1].size()) {
            cout << "Il vettore " << i+1 << "-esimo ha dimensione diversa" << endl;
        }
    }
    long columns = matrix[0].size();
    ofstream text(name);
    if (!transposed) {
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < columns; ++j) {
                text << matrix[i][j] << " ";
            }
            text << endl;
        }
    } else {
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < columns; ++j) {
                text << matrix[j][i] << " "; //trasposta della matrice di partenza
            }
            text << endl;
        }
    }
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
            case 2:
                for (int i = 0; i<size; ++i) { sum += pow(X[i]/SY[i], 2); }
            case 3:
                for (int i = 0; i<size; ++i) { sum += X[i]/pow(SY[i], 2); }
            case 4:
                for (int i = 0; i<size; ++i) { sum += Y[i]/pow(SY[i], 2); }
            case 5:
                for (int i = 0; i<size; ++i) { sum += X[i]*Y[i]/pow(SY[i], 2); }
            default:
                break;
        }
        return sum;
    }
    double delta() {
        return term(1)*term(2)-pow(term(3), 2);
    }
    double a() {
        return pow(delta(), -1)*(term(2)*term(4)-term(3)*term(5));
    }
    double incA() {
        return sqrt(pow(delta(), -1)*term(2));
    }
    double b() {
        return pow(delta(), -1)*(term(1)*term(5)-term(3)*term(4));
    }
    double incB() {
        return sqrt(pow(delta(), -1)*term(1));
    }
};



int main() {
    vector<double> list = loadDataManually();
    cout << mode(list) << endl;
    return 0;
}
