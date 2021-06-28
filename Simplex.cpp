#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <vector>
#include <algorithm>
#include <string>
#include <iomanip>

using namespace std;

struct Tableau {

    vector <double> z_c; // Armazena valor da função objetivo + coeficientes das variáveis não básicas
    vector < vector <double> > Y; // Armazena os coeficientes das restrições
    vector <double> x_B; // Armazena os valores das variáveis básicas
    double obj; // Valor da função objetivo;
    vector <int> b_indx; // Armazena os índices das variáveis básicas
    vector <int> nb_indx; // Armazena os índice das variáveis não básica
    vector <double> rB; //Armazena os valores do lado direito das inequações para análise de sensibilidade
    vector <double> fo; //Armazena os coeficientes da função objetivo para análise de sensibilidade

};

struct Dados {

    int nVariables; // Número de variáveis
    int nConstraints; // Número de restrições

};

void leitor(int, char **, Dados &, Tableau &); // Lê os dados do problema
void extractIntegerWords(string, const int &, Tableau &, Dados &); // Extrai os coeficientes
void criaPrimeiroTableau(const Dados & d, Tableau &); // Cria o primeiro tableau
void printaTableau(const Tableau &, const Dados &); // Escreve tableau
int checaParada (const Tableau &); // Checa se o algoritmo pode parar e retorna o valor da coluna pivô
int linhaPivo (const Dados &, const Tableau &, const int &); // Retorna a linha pivô
void divideLinhaPivo (Tableau &, const int &, const int &); // Divide a linha pivô pelo número pivô
void modificaBase (const Dados &, Tableau &, const int &, const int &); // Cria próximo tableau
void mostraUnbounded(const Dados &, const Tableau &, const int &); // Caso o problema seja unbounded, mostra como podemos aumentar o valor de z em função de t
void analiseSensibilidade (Tableau &, Dados & d); //Faz análise de sensibilidade
void printaSolucaoDual (Tableau &, Dados &); //Escreve a solução do dual

int main (int argc, char ** argv) {

    Dados d; // Armazena os dados
    Tableau T; // Tableau

    leitor(argc, argv, d, T); // Lê os dados do problema
    
    criaPrimeiroTableau(d, T); // Cria primeiro tableau

    int cont = 1; // Conta o número de iterações do algoritmo
    int sol = 1; // Variável para classificar a solução. sol = 1 -> ótima; sol = 0 -> unbounded; sol = -1 -> inviável

    cout << "Tableau " << cont << endl << endl;
    printaTableau(T, d);

    int colunaPivo = checaParada(T); // Retorna coluna pivô ou para o algoritmo
    int lPivo = linhaPivo(d, T, colunaPivo);

    if (colunaPivo == -1) {
        cout << "Solução ótima encontrada.";
    } else {
        while (colunaPivo != -1) {
            int lPivo = linhaPivo(d, T, colunaPivo); // Calcula linha pivô
            
            // lPivo = -1 indica que o problema é unbounded
            if (lPivo == -1) {
                cout << "Unbounded!!" << endl << endl;
                mostraUnbounded(d, T, colunaPivo); // Mostra resultado unbounded
                sol = 0; // Indica que é unbounded
                break; // Para execução do simplex
            }
            
            int numeroPivo = T.Y[lPivo][colunaPivo]; // Pega o numero pivô
            divideLinhaPivo(T, lPivo, colunaPivo); // Divide linha pivô pelo número pivô
            modificaBase(d, T, lPivo, colunaPivo); // Gera próximo tableau
            cont++;

            cout << "Tableau " << cont << endl << endl;
            printaTableau(T, d);

            colunaPivo = checaParada(T); // Checa critério de parada e inicia a próxima iteração do algoritmo
        }

        for (int i = 0; i < T.b_indx.size(); i++) {
            if (T.x_B[i] < 0 || (T.b_indx[i] > d.nVariables + d.nConstraints && T.x_B[i] != 0.00)) {
                sol = -1;
                break;
            }
        }

        // Sol = 1 indica solução ótima
        if (sol == 1) {
            cout << "Solução ótima:\n\n";

            // Mostra os valores das variáveis
            for (int i = 0; i < T.b_indx.size(); i++) {
                if (T.b_indx[i] <= d.nVariables && T.b_indx[i] != 0) {
                    cout << "x" << T.b_indx[i] << " = " << T.x_B[i] << endl;
                }  
            }

            for (int i = 0; i < T.nb_indx.size(); i++) {
                if (T.nb_indx[i] <= d.nVariables && T.b_indx[i] != 0) {
                    cout << "x" << T.nb_indx[i] << " = " << 0 << endl;
                }  
            }

            // Mostra o valor da função objetivo
            cout << endl << "z = " << T.obj << endl << endl;

            printaSolucaoDual(T, d);
            analiseSensibilidade(T, d);
        } else if (sol == -1) {
            cout << "Não há solução viável para o problema." << endl;
        }
    }
    
    return 0; // Fim da execução

}

