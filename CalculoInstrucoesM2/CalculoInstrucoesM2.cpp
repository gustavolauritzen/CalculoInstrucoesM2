#include <iostream>
#include <fstream>
#include <string>
#include "lde.h"

// Desenvolvido por Matheus Baron Lauritzen e Gustavo Baron Lauritzen

using namespace std;

struct Organizacao {
	double CPI, cpiPipeline, sobreCustoInstrucao, tExec, ciclosProg, totalCiclos, contadorInstrucaoOriginal, contadorInstrucaoFinal, tempoClock, tempoExecucaoCPU;
};

struct Instrucao {
	string instCompleta, opCode, rs1, rs2, rd;
};

void calcularImprimirDesempenho(Organizacao& org) {

	org.sobreCustoInstrucao = org.contadorInstrucaoFinal - org.contadorInstrucaoOriginal;
	org.CPI = (5 + 1 * (org.contadorInstrucaoFinal - 1)) / org.contadorInstrucaoFinal;
	org.ciclosProg = (5 + 1 * (org.contadorInstrucaoFinal - 1));
	org.tExec = org.contadorInstrucaoFinal * org.CPI * org.tempoClock;

	cout << "Sobrecusto em instrucoes da solucao: " << org.sobreCustoInstrucao << endl;
	cout << "Ciclos por Instrucao(CPI): " << org.CPI << endl;
	cout << "Numero de Ciclos do Programa: " << org.ciclosProg << endl;
	cout << "Tempo de Execucao: " << org.tExec << endl;

}

//  Indice baseado em 0 do primeiro caractere que voce  deseja pegar e indice baseado em 0 do ultimo caractere que voce deseja pegar
void leBinarioArquivoEscreveLista(Organizacao& org, LDE <string>& lista, string nomeArquivo) {
	ifstream file(nomeArquivo);
	int start = 0;
	int end = 31;
	org.contadorInstrucaoOriginal = 0;
	string instrucao;
	if (!file.is_open()) {
		cout << "Nao foi possivel abrir o arquivo." << endl;
		return;
	}
	string line;
	while (getline(file, line)) {

		if (start >= 0 && end < line.length()) {
			instrucao = line.substr(start, end - start + 1);
			inserirFinalLDE(lista, instrucao);
			org.contadorInstrucaoOriginal++;
		}
		else {
			cout << "Intervalo fora dos limites da linha." << endl;
		}
	}
	file.close();
}

string verificaInstrucao(string inst) {
	int startOpcode = 25, endOpcode = 31;
	Instrucao instrucao;

	instrucao.opCode = inst.substr(startOpcode, endOpcode - startOpcode + 1);

	if ((inst == "00000000000000000000000000010011 nop") || (inst == "00000000000000000000000001110011")) { //nop ou ecall
		return "NOP";
	}
	else if (instrucao.opCode == "0110111" || instrucao.opCode == "0010111") {  //instrucao tipo U  
		return "U";
	}
	else if (instrucao.opCode == "1101111" || instrucao.opCode == "1100111") { //instrucao tipo J
		return "J";
	}
	else if (instrucao.opCode == "1100011") { //instrucao tipo B
		return "B";
	}
	else if (instrucao.opCode == "0000011") { //instrucao tipo I memoria
		return "IM";
	}
	else if (instrucao.opCode == "0100011") { //instrucao tipo S
		return "S";
	}
	else if (instrucao.opCode == "0010011" || instrucao.opCode == "1110011") { //instrucao tipo I aritmetico e ecall
		return "IAE";
	}
	else if (instrucao.opCode == "0110011") { //instrucao tipo R
		return "R";
	}
	return "";
}

void escreveListaParaTxt(LDE<string> lista, string nomeArquivo, Organizacao& org) {
	ofstream arquivo(nomeArquivo);
	org.contadorInstrucaoFinal = 0;
	if (!arquivo.is_open()) {
		cout << "Não foi possível abrir o arquivo." << endl;
		return;
	}

	No<string>* atual = lista.comeco;

	while (atual != nullptr) {
		arquivo << atual->info << endl;
		org.contadorInstrucaoFinal++;
		atual = atual->eloP;
	}

	arquivo.close();
}

void insere2Nop(LDE<string>& lista, No<string>*& aux) {
	No <string>* proximo = new No <string>;
	No <string>* nop1 = new No <string>;
	No <string>* nop2 = new No <string>;

	//dump Nop 32bits = 00000000000000000000000000010011 ******* addi zero, zero, 0
	nop1->info = "00000000000000000000000000010011 nop"; // addi zero, zero, 0
	nop2->info = "00000000000000000000000000010011 nop"; // addi zero, zero, 0

	//inserindo 2 nop
	proximo = aux->eloP;
	aux->eloP = nop1;
	nop1->eloA = aux;
	nop1->eloP = nop2;
	nop2->eloA = nop1;
	nop2->eloP = proximo;
	proximo->eloA = nop2;
}

void insere1Nop(LDE<string>& lista, No<string>*& aux) {
	No <string>* proximo = new No <string>;
	No <string>* nop1 = new No <string>;

	nop1->info = "00000000000000000000000000010011 nop"; // addi zero, zero, 0

	//inserindo 1 nop
	proximo = aux->eloP;
	aux->eloP = nop1;
	nop1->eloA = aux;
	nop1->eloP = proximo;
	proximo->eloA = nop1;

}

void reinserir2Inst(LDE <string>& listaOriginal, No<string>*& atual, No<string>*& inst1, No<string>*& inst2) {
	No <string>* proximo = new No <string>;
	proximo = atual->eloP;

	atual->eloP = inst1;
	inst1->eloA = atual;
	inst1->eloP = inst2;
	inst2->eloA = inst1;
	inst2->eloP = proximo;
	proximo->eloA = inst2;
}

void reinserirInst(LDE <string>&  listaOriginal, No<string>*& atual, No<string>*&  aux) {
	No <string>* proximo = new No <string>;

	//mexendo em no
	proximo = atual->eloP;
	atual->eloP = aux;
	aux->eloA = atual;
	aux->eloP = proximo;
	proximo->eloA = aux;

}

void reinserirInstInserir1Nop(LDE <string>& listaOriginal, No<string>*& atual, No<string>*& inst) {
	No <string>* proximo = new No <string>;
	proximo = atual->eloP;

	No <string>* nop1 = new No <string>;
	nop1->info = "00000000000000000000000000010011 nop"; // addi zero, zero, 0

	atual->eloP = inst;
	inst->eloA = atual;
	inst->eloP = nop1;
	nop1->eloA = inst;
	nop1->eloP = proximo;
	proximo->eloA = nop1;

}

bool reordenar1InstForwarding(LDE <string>& listaOriginal, No<string>*& atual) {
	int startRd = 20, endRd = 24;
	int startRs1 = 12, endRs1 = 16;
	int startRs2 = 7, endRs2 = 11;
	No<string>* aux = new No<string>;
	No<string>* auxanterior1 = new No<string>;
	No<string>* auxanterior2 = new No<string>;
	No<string>* atualproximo1 = new No<string>;
	No<string>* atualproximo2 = new No<string>;
	No<string>* atualanterior1 = new No<string>;
	string retornoAuxAnterior1 = "", retornoAuxAnterior2 = "", retornoAux = "", auxRs1 = "", auxRs2 = "";
	if (atual != nullptr) {
		if (atual->eloP != nullptr) {
			atualproximo1 = atual->eloP;
			if (atual->eloP->eloP != nullptr) {
				atualproximo2 = atual->eloP->eloP;
				aux = atualproximo2;
			}
			else {
				aux->eloP = nullptr;
			}
		}
		if (atual->eloA != nullptr) {
			atualanterior1 = atual->eloA;
		}
	}
	
	while (aux->eloP != nullptr) {
		auxRs1 = aux->info.substr(startRs1, endRs1 - startRs1 + 1);
		auxRs2 = aux->info.substr(startRs2, endRs2 - startRs2 + 1);


		if (aux->eloA != nullptr) {
			auxanterior1 = aux->eloA;
			if (aux->eloA->eloA != nullptr) {
				auxanterior2 = aux->eloA->eloA;
			}
			else {
				auxanterior2 = nullptr;
			}
		}
		else {
			auxanterior1 = nullptr;
		}

		retornoAux = verificaInstrucao(aux->info);
		if (auxanterior1 != nullptr) {
			retornoAuxAnterior1 = verificaInstrucao(auxanterior1->info);
			if (auxanterior2 != nullptr) {
				retornoAuxAnterior2 = verificaInstrucao(auxanterior2->info);
			}
		}

		if (retornoAux == "J") {
			return false;
		}


		if ((retornoAux == "B") || (retornoAux == "S") || (retornoAux == "R")) {
			if (auxanterior1 != nullptr) {
				//Verifica se existe uma instrucao possivel de retirar
				if (retornoAuxAnterior1 == "IM") {
					if ((auxRs1 != auxanterior1->info.substr(startRd, endRd - startRd + 1)) && (auxRs2 != auxanterior1->info.substr(startRd, endRd - startRd + 1))) {
						//Verifica se existe uma instrucao possivel de reinserir
						if (atual != nullptr) {
							if (atualanterior1 != nullptr) {
								//verifica os anteriores do atual(conflito) para ver se nao ira gerar problema
								if ((auxRs1 != atual->info.substr(startRd, endRd - startRd + 1) && (auxRs2 != atual->info.substr(startRd, endRd - startRd + 1))) && ((auxRs1 != atualanterior1->info.substr(startRd, endRd - startRd + 1)) && (auxRs2 != atualanterior1->info.substr(startRd, endRd - startRd + 1)))) {
									//verifica os proximos do atual(conflito) para ver se nao ira gerar problema
									if (atualproximo1 != nullptr) {
										if (atualproximo2 != nullptr) {
											if (((auxRs1 != atualproximo1->info.substr(startRd, endRd - startRd + 1)) && (auxRs2 != atualproximo1->info.substr(startRd, endRd - startRd + 1))) && ((auxRs1 != atualproximo2->info.substr(startRd, endRd - startRd + 1)) && (auxRs2 != atualproximo2->info.substr(startRd, endRd - startRd + 1)))) {
												retirarLDE(listaOriginal, aux);
												reinserirInst(listaOriginal, atual, aux);
												return true;
											}
										}
									}
								}
							}
						}
					}
				} else {
					retirarLDE(listaOriginal, aux);
					reinserirInst(listaOriginal, atual, aux);
					return true;
				}
			}
		}

		if ((retornoAux == "IAE") || (retornoAux == "IM")) {
			if (auxanterior1 != nullptr) {
				//Verifica se existe uma instrucao possivel de retirar
				if (retornoAuxAnterior1 == "IM") {
					if (auxRs1 != auxanterior1->info.substr(startRd, endRd - startRd + 1)) {
						//Verifica se existe uma instrucao possivel de reinserir
						if (atual != nullptr) {
							if (atualanterior1 != nullptr) {
								//verifica os anteriores do atual(conflito) para ver se nao ira gerar problema
								if ((auxRs1 != atual->info.substr(startRd, endRd - startRd + 1)) && (auxRs1 != atualanterior1->info.substr(startRd, endRd - startRd + 1))) {
									//verifica os proximos do atual(conflito) para ver se nao ira gerar problema
									if (atualproximo1 != nullptr) {
										if (atualproximo2 != nullptr) {
											if ((auxRs1 != atualproximo1->info.substr(startRd, endRd - startRd + 1)) && (auxRs1 != atualproximo2->info.substr(startRd, endRd - startRd + 1))) {
												retirarLDE(listaOriginal, aux);
												reinserirInst(listaOriginal, atual, aux);
												return true;
											}
										}
									}
								}
							}
						}
					}
				} else {
					retirarLDE(listaOriginal, aux);
					reinserirInst(listaOriginal, atual, aux);
					return true;
				}
			}
		}

		aux = aux->eloP;
	}
	return false;
}