void leitor(int argc, char ** argv, Dados & d, Tableau & T) {

    char * ins;
    ins = argv[1];

    ifstream Instancia (ins, ios::in);

    if (!Instancia) {
        cerr << "O arquivo nao pode ser aberto." << endl << endl;
        exit(1);
    }

    string auxiliar;

    getline(Instancia, auxiliar);
    d.nVariables = atoi(auxiliar.c_str()); //Número de variáveis

    getline(Instancia, auxiliar);
    d.nConstraints = atoi(auxiliar.c_str()); //Número de restrições

    int cont = 1;
    string aux;
    int q;
    while (!Instancia.eof()) {
       getline(Instancia, aux);
       extractIntegerWords(aux, cont, T, d); //Retira os coeficientes do arquivo .txt para formar o tableau
       cont++;
    }

}

void extractIntegerWords(string str, const int & n, Tableau & T, Dados & d) {
    stringstream ss;    
  
    /* Storing the whole string into string stream */
    ss << str;
  
    /* Running loop till the end of the stream */
    string temp;
    string aux;
    double found;
    int auxx = 0;
    int auxx2 = 0;
    int cont = 0;
    vector <double> auxTableau;
    vector <double> auxTableau2;
   
    while (!ss.eof()) {
  
        /* extracting word by word from stream */
        ss >> temp;
  
        /* Checking the given word is integer or not */
        if (stringstream(temp) >> found) {
            if (n == 1) {
                if (aux != "-") {
                    T.z_c.push_back(found);
                    T.fo.push_back(found);
                } else {
                    T.z_c.push_back(-found);
                    T.fo.push_back(found);
                }
            } else {
                if (aux != "-") {
                    auxTableau.push_back(found);
                } else {
                    auxTableau.push_back(-found);
                }

                if ((aux == ">=" || aux == "=") && cont == 0) {
                    auxx = 1;
                    cont = 1;
                }
            
            }
        } 
        /* To save from space at the end of string */
        aux = temp;
        temp = "";
    }

    if (auxx == 1) {
        for (int i = 0; i < auxTableau.size(); i++) {
            auxTableau[i] = -1 * auxTableau[i];
        }
    } 
    T.Y.push_back(auxTableau);

}

void printaTableau(const Tableau & T, const Dados & d) {

    int tam = T.Y[1].size(); //Quantidade de variáveis (Variáveis de decisão, variáveis de folga, variáveis auxiliares)
    for (int i = 1; i < tam; i++) {
        cout << setw(12) << "X" << i;
    }
    cout << setw(12) << "B" << endl;

    //Escreve o tableau
    for (int i = 1; i < d.nConstraints + 1; i++) {
        cout << "X" << T.b_indx[i-1];
        for (int j = 0; j < T.Y[i].size(); j++) {
            cout << fixed << setprecision(2) << setw(12) << T.Y[i][j] << " ";
        }
        cout << endl;
    }

    //Escreve a linha Z
    cout << " Z";
    for (int i = 0; i < T.z_c.size(); i++) {
        cout << fixed << setprecision(2) << setw(12) << T.z_c[i] << " ";
    }
    cout << endl << endl;

}