bool reordenar1Inst(LDE <string>& listaOriginal, No<string>*& atual) {
	if (verificaInstrucao(atual->info) == "J") {
		return false;
	}
	int startRd = 20, endRd = 24;
	int startRs1 = 12, endRs1 = 16;
	int startRs2 = 7, endRs2 = 11;
	No<string>* aux = new No<string>;
	No<string>* auxanterior1 = new No<string>;
	No<string>* auxanterior2 = new No<string>;
	No<string>* atualproximo1 = new No<string>;
	No<string>* atualproximo2 = new No<string>;
	No<string>* atualanterior1 = new No<string>;
	string retornoAuxAnterior1 = "", retornoAuxAnterior2 = "", retornoAux = "", auxRs1 = "", auxRs2 = "";
	if (atual != nullptr) {
		if (atual->eloP != nullptr) {
			atualproximo1 = atual->eloP;
			if (atual->eloP->eloP != nullptr) {
				atualproximo2 = atual->eloP->eloP;
				aux = atualproximo2;
			}
			else {
				aux->eloP = nullptr;
			}
		}
		if (atual->eloA != nullptr) {
			atualanterior1 = atual->eloA;
		}
	}

	while (aux != nullptr) {
		auxRs1 = aux->info.substr(startRs1, endRs1 - startRs1 + 1);
		auxRs2 = aux->info.substr(startRs2, endRs2 - startRs2 + 1);


		if (aux->eloA != nullptr) {
			auxanterior1 = aux->eloA;
			if (aux->eloA->eloA != nullptr) {
				auxanterior2 = aux->eloA->eloA;
			}
			else {
				auxanterior2 = nullptr;
			}
		}
		else {
			auxanterior1 = nullptr;
		}

		retornoAux = verificaInstrucao(aux->info);
		if (auxanterior1 != nullptr) {
			retornoAuxAnterior1 = verificaInstrucao(auxanterior1->info);
			if (auxanterior2 != nullptr) {
				retornoAuxAnterior2 = verificaInstrucao(auxanterior2->info);
			}
		}

		if (retornoAux == "J") {
			return false;
		}


		if ((retornoAux == "B") || (retornoAux == "S") || (retornoAux == "R")) {
			if (auxanterior1 != nullptr) {
				if (auxanterior2 != nullptr) {
					//Verifica se existe uma instrucao possivel de retirar
					if (((auxRs1 != auxanterior1->info.substr(startRd, endRd - startRd + 1)) && (auxRs2 != auxanterior1->info.substr(startRd, endRd - startRd + 1))) && ((auxRs1 != auxanterior2->info.substr(startRd, endRd - startRd + 1)) && (auxRs2 != auxanterior2->info.substr(startRd, endRd - startRd + 1)))) {
						//Verifica se existe uma instrucao possivel de reinserir
						if (atual != nullptr) {
							if (atualanterior1 != nullptr) {
								//verifica os anteriores do atual(conflito) para ver se nao ira gerar problema
								if ((auxRs1 != atual->info.substr(startRd, endRd - startRd + 1) && (auxRs2 != atual->info.substr(startRd, endRd - startRd + 1))) && ((auxRs1 != atualanterior1->info.substr(startRd, endRd - startRd + 1)) && (auxRs2 != atualanterior1->info.substr(startRd, endRd - startRd + 1)))) {
									//verifica os proximos do atual(conflito) para ver se nao ira gerar problema
									if (atualproximo1 != nullptr) {
										if (atualproximo2 != nullptr) {
											if (((auxRs1 != atualproximo1->info.substr(startRd, endRd - startRd + 1)) && (auxRs2 != atualproximo1->info.substr(startRd, endRd - startRd + 1))) && ((auxRs1 != atualproximo2->info.substr(startRd, endRd - startRd + 1)) && (auxRs2 != atualproximo2->info.substr(startRd, endRd - startRd + 1)))) {
												retirarLDE(listaOriginal, aux);
												reinserirInst(listaOriginal, atual, aux);
												return true;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}

		if ((retornoAux == "IAE") || (retornoAux == "IM")) {
			if (auxanterior1 != nullptr) {
				if (auxanterior2 != nullptr) {
					//Verifica se existe uma instrucao possivel de retirar
					if ((auxRs1 != auxanterior1->info.substr(startRd, endRd - startRd + 1)) && (auxRs1 != auxanterior2->info.substr(startRd, endRd - startRd + 1))) {
						//Verifica se existe uma instrucao possivel de reinserir
						if (atual != nullptr) {
							if (atualanterior1 != nullptr) {
								//verifica os anteriores do atual(conflito) para ver se nao ira gerar problema
								if ((auxRs1 != atual->info.substr(startRd, endRd - startRd + 1)) && (auxRs1 != atualanterior1->info.substr(startRd, endRd - startRd + 1))) {
									//verifica os proximos do atual(conflito) para ver se nao ira gerar problema
									if (atualproximo1 != nullptr) {
										if (atualproximo2 != nullptr) {
											if ((auxRs1 != atualproximo1->info.substr(startRd, endRd - startRd + 1)) && (auxRs1 != atualproximo2->info.substr(startRd, endRd - startRd + 1))) {
												retirarLDE(listaOriginal, aux);
												reinserirInst(listaOriginal, atual, aux);
												return true;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}

		aux = aux->eloP;
	}
	return false;
}

bool reordenar2Inst(LDE <string>& listaOriginal, No<string>*& atual) {
	int startRd = 20, endRd = 24;
	int startRs1 = 12, endRs1 = 16;
	int startRs2 = 7, endRs2 = 11;
	No<string>* aux = new No<string>;
	No<string>* auxanterior1 = new No<string>;
	No<string>* auxanterior2 = new No<string>;
	No<string>* atualproximo1 = new No<string>;
	No<string>* atualproximo2 = new No<string>;
	No<string>* atualanterior1 = new No<string>;
	No<string>* instBoa1 = new No<string>;
	No<string>* instBoa2 = new No<string>;
	string retornoAuxAnterior1 = "", retornoAuxAnterior2 = "", retornoAux = "", auxRs1 = "", auxRs2 = "", atualRd = "", atualAnterior1Rd = "";
	instBoa1->info = "";
	instBoa2->info = "";
	if (atual != nullptr) {
		atualRd = atual->info.substr(startRd, endRd - startRd + 1);
		if (atual->eloP != nullptr) {
			atualproximo1 = atual->eloP;
			if (atual->eloP->eloP != nullptr) {
				atualproximo2 = atual->eloP->eloP;
				aux = atualproximo2;
			}
			else {
				aux->eloP = nullptr;
			}
		}
		if (atual->eloA != nullptr) {
			atualanterior1 = atual->eloA;
			atualAnterior1Rd = atualanterior1->info.substr(startRd, endRd - startRd + 1);
		}
	}
	
	while (aux->eloP != nullptr) {
		auxRs1 = aux->info.substr(startRs1, endRs1 - startRs1 + 1);
		auxRs2 = aux->info.substr(startRs2, endRs2 - startRs2 + 1);


		if (aux->eloA != nullptr) {
			auxanterior1 = aux->eloA;
			if (aux->eloA->eloA != nullptr) {
				auxanterior2 = aux->eloA->eloA;
			}
			else {
				auxanterior2 = nullptr;
			}
		}
		else {
			auxanterior1 = nullptr;
		}

		retornoAux = verificaInstrucao(aux->info);
		if (auxanterior1 != nullptr) {
			retornoAuxAnterior1 = verificaInstrucao(auxanterior1->info);
			if (auxanterior2 != nullptr) {
				retornoAuxAnterior2 = verificaInstrucao(auxanterior2->info);
			}
		}

		if (retornoAux == "J") {
			break;
		}


		if ((retornoAux == "B") || (retornoAux == "S") || (retornoAux == "R")) {
			if (auxanterior1 != nullptr) {
				if (auxanterior2 != nullptr) {
					//Verifica se existe uma instrucao possivel de retirar
					if (((auxRs1 != auxanterior1->info.substr(startRd, endRd - startRd + 1)) && (auxRs2 != auxanterior1->info.substr(startRd, endRd - startRd + 1))) && ((auxRs1 != auxanterior2->info.substr(startRd, endRd - startRd + 1)) && (auxRs2 != auxanterior2->info.substr(startRd, endRd - startRd + 1)))) {
						//Verifica se existe uma instrucao possivel de reinserir
						if (atual != nullptr) {
							if (atualanterior1 != nullptr) {
								//verifica os anteriores do atual(conflito) para ver se nao ira gerar problema
								if ((auxRs1 != atual->info.substr(startRd, endRd - startRd + 1)) && (auxRs2 != atual->info.substr(startRd, endRd - startRd + 1)) && ((auxRs1 != atualanterior1->info.substr(startRd, endRd - startRd + 1)) && (auxRs2 != atualanterior1->info.substr(startRd, endRd - startRd + 1)))) {
									//verifica os proximos do atual(conflito) para ver se nao ira gerar problema
									if (atualproximo1 != nullptr) {
										if ((auxRs1 != atualproximo1->info.substr(startRd, endRd - startRd + 1)) && (auxRs2 != atualproximo1->info.substr(startRd, endRd - startRd + 1))) {
											if (instBoa1->info == "") {
												instBoa1 = aux;
												retirarLDE(listaOriginal, instBoa1);
												aux = aux->eloP;
												continue;
											}
										}
									}
								}
							}
							if ((auxRs1 != atual->info.substr(startRd, endRd - startRd + 1) && (auxRs2 != atual->info.substr(startRd, endRd - startRd + 1)))) {
								if (atualproximo1 != nullptr) {
									if (atualproximo2 != nullptr) {
										if (((auxRs1 != atualproximo1->info.substr(startRd, endRd - startRd + 1)) && (auxRs2 != atualproximo1->info.substr(startRd, endRd - startRd + 1))) && ((auxRs1 != atualproximo2->info.substr(startRd, endRd - startRd + 1)) && (auxRs2 != atualproximo2->info.substr(startRd, endRd - startRd + 1)))) {
											if (instBoa1->info != "") {
												if (instBoa2->info == "") {
													if ((auxRs1 != instBoa1->info.substr(startRd, endRd - startRd + 1)) && (auxRs2 != instBoa1->info.substr(startRd, endRd - startRd + 1))) {
														instBoa2 = aux;
														retirarLDE(listaOriginal, instBoa2);
														break;
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}

		if ((retornoAux == "IAE") || (retornoAux == "IM")) {
			if (auxanterior1 != nullptr) {
				if (auxanterior2 != nullptr) {
					//Verifica se existe uma instrucao possivel de retirar
					if ((auxRs1 != auxanterior1->info.substr(startRd, endRd - startRd + 1)) && (auxRs1 != auxanterior2->info.substr(startRd, endRd - startRd + 1))) {
						//Verifica se existe uma instrucao possivel de reinserir
						if (atual != nullptr) {
							if (atualanterior1 != nullptr) {
								//verifica os anteriores do atual(conflito) para ver se nao ira gerar problema
								if ((auxRs1 != atualRd) && (auxRs1 != atualAnterior1Rd)) {
									//verifica os proximos do atual(conflito) para ver se nao ira gerar problema
									if (atualproximo1 != nullptr) {
										if (auxRs1 != atualproximo1->info.substr(startRd, endRd - startRd + 1)) {
											if (instBoa1->info == "") {
												instBoa1 = aux;
												retirarLDE(listaOriginal, instBoa1);
												aux = aux->eloP;
												continue;
											}
										}
									}
								}
							}
							if (auxRs1 != atual->info.substr(startRd, endRd - startRd + 1)) {
								if (atualproximo1 != nullptr) {
									if (atualproximo2 != nullptr) {
										if ((auxRs1 != atualproximo1->info.substr(startRd, endRd - startRd + 1)) && (auxRs1 != atualproximo2->info.substr(startRd, endRd - startRd + 1))) {
											if (instBoa1->info != "") {
												if (instBoa2->info == "") {
													if (auxRs1 != instBoa1->info.substr(startRd, endRd - startRd + 1)) {
														instBoa2 = aux;
														retirarLDE(listaOriginal, instBoa2);
														break;
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}

		}

		aux = aux->eloP;
	}


	if (instBoa1->info != "") {
		if (instBoa2->info != "") {
			reinserir2Inst(listaOriginal, atual, instBoa1, instBoa2);
			return true;
		}
		reinserirInstInserir1Nop(listaOriginal, atual, instBoa1);
		return true;
	} else {
		return false;
	}

}

void solucao4(LDE <string>& listaOriginal) {
	int startOpcode = 25, endOpcode = 31;
	int startRd = 20, endRd = 24;
	int startRs1 = 12, endRs1 = 16;
	int startRs2 = 7, endRs2 = 11;
	string retornoInst1 = "", retornoInst2 = "", retornoAux = "";
	Instrucao instrucao;
	No <string>* auxproximo1 = new No <string>;
	No <string>* auxproximo2 = new No <string>;
	No <string>* aux = new No <string>;
	aux = listaOriginal.comeco;

	if (listaOriginal.comeco != NULL) {
		while (aux != NULL) {
			instrucao.opCode = aux->info.substr(startOpcode, endOpcode - startOpcode + 1);
			instrucao.rd = aux->info.substr(startRd, endRd - startRd + 1);
			instrucao.rs1 = aux->info.substr(startRs1, endRs1 - startRs1 + 1);
			instrucao.rs2 = aux->info.substr(startRs2, endRs2 - startRs2 + 1);
			instrucao.instCompleta = aux->info;


			if (aux->eloP != nullptr) {
				auxproximo1 = aux->eloP;
				if (aux->eloP->eloP != nullptr) {
					auxproximo2 = aux->eloP->eloP;
				}
				else {
					auxproximo2 = nullptr;
				}
			}
			else {
				auxproximo1 = nullptr;
			}

			retornoAux = verificaInstrucao(aux->info);
			if (auxproximo1 != nullptr) {
				retornoInst1 = verificaInstrucao(auxproximo1->info);
				if (auxproximo2 != nullptr) {
					retornoInst2 = verificaInstrucao(auxproximo2->info);
				}
			}

			if (retornoAux == "NOP") {
				aux = aux->eloP;
				continue;
			}

			if (retornoAux == "IM") {
				if (auxproximo1 != nullptr) {
					if ((retornoInst1 == "B") || (retornoInst1 == "S") || (retornoInst1 == "R")) {
						if ((instrucao.rd == auxproximo1->info.substr(startRs1, endRs1 - startRs1 + 1)) || (instrucao.rd == auxproximo1->info.substr(startRs2, endRs2 - startRs2 + 1))) {
							if (reordenar1InstForwarding(listaOriginal, aux)) {
								aux = aux->eloP;
								continue;
							}
							else {
								insere1Nop(listaOriginal, aux);
								aux = aux->eloP;
								continue;
							}
						}
					}
					else if ((retornoInst1 == "IM") || (retornoInst1 == "IAE")) {
						if (instrucao.rd == auxproximo1->info.substr(startRs1, endRs1 - startRs1 + 1)) {
							if (reordenar1InstForwarding(listaOriginal, aux)) {
								aux = aux->eloP;
								continue;
							}
							else {
								insere1Nop(listaOriginal, aux);
								aux = aux->eloP;
								continue;
							}
						}
					}
				}
			}

			aux = aux->eloP;

		}
	}
}

void solucao3(LDE <string>& listaOriginal) {
	int startOpcode = 25, endOpcode = 31;
	int startRd = 20, endRd = 24;
	int startRs1 = 12, endRs1 = 16;
	int startRs2 = 7, endRs2 = 11;
	string retornoInst1 = "", retornoInst2 = "", retornoAux = "";
	Instrucao instrucao;
	No <string>* auxproximo1 = new No <string>;
	No <string>* auxproximo2 = new No <string>;
	No <string>* aux = new No <string>;
	aux = listaOriginal.comeco;

	if (listaOriginal.comeco != NULL) {
		while (aux != NULL) {
			instrucao.opCode = aux->info.substr(startOpcode, endOpcode - startOpcode + 1);
			instrucao.rd = aux->info.substr(startRd, endRd - startRd + 1);
			instrucao.rs1 = aux->info.substr(startRs1, endRs1 - startRs1 + 1);
			instrucao.rs2 = aux->info.substr(startRs2, endRs2 - startRs2 + 1);
			instrucao.instCompleta = aux->info;


			if (aux->eloP != nullptr) {
				auxproximo1 = aux->eloP;
				if (aux->eloP->eloP != nullptr) {
					auxproximo2 = aux->eloP->eloP;
				}
				else {
					auxproximo2 = nullptr;
				}
			}
			else {
				auxproximo1 = nullptr;
			}

			retornoAux = verificaInstrucao(aux->info);
			if (auxproximo1 != nullptr) {
				retornoInst1 = verificaInstrucao(auxproximo1->info);
				if (auxproximo2 != nullptr) {
					retornoInst2 = verificaInstrucao(auxproximo2->info);
				}
			}

			if (retornoAux == "NOP") {
				aux = aux->eloP;
				continue;
			}


			if (auxproximo1 != nullptr) {
				if (retornoInst1 == "J") {
					if (reordenar1Inst(listaOriginal, aux)) {
						aux = aux->eloP;
						continue;
					}
					else {
						insere1Nop(listaOriginal, aux);
						aux = aux->eloP;
						continue;
					}
				}
			}

			if (retornoAux == "IAE" || retornoAux == "U" || retornoAux == "IM" || retornoAux == "R") {
				if (auxproximo1 != nullptr) {
					if ((retornoInst1 == "B") || (retornoInst1 == "S") || (retornoInst1 == "R")) {
						if ((instrucao.rd == auxproximo1->info.substr(startRs1, endRs1 - startRs1 + 1)) || (instrucao.rd == auxproximo1->info.substr(startRs2, endRs2 - startRs2 + 1))) {
							if (reordenar2Inst(listaOriginal, aux)) {
								aux = aux->eloP;
								continue;
							} else {
								insere2Nop(listaOriginal, aux);
								aux = aux->eloP;
								continue;
							}
						}
					}
					else if ((retornoInst1 == "IM") || (retornoInst1 == "IAE")) {
						if (instrucao.rd == auxproximo1->info.substr(startRs1, endRs1 - startRs1 + 1)) {
							if (reordenar2Inst(listaOriginal, aux)) {
								aux = aux->eloP;
								continue;
							} else {
								insere2Nop(listaOriginal, aux);
								aux = aux->eloP;
								continue;
							}
						}
					}
					if (auxproximo2 != nullptr) {
						if ((retornoInst2 == "B") || (retornoInst2 == "S") || (retornoInst2 == "R")) {
							if ((instrucao.rd == auxproximo2->info.substr(startRs1, endRs1 - startRs1 + 1)) || (instrucao.rd == auxproximo2->info.substr(startRs2, endRs2 - startRs2 + 1))) {
								if (reordenar1Inst(listaOriginal, auxproximo1)) {
									aux = aux->eloP;
									continue;
								} else{
									insere1Nop(listaOriginal, auxproximo1);
									aux = aux->eloP;
									continue;
								}
							}
						}
						else if ((retornoInst2 == "IM") || (retornoInst2 == "IAE")) {
							if (instrucao.rd == auxproximo2->info.substr(startRs1, endRs1 - startRs1 + 1)) {
								if (reordenar1Inst(listaOriginal, auxproximo1)) {
									aux = aux->eloP;
									continue;
								}
								else {
									insere1Nop(listaOriginal, auxproximo1);
									aux = aux->eloP;
									continue;
								}
							}
						}
					}
				}
			}

			aux = aux->eloP;

		}
	}
}

void solucao2(LDE <string>& listaOriginal) {
	int startOpcode = 25, endOpcode = 31;
	int startRd = 20, endRd = 24;
	int startRs1 = 12, endRs1 = 16;
	int startRs2 = 7, endRs2 = 11;
	string retornoInst1 = "", retornoInst2 = "", retornoAux = "";
	Instrucao instrucao;
	No <string>* auxproximo1 = new No <string>;
	No <string>* auxproximo2 = new No <string>;
	No <string>* aux = new No <string>;
	aux = listaOriginal.comeco;

	if (listaOriginal.comeco != NULL) {
		while (aux != NULL) {
			instrucao.opCode = aux->info.substr(startOpcode, endOpcode - startOpcode + 1);
			instrucao.rd = aux->info.substr(startRd, endRd - startRd + 1);
			instrucao.rs1 = aux->info.substr(startRs1, endRs1 - startRs1 + 1);
			instrucao.rs2 = aux->info.substr(startRs2, endRs2 - startRs2 + 1);
			instrucao.instCompleta = aux->info;


			if (aux->eloP != nullptr) {
				auxproximo1 = aux->eloP;
				if (aux->eloP->eloP != nullptr) {
					auxproximo2 = aux->eloP->eloP;
				}
				else {
					auxproximo2 = nullptr;
				}
			}
			else {
				auxproximo1 = nullptr;
			}

			retornoAux = verificaInstrucao(aux->info);
			if (auxproximo1 != nullptr) {
				retornoInst1 = verificaInstrucao(auxproximo1->info);
				if (auxproximo2 != nullptr) {
					retornoInst2 = verificaInstrucao(auxproximo2->info);
				}
			}

			if (retornoAux == "NOP") {
				aux = aux->eloP;
				continue;
			}

			if (retornoAux == "IM") {
				if (auxproximo1 != nullptr) {
					if ((retornoInst1 == "B") || (retornoInst1 == "S") || (retornoInst1 == "R")) {
						if ((instrucao.rd == auxproximo1->info.substr(startRs1, endRs1 - startRs1 + 1)) || (instrucao.rd == auxproximo1->info.substr(startRs2, endRs2 - startRs2 + 1))) {
							insere1Nop(listaOriginal, aux);
							aux = aux->eloP;
							continue;
						}
					}
					else if ((retornoInst1 == "IM") || (retornoInst1 == "IAE")) {
						if (instrucao.rd == auxproximo1->info.substr(startRs1, endRs1 - startRs1 + 1)) {
							insere1Nop(listaOriginal, aux);
							aux = aux->eloP;
							continue;
						}
					}
				}
			}

			aux = aux->eloP;

		}
	}
}

void solucao1(LDE <string>& listaOriginal) {
	int startOpcode = 25, endOpcode = 31;
	int startRd = 20, endRd = 24;
	int startRs1 = 12, endRs1 = 16;
	int startRs2 = 7, endRs2 = 11;
	string retornoInst1 = "", retornoInst2 = "", retornoAux = "";
	Instrucao instrucao;
	No <string>* auxproximo1 = new No <string>;
	No <string>* auxproximo2 = new No <string>;
	No <string>* aux = new No <string>;
	aux = listaOriginal.comeco;

	if (listaOriginal.comeco != NULL) {
		while (aux != NULL) {
			instrucao.opCode = aux->info.substr(startOpcode, endOpcode - startOpcode + 1);
			instrucao.rd = aux->info.substr(startRd, endRd - startRd + 1);
			instrucao.rs1 = aux->info.substr(startRs1, endRs1 - startRs1 + 1);
			instrucao.rs2 = aux->info.substr(startRs2, endRs2 - startRs2 + 1);
			instrucao.instCompleta = aux->info;


			if (aux->eloP != nullptr) {
				auxproximo1 = aux->eloP;
				if (aux->eloP->eloP != nullptr) {
					auxproximo2 = aux->eloP->eloP;
				}
				else {
					auxproximo2 = nullptr;
				}
			}
			else {
				auxproximo1 = nullptr;
			}

			retornoAux = verificaInstrucao(aux->info);
			if (auxproximo1 != nullptr) {
				retornoInst1 = verificaInstrucao(auxproximo1->info);
				if (auxproximo2 != nullptr) {
					retornoInst2 = verificaInstrucao(auxproximo2->info);
				}
			}

			if (retornoAux == "NOP") {
				aux = aux->eloP;
				continue;
			}

			if (auxproximo1 != nullptr) {
				if (retornoInst1 == "J") {
					insere1Nop(listaOriginal, aux);
					aux = aux->eloP;
					continue;
				}
			}
			

			if (retornoAux == "IAE" || retornoAux == "U" || retornoAux == "IM" || retornoAux == "R") {
				if (auxproximo1 != nullptr) {
					if ((retornoInst1 == "B") || (retornoInst1 == "S") || (retornoInst1 == "R")) {
						if ((instrucao.rd == auxproximo1->info.substr(startRs1, endRs1 - startRs1 + 1)) || (instrucao.rd == auxproximo1->info.substr(startRs2, endRs2 - startRs2 + 1))) {
							insere2Nop(listaOriginal, aux);
							aux = aux->eloP;
							continue;
						}
					} else if ((retornoInst1 == "IM") || (retornoInst1 == "IAE")) {
						if (instrucao.rd == auxproximo1->info.substr(startRs1, endRs1 - startRs1 + 1)) {
							insere2Nop(listaOriginal, aux);
							aux = aux->eloP;
							continue;
						}
					}
					if (auxproximo2 != nullptr) {
						if ((retornoInst2 == "B") || (retornoInst2 == "S") || (retornoInst2 == "R")) {
							if ((instrucao.rd == auxproximo2->info.substr(startRs1, endRs1 - startRs1 + 1)) || (instrucao.rd == auxproximo2->info.substr(startRs2, endRs2 - startRs2 + 1))) {
								insere1Nop(listaOriginal, auxproximo1);
								aux = aux->eloP;
								continue;
							}
						} else if ((retornoInst2 == "IM") || (retornoInst2 == "IAE")) {
							if (instrucao.rd == auxproximo2->info.substr(startRs1, endRs1 - startRs1 + 1)) {
								insere1Nop(listaOriginal, auxproximo1);
								aux = aux->eloP;
								continue;
							}
						}
					}
				}
			}

			aux = aux->eloP;

		}
	}
}

void gerenciarCalculoOrganizacao() {
	Organizacao org1, org2, org3, org4;
	LDE <string> listaOriginal1, listaOriginal2, listaOriginal3, listaOriginal4;
	inicializarLDE(listaOriginal1);
	inicializarLDE(listaOriginal2);
	inicializarLDE(listaOriginal3);
	inicializarLDE(listaOriginal4);
	cout << "Insira o tempo de clock do Pipeline(Solucao 1):" << endl;
	cin >> org1.tempoClock;
	cout << "Insira o tempo de clock do Pipeline(Solucao 2):" << endl;
	cin >> org2.tempoClock;
	cout << "Insira o tempo de clock do Pipeline(Solucao 3):" << endl;
	cin >> org3.tempoClock;
	cout << "Insira o tempo de clock do Pipeline(Solucao 4):" << endl;
	cin >> org4.tempoClock;
	//solucao 1
	leBinarioArquivoEscreveLista(org1, listaOriginal1, "DumpBinario1.txt");
	solucao1(listaOriginal1);
	escreveListaParaTxt(listaOriginal1, "DumpSolucao1.txt", org1);
	//solucao 2
	leBinarioArquivoEscreveLista(org2, listaOriginal2, "DumpBinario2.txt");
	solucao2(listaOriginal2);
	escreveListaParaTxt(listaOriginal2, "DumpSolucao2.txt", org2);
	//solucao 3
	leBinarioArquivoEscreveLista(org3, listaOriginal3, "DumpBinario3.txt");
	solucao3(listaOriginal3);
	escreveListaParaTxt(listaOriginal3, "DumpSolucao3.txt", org3);
	//solucao 4
	leBinarioArquivoEscreveLista(org4, listaOriginal4, "DumpBinario4.txt");
	solucao4(listaOriginal4);
	escreveListaParaTxt(listaOriginal4, "DumpSolucao4.txt", org4);
	//resultado solucao 1
	cout << endl;
	cout << "Calculo Desempenho para a Solucao 1: " << endl;
	calcularImprimirDesempenho(org1);
	//resultado solucao 2
	cout << endl; cout << endl;
	cout << "Calculo Desempenho para a Solucao 2: " << endl;
	calcularImprimirDesempenho(org2);
	//resultado solucao 3
	cout << endl; cout << endl;
	cout << "Calculo Desempenho para a Solucao 3: " << endl;
	calcularImprimirDesempenho(org3);
	//resultado solucao 4
	cout << endl; cout << endl;
	cout << "Calculo Desempenho para a Solucao 4: " << endl;
	calcularImprimirDesempenho(org4);
	cout << endl; cout << endl;
}

//Menu inicial
int menuInicial() {
	int escolhaMenuInicial;
	do {
		cout << "Bem vindo ao Calculo de Instrucoes" << endl;
		cout << "1 - Entrar" << endl;
		cout << "2 - Sair" << endl;
		cin >> escolhaMenuInicial;
		switch (escolhaMenuInicial) {
		case 1:
			system("cls");
			gerenciarCalculoOrganizacao();
			return 0;
			break;
		case 2:
			system("cls");
			return 0;
			break;
		default:
			cout << "Opcao invalida. Tente novamente." << endl;
			system("pause");
			system("cls");
		}
	} while (escolhaMenuInicial != 1 && escolhaMenuInicial != 2);
	return 0;
}

int main() {
	menuInicial();
	return 0;
}