void criaPrimeiroTableau(const Dados & d, Tableau & T) {

    //Insere 0 para cada uma das variáveis adicionais e o valor da função objetivo z
    for (int i = 1; i <= d.nConstraints + 1; i++) {
        T.z_c.push_back(0);
    }

    //Para cada uma das restrições insere 0 ou 1, correspondendo as variáveis adicionais
    for (int i = 1; i < d.nConstraints + 1; i++) {
        for (int j = 1; j < d.nConstraints + 1; j++) {
            if (i == j) {
               T.Y[i].insert(T.Y[i].end() - 1, 1); 
            } else {
               T.Y[i].insert(T.Y[i].end() - 1, 0);
            }
        }
    }

    //Valores das variáveis básicas
    for (int i = 1; i < d.nConstraints + 1; i++) {
        T.x_B.push_back(T.Y[i][T.Y[i].size() - 1]);
    }

    //Determina as variáveis básica/não-básicas
    for (int i = 1; i <= d.nConstraints + d.nVariables; i++) {
        if (i <= d.nVariables) {
            T.nb_indx.push_back(i);
        } else {
            T.b_indx.push_back(i);
        }
    }

     //Determina se vai ser necessário utilizar o método Big-M
    int aux = 0;
    for (int i = 1; i < d.nConstraints + 1; i++) {
        if (T.Y[i][T.Y[i].size() - 1] < 0) {
            aux = 1;
        }
        T.rB.push_back(T.Y[i][T.Y[i].size() - 1]);
    }   


    //Inicia Big-M
    int cont = 0;
    if (aux == 1) {
        int M = -9999; //Valor do Big-M;
        vector <int> indices; //Armeazena os índices das variáveis em que foi necessário multiplicar a linha do tableau por -1
        for (int i = 1; i < d.nConstraints + 1; i++) {
            if (T.Y[i][T.Y[i].size() - 1] < 0) {
                for (int j = 0; j < T.Y[i].size(); j++) {
                    if (T.Y[i][j] != 0) {
                        T.Y[i][j] = -1 * T.Y[i][j];
                    }
                } //Se o valor da variável for menor que 0, multiplica a linha inteira por -1
                T.x_B[i - 1] = -1 * T.x_B[i - 1]; //Altera o valor da variável multiplicando ela por -1
                indices.push_back(i); //Salva o índice da variável
                cont++; //Incrementa para saber quantas vezes o processo acontece
            } 
        }

        //Insere o valor de M na função objetivo baseado na quantidade de variáveis
        for (int i = 0; i < cont; i++) {
            T.z_c.insert(T.z_c.end() - 1, M);
        }

        int k = 0;
        int aux = 0;
    
        for (int i = 1; i < d.nConstraints + 1; i++) {
            if (i != indices[k]) {
                for (int j = 0; j < cont; j++) {
                    T.Y[i].insert(T.Y[i].end() - 1, 0);
                }
            } else {
                for (int j = 0; j < cont; j++) {
                    if (aux == j) {
                        T.Y[i].insert(T.Y[i].end() - 1, 1);
                    } else {
                        T.Y[i].insert(T.Y[i].end() - 1, 0);
                    }
                }
                aux++;
                k++;
            }
            

        }
            
        for (int k = 0; k < indices.size(); k++) {
            for (int i = 0; i < T.z_c.size(); i++) {
                T.z_c[i] = T.z_c[i] - T.Y[indices[k]][i] * M;
            }
        }

        for (int i = 0; i < indices.size(); i++) {
            vector<int>::iterator location = find(T.b_indx.begin(), T.b_indx.end(), indices[i] + d.nVariables);
            T.nb_indx.push_back(*location);
            T.b_indx.erase(location);
        }

        for (int i = d.nConstraints + d.nVariables + 1; i <= d.nConstraints + d.nVariables + cont ; i++) {
            T.b_indx.push_back(i);
        }
    }    

}

int checaParada (const Tableau & T) {

    //Conta quantos valores na linha da função objetivo são menores ou iguais a zero
    int cont = 0;
    for (int i = 0; i < T.z_c.size() - 1; i++) {
        if (T.z_c[i] <= 0) {
            cont++;
        }
    }

    /*Se a quantidade de valores na linha da função objetivo menores ou iguais a zero forem iguais ao tamanho do vetor, 
    retorna -1, significa que o simplex acabou. Caso contrário, retorne o índice do maior elemento da linha*/
    if (cont == T.z_c.size() - 1) {
        return -1;
    } else {
        return max_element(T.z_c.begin(), T.z_c.end() - 1) - T.z_c.begin();
    }

}

int linhaPivo (const Dados & d, const Tableau & T, const int & colunaPivo) {
    
    double menor = 99999;
    int indice;
    int cont = 0;
    for (int i = 1; i < d.nConstraints + 1; i++) {
        if (T.Y[i][colunaPivo] > 0) {
            double razao =  T.x_B[i-1] / T.Y[i][colunaPivo]; //Calcula a razão 

            //Armazena a menor razão, pois significa o maior aumento que pode ser alcançado na função objetivo
            if (razao < menor) {
                menor = razao;
                indice = i;
            } 
        } else {
            cont++; //Incrementa cont caso o valor a ser dividido não seja maior que zero
        }
    }

    /*Caso cont seja diferente do número de restrições,  significa que o simplex pode continuar e um aumento na 
    função objetivo pode ser obtido. Caso contrário, retorna -1 e o problema é unbounded.*/
    if (cont != d.nConstraints) {
        return indice;
    } else {
        return -1;
    }

}

void divideLinhaPivo (Tableau & T, const int & lPivo, const int & cPivo) {

    //Divide todos os elementos da linha pivo pelo elemento pivo
    double valor = T.Y[lPivo][cPivo];
    for (int j = 0; j < T.Y[lPivo].size(); j++) {
        T.Y[lPivo][j] = T.Y[lPivo][j] / valor;
    }

}

void modificaBase (const Dados & d, Tableau & T, const int & lPivo, const int & cPivo) {

    vector <double> auxiliar;
    auxiliar = T.Y[lPivo];
    double valor;
    for (int i = 1; i < d.nConstraints + 1; i++) {
        for (int j = 0; j < T.Y[i].size(); j++) {
            if (i != lPivo) {
                if (j == 0) {
                    valor = T.Y[i][cPivo];
                }

                T.Y[i][j] = T.Y[i][j] - auxiliar[j] * valor;
            }
        }
    }

    for (int i = 0; i < T.z_c.size(); i++) {
        if (i == 0) {
            valor = T.z_c[cPivo];
        }
        
        T.z_c[i] = T.z_c[i] - auxiliar[i] * valor;
    }

    T.obj = -T.z_c[T.z_c.size() - 1];

    //cout << T.b_indx[lPivo - 1] << " " << T.nb_indx[cPivo] << endl << endl;
   /* double aux = T.b_indx[lPivo - 1];
    T.b_indx[lPivo - 1] = T.nb_indx[cPivo];
    T.nb_indx[cPivo] = aux;*/

    double aux = T.b_indx[lPivo - 1];
    vector <int> :: iterator p = find(T.nb_indx.begin(), T.nb_indx.end(), cPivo + 1);
    T.b_indx[lPivo - 1] = T.nb_indx[p - T.nb_indx.begin()];
    T.nb_indx[p - T.nb_indx.begin()] = aux;

    //cout << T.b_indx[lPivo - 1] << " " << T.nb_indx[cPivo] << endl;
    for (int i = 1; i < d.nConstraints + 1; i++) {
        T.x_B[i-1] = T.Y[i][T.Y[i].size() - 1];
    }

}

void mostraUnbounded(const Dados & d, const Tableau & T, const int & cPivo) {

    cout << "Faça:\n\n";
    for (int i = 0; i < T.nb_indx.size(); i++) {
        if (T.nb_indx[i] <= d.nVariables) {
            if (T.nb_indx[i] == T.nb_indx[cPivo]) {
                cout << "x" << T.nb_indx[cPivo] << " = t" << endl;
            } else {
                cout << "x" << T.nb_indx[i] << " = 0" << endl;
            }
        }
    }

    for (int i = 0; i < T.b_indx.size(); i++) {
        if (T.b_indx[i] <= d.nVariables) {
            cout << "x" << T.b_indx[i] << " = " << T.x_B[i]  << " + " << -(T.Y[i+1][cPivo]) << "t" << endl;
        }
    }

}

void analiseSensibilidade (Tableau & T, Dados & d) {

    cout << "\t\tANÁLISE DE SENSIBILIDADE" << endl << endl;

    double matrizFolga[d.nConstraints][d.nConstraints];
    double maiorValor[d.nConstraints], maiorValorAux[d.nConstraints];
    double menorValor[d.nConstraints], menorValorAux[d.nConstraints];

    for (int i = 0; i < d.nConstraints; i++) {
        if (T.rB[i] >= 0) {
            maiorValor[i] = 9999;
            menorValor[i] = -9999;
        } else {
            maiorValor[i] = -9999;
            menorValor[i] = 9999;
        }

        maiorValorAux[i] = 0;
        menorValorAux[i] = 0;
    }

    int k, l;
    k = l = 0;

    for (int i = 1; i < d.nConstraints + 1; i++) {
        for (int j = d.nVariables; j < d.nVariables + d.nConstraints; j++) {
            matrizFolga[k][l] = T.Y[i][j];
            l++;
        }
        k++;
        l = 0;
    }

    double auxiliar;
    for (int i = 0; i < d.nConstraints; i++) {
        for (int j = 0; j < d.nConstraints; j++) {
            for (int k = 0; k < d.nConstraints; k++) {
                if (i != k) {
                    maiorValorAux[i] += matrizFolga[j][k] * T.rB[k];  
                } else {
                    auxiliar = matrizFolga[j][k];
                }
            }
            
            if (auxiliar != 0) {
                auxiliar = -1 * auxiliar;
            }

            
            if (T.rB[i] >= 0 ) {
                if (auxiliar < 0 && (maiorValorAux[i] / auxiliar) > menorValor[i]) {
                    menorValor[i] = maiorValorAux[i] / auxiliar;
                } else if (auxiliar > 0 && (maiorValorAux[i] / auxiliar) < maiorValor[i]) {
                    maiorValor[i] = maiorValorAux[i] / auxiliar;
                }
            } else {
                if (auxiliar < 0 && (maiorValorAux[i] / auxiliar) > maiorValor[i]) {
                    maiorValor[i] = maiorValorAux[i] / auxiliar;
                } else if (auxiliar > 0 && (maiorValorAux[i] / auxiliar) < menorValor[i]) {
                    menorValor[i] = maiorValorAux[i] / auxiliar;
                }
            }
 
            maiorValorAux[i] = 0;
            menorValorAux[i] = 0;
        }

    }

    cout << "\tValores do lado direito das restrições" << endl << endl;
    cout << "Restrição\t" << "Coeficiente atual\t" << "Incremento Permitido\t" << "Decremento Permitido " << endl;

    for (int i = 0; i < d.nConstraints; i++) {
        if (T.rB[i] >= 0) {
            if (maiorValor[i] == 9999) {
                cout << setw(3) << i+1 << setw(20) << T.rB[i] << setw(27) << "inf" << setw(27) << T.rB[i] - menorValor[i] << endl;
            } else if (menorValor[i] == -9999) {
                cout << setw(3) << i+1 << setw(20) << T.rB[i] << setw(27) << maiorValor[i] - T.rB[i] << setw(27) << "inf" << endl;
            } else {
                cout << setw(3) << i+1 << setw(20) << T.rB[i] << setw(27) << maiorValor[i] - T.rB[i] << setw(27) << T.rB[i] - menorValor[i] << endl;
            }
        } else {
            if (maiorValor[i] == -9999) {
                cout << setw(3) << i+1 << setw(20) << T.rB[i] << setw(27) << "inf" << setw(27) << menorValor[i] - T.rB[i] << endl;
            } else if (menorValor[i] == 9999) {
                cout << setw(3) << i+1 << setw(20) << T.rB[i] << setw(27) << T.rB[i] - maiorValor[i] << setw(27) << "inf" << endl;
            } else {
                cout << setw(3) << i+1 << setw(20) << T.rB[i] << setw(27) << T.rB[i] - maiorValor[i] << setw(27) << menorValor[i] - T.rB[i] << endl;
            }
        }
        
    }
    cout << endl;

    double maiorVal[d.nVariables];
    double menorVal[d.nVariables];

    for (int i = 0; i < d.nVariables; i++) {
        maiorVal[i] = 999;
        menorVal[i] = 999;
    }

    double teste;
    int indiceValor = 0;
    for (int i = 0; i < T.b_indx.size(); i++) {
        if (T.b_indx[i] <= d.nVariables) {
            for (int j = 1; j <= d.nVariables + d.nConstraints; j++) {
                if (count(T.nb_indx.begin(), T.nb_indx.end(), j)) {
                    //vector <int> :: iterator p = find(T.b_indx.begin(), T.b_indx.end(), i);
        
                    auxiliar = -1 * T.z_c[j - 1];

                    //cout << i << "q " << auxiliar << " " << T.Y[i+1][j-1] << " " << auxiliar / T.Y[i+1][j-1] << endl << endl;

                    if (auxiliar > 0 && T.Y[i+1][j-1] > 0) {
                        teste = -auxiliar / T.Y[i+1][j-1];
                        //cout << i << " " << teste << " " << menorVal[indiceValor] << " 1" << endl;
                        if (teste < 0) {
                            teste = -1 * teste;
                        }

                        if (teste <= menorVal[indiceValor]) {
                            menorVal[indiceValor] = teste;

                            if (menorVal[indiceValor] < 0) {
                                menorVal[indiceValor] = -1 * menorVal[indiceValor];
                            }
                        }

                    } else if (auxiliar < 0 && T.Y[i+1][j-1] > 0) {
                        teste = auxiliar / T.Y[i+1][j-1];
                        //cout << i << " " << teste << " " << menorVal[indiceValor] << " 2" << endl;

                        if (teste < 0) {
                            teste = -1 * teste;
                        }

                        if (teste <= menorVal[indiceValor]) {
                            menorVal[indiceValor] = teste;

                            if (menorVal[indiceValor] < 0) {
                                menorVal[indiceValor] = -1 * menorVal[indiceValor];
                            }
                        }
                    } else if (auxiliar > 0 && T.Y[i+1][j-1] < 0) {
                        teste = -auxiliar / (-T.Y[i+1][j-1]);
                        //cout << i << " " << teste << " " << maiorVal[indiceValor] << " 3" << endl;

                        if (teste < 0) {
                            teste = -1 * teste;
                        }

                        if (teste <= maiorVal[indiceValor]) {
                            maiorVal[indiceValor] = teste;

                            if (maiorVal[indiceValor] < 0) {
                                maiorVal[indiceValor] = -1 * maiorVal[indiceValor];
                            }
                        }
                    } else if (auxiliar < 0 && T.Y[i+1][j-1] < 0) {
                        teste = auxiliar / (-T.Y[i+1][j-1]);
                        //cout << i << " " << teste << " " << maiorVal[indiceValor] << " 4" << endl;

                        if (teste < 0) {
                            teste = -1 * teste;
                        }

                        if (teste <= maiorVal[indiceValor]) {
                            maiorVal[indiceValor] = teste;

                            if (maiorVal[indiceValor] < 0) {
                                maiorVal[indiceValor] = -1 * maiorVal[indiceValor];
                            }
                        }
                    }
                    
                   /* if ((auxiliar / T.Y[i+1][j-1] ) < menorVal[indiceValor]) {
                        menorVal[indiceValor] = auxiliar / T.Y[i+1][j-1] ;
                    } else if ((auxiliar / T.Y[i+1][j-1] ) > maiorVal[indiceValor]) {
                        maiorVal[indiceValor] = auxiliar / T.Y[i+1][j-1] ;
                    }*/
                }
            }
            indiceValor += 1;
        }
    }

    cout << endl;
    cout << "\tValores dos coeficientes da função objetivo" << endl << endl;
    cout << "Variável\t" << "Coeficiente atual\t" << "Incremento Permitido\t" << "Decremento Permitido " << endl;

    for (int i = 0; i < d.nVariables; i++) {
        if (menorVal[i] < 0) {
            menorVal[i] = -1 * menorVal[i];
        } else if (maiorVal[i] < 0) {
            maiorVal[i] = -1 * maiorVal[i];
        }
    }

    indiceValor = 0;
    for (int i = 0; i < T.b_indx.size(); i++) {
        if (T.b_indx[i] <= d.nVariables) {

            if (maiorVal[indiceValor] == 999.00) {
                cout << setw(3) << T.b_indx[i] << setw(20) << T.fo[T.b_indx[i] - 1] << setw(27) << "inf" << setw(27) << menorVal[indiceValor] << endl;
            } else if (menorVal[indiceValor] == 999.00) {
                cout << setw(3) << T.b_indx[i] << setw(20) << T.fo[T.b_indx[i] - 1] << setw(27) << maiorVal[indiceValor] << setw(27) << "inf" << endl;
            } else {
                cout << setw(3) << T.b_indx[i] << setw(20) << T.fo[T.b_indx[i] - 1] << setw(27) << maiorVal[indiceValor] << setw(27) << menorVal[indiceValor] << endl;
            }
            indiceValor++;
        }

    }

    for (int i = 0; i < T.nb_indx.size(); i++) {
        if (T.nb_indx[i] <= d.nVariables) {
            cout << setw(3) << T.nb_indx[i] << setw(20) << T.fo[T.nb_indx[i] - 1] << setw(27) << -T.z_c[T.nb_indx[i] - 1] << setw(27) << "inf" << endl;
        }
    }

}

void printaSolucaoDual (Tableau & T, Dados & d) {

    cout << "Solução dual: " << endl << endl;

    int cont = 1;
    for (int i = d.nVariables; i < d.nConstraints + d.nVariables; i++) {
        if (T.z_c[i] != 0) {
            cout << "y" << cont << " = " << -T.z_c[i] << endl;
            cont++;
        } else {
            cout << "y" << cont << " = 0.00" << endl;
            cont++;
        }
    }
    cout << endl;

}