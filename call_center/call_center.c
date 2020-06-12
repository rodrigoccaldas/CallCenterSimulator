#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>
#include "linked_list.h"

#define LAMBDA 80 // 80 chamadas por hora
#define lista_de_eventos_totais 1000000
#define fila_geral 3 // O tamanho finito da fila_operador_geral do atendimento geral
#define tempo_medio_geral_dist_exp 2 // O tempo medio da distribuição exponencial do atandimento geral
#define tempo_medio_geral_para_especialista_dist_exp 2.5 // O tempo medio da distribuição exponencial do atandimento geral que vai para especialista
#define numero_operadores_gerais 4 // Numero de operadores na linha geral
#define numero_operadores_especiais 8 // Numero de operadores na linha especifico
#define chegada_operador_geral 1
#define partida_operador_geral 2
#define chegada_operador_especifico 3
#define partida_operador_especifico 4
#define DELAY 1 
#define previsao 2  
#define positivo 7  
#define negativo -7 
#define media 1 // Media = 1 min
#define desvio  0.33 // Desvio da media = 20s
#define janela 100  




int func_chegada_geral_especial () {  
  if ((rand() % 100) < 70) {
  	return chegada_operador_especifico;
  } else {
  	return chegada_operador_geral;
  }
}

//box muller (ver apontamentos do professor)
double func_box_muller(void) {    
  double U1, U2, W, mult;
  static double X1, X2;
  static int call = 0;

  if (call == 1) {
    call = !call;
    return (media + desvio * (double) X2);
  }

  do {
    U1 = -1.0 + ((double) rand () / RAND_MAX) * 2.0;
    U2 = -1.0 + ((double) rand () / RAND_MAX) * 2.0;
    W = U1*U1 + U2*U2;
  } while (W >= 1 || W == 0);

  mult = sqrt ((-2 * log (W)) / W);
  X1 = U1 * mult;
  X2 = U2 * mult;

  call = !call;

  return (media + desvio * (double) X1);
}


double func_calc_tempo(int tipo, double lambda) {    
  double u = 0, C = 0, S = 0, E = 0;

  if (tipo == partida_operador_geral) { 
    do {
    	u = (rand()%RAND_MAX+1) / (double)RAND_MAX;
    	S = -(log(u)*tempo_medio_geral_dist_exp);
  	} while ((S < 1) || (S > 5));// tempo minimo=1min, tempo maximo=5min
  	return S;

  } else if (tipo == partida_operador_especifico) { 
      do {
      	u = (rand()%RAND_MAX+1) / (double)RAND_MAX;
      	S = -(log(u)*tempo_medio_geral_para_especialista_dist_exp);
    	} while (S < 1);// tempo minimo=1min
    	return S;

  } else if (tipo == chegada_operador_especifico) { 
  	do {
  		E = func_box_muller();
  	} while ((E < 0.5) || (E > 2));// tempo minimo=30s, tempo maximo=120s
    return E;

  } else { 
  	u = (rand()%RAND_MAX+1) / (double)RAND_MAX;
    C = -(log(u)/lambda);
    return C;
  }
}


list * func_adiciona_novo_evento(double t, double tempo_de_chegada, int tipo, list* lista_de_eventos, double lambda) {    
  lista_de_eventos = lista_adiciona(lista_de_eventos, tipo, (t + func_calc_tempo(tipo, lambda)), tempo_de_chegada, 0, 0);
  return lista_de_eventos;
}


double func_calc_tempo_prevido(double delay_medio, int posicao_buffer, int eventos_buffer) {    //tempo previsto
  if (eventos_buffer == posicao_buffer) {
    return posicao_buffer * 0.5 * tempo_medio_geral_dist_exp;
  } else {
    return delay_medio * posicao_buffer;
  }
  return 0;
}


double func_calc_desvio_standard(double histograma_valores_previstos[], int contagem_delay) {    
  int i = 0;
  double soma = 0.0, media_desvio = 0.0, desvio_standard = 0.0;

  for (i = 0; i < contagem_delay; i++) {
      soma += histograma_valores_previstos[i];
  }
  media_desvio = soma / contagem_delay;
  for (i = 0; i < contagem_delay; i++) {
      desvio_standard += pow(histograma_valores_previstos[i] - media_desvio, 2);
  }

  return sqrt(desvio_standard / (contagem_delay - 1));
}


int* func_insercao_valores_histograma (int index, int* tamanho_histograma, int* histograma) {    
  int i = 0, hist_aux = 0;
  index = abs(index);
  if (index + 1 > *tamanho_histograma) {
    hist_aux = *tamanho_histograma;
    *tamanho_histograma = index + 1;
    histograma = (int*)realloc(histograma, (*tamanho_histograma) * sizeof(int));
    for (i = hist_aux; i < *tamanho_histograma; i++) {
      histograma[i] = 0;
    }
    if (histograma == NULL) {
      perror("realloc");
      return NULL;
    }
  }
  histograma[index]++;
  return histograma;
}


int saveInCSV(char* nome_ficheiro, int* histograma, int tamanho_histograma, int tipo_histograma, int codigo) {   
  FILE *CSV;
  int i;
  printf( "\n\n\tA guardar o histograma no ficheiro: \"%s\"\n", nome_ficheiro);

  CSV = fopen(nome_ficheiro,"a");
  if(CSV == NULL) {
    perror("fopen");
    return -1;
  }

  // histograma do delay
  if (tipo_histograma == DELAY) {
    fprintf(CSV, "Index, Delay\n");
    for (i = 0; i < tamanho_histograma; i++) {
      fprintf(CSV, "%d, %d\n", i, histograma[i]);
    }

  // histograma da previsao
  } else if (codigo == negativo){
    fprintf(CSV, "Index, Prevision\n");
    for (i = tamanho_histograma - 1; i > 0; i--) {
      fprintf(CSV, "%d, %d\n", -i, histograma[i]);
    }
  } else if (codigo == positivo){
    //fprintf(CSV, "Index, Prevision\n");
    for (i = 0; i < tamanho_histograma; i++) {
      fprintf(CSV, "%d, %d\n", i, histograma[i]);
    }
  }

  fclose(CSV);

  return 0;
}


int main(int argc, char* argv[]) {

  
  int minimo = 0, maximo = 0, salto = 0, tamanho = 0, k = 0, j = 0, codigo_sensibilidade = 0;
  int* valores_lambda = NULL;
  double* delays = NULL;
  char c = '\0';
  printf("\n\t************************************************************************************\n");
  printf("\t* Simulação trabalho final de STEL [Rodrigo Caldas; Tiago Ribeiro] *\n");
  printf("\t************************************************************************************\n");

    codigo_sensibilidade = 1;
    printf("\n\tDefina os valores da sensibilidade do lambda:\n");
    do {
      printf("\tminimo: ");
      scanf("%d", &minimo);
    } while (minimo < 0);
    do {
      printf("\tmaximo: ");
      scanf("%d", &maximo);
    } while (maximo <= 0);
    do {
      printf("\tsalto: ");
      scanf("%d", &salto);
    } while (salto <= 0);

    tamanho = (maximo - minimo) / (float)salto;
    valores_lambda = (int*)calloc(tamanho + 1, sizeof(int));
    delays = (double*)calloc(tamanho + 1, sizeof(double));


  for (k = minimo; k <= maximo; k += salto) {
    if (codigo_sensibilidade) {
      valores_lambda[j] = k;
    }
    int *histograma_delay = NULL, index_delay = 0, tamanho_histograma_delay = 0;
    int i = 0, tipo_auxiliar = 0, perdas = 0, contagem_delay = 0;
    double lambda = k / 70.0, tempo_auxiliar = 0, tempo_de_chegada = 0, calculo_tempo = 0, tempo_delay = 0, delay_geral_especial = 0;
    
    char nome_ficheiro_delay[70] = "";
    double delay_medio = 0, tempo_novo = 0, erro_absoluto = 0, erro_relativo = 0;
    double* histograma_valores_previstos = NULL;
    char nome_ficheiro_previsao[70] = "";
    int index_previsao = 0, tamanho_histograma_previsao_negativo = 0, tamanho_histograma_previsao_positivo = 0;
    int *histograma_previsao_positivo = NULL, *histograma_previsao_negativo = NULL;
    double delay_medio = 0, tempo_novo = 0, erro_absoluto = 0, erro_relativo = 0;
    double* histograma_valores_previstos = NULL;
    char nome_ficheiro_previsao[70] = "";
    int eventos_chegada = 0, canais_geral = 0, tamanho_buffer = fila_geral, eventos_buffer = 0;
    list *eventos = NULL, *fila_operador_geral = NULL;

    int eventos_especifica = 0, canais_especial = 0;
    list *fila_especifica = NULL;
    srand(time(NULL));

    strcat(nome_ficheiro_delay, argv[1]);
    histograma_delay = (int*)calloc(1, sizeof(int));

    strcat(nome_ficheiro_previsao, argv[2]);
    histograma_previsao_positivo = (int*)calloc(1, sizeof(int));
    histograma_previsao_negativo = (int*)calloc(1, sizeof(int));
    histograma_valores_previstos = (double*)calloc(1, sizeof(double));

    eventos = lista_adiciona(eventos, chegada_operador_geral, tempo_auxiliar, tempo_de_chegada, 0, 0);

    for(i = 1; i < lista_de_eventos_totais; i++) {
    	tempo_auxiliar = eventos->lista_tempo;
      tempo_de_chegada = eventos->lista_tempo_chegada;
      tipo_auxiliar = eventos->lista_tipo;
      eventos = lista_remove(eventos);

      if (tipo_auxiliar == chegada_operador_geral) { 
        tempo_de_chegada = tempo_auxiliar;
        eventos_chegada++;  
        if (canais_geral < numero_operadores_gerais) { 
          canais_geral++; 
          if (func_chegada_geral_especial() == chegada_operador_geral) { 
            eventos = func_adiciona_novo_evento(tempo_auxiliar, tempo_de_chegada, partida_operador_geral, eventos, lambda);
          } else { 
            eventos = func_adiciona_novo_evento(tempo_auxiliar, tempo_de_chegada, chegada_operador_especifico, eventos, lambda);
          }
        } else if (tamanho_buffer > 0) { 
            fila_operador_geral = lista_adiciona(fila_operador_geral, chegada_operador_geral, tempo_auxiliar, tempo_de_chegada, func_calc_tempo_prevido(delay_medio, fila_geral-tamanho_buffer, eventos_buffer), fila_geral-tamanho_buffer); 
            tamanho_buffer--;
            eventos_buffer++;
        } else { 
          perdas++;
        }
        calculo_tempo = func_calc_tempo(chegada_operador_geral, lambda);
        eventos = lista_adiciona(eventos, chegada_operador_geral, (tempo_auxiliar + calculo_tempo), (tempo_auxiliar + calculo_tempo), 0, 0);

      } else if (tipo_auxiliar == partida_operador_geral) { 
        canais_geral--;
        if (fila_operador_geral != NULL) { 
          canais_geral++; 
          if (func_chegada_geral_especial() == chegada_operador_geral) { 
            eventos = func_adiciona_novo_evento(fila_operador_geral->lista_tempo, fila_operador_geral->lista_tempo_chegada, partida_operador_geral, eventos, lambda);
          } else { 
            eventos = func_adiciona_novo_evento(fila_operador_geral->lista_tempo, fila_operador_geral->lista_tempo_chegada, chegada_operador_especifico, eventos, lambda);
          }

          index_delay = (int) 70 * (tempo_auxiliar - fila_operador_geral->lista_tempo);
          if ((histograma_delay = func_insercao_valores_histograma(index_delay, &tamanho_histograma_delay, histograma_delay)) == NULL) {
            printf("\n\n\t\t!!! Erro na inserção de valores no histograma de delay !!!\n\n");
            return -1;
          }
          tempo_delay += tempo_auxiliar - fila_operador_geral->lista_tempo;

          tempo_novo = (tempo_auxiliar - fila_operador_geral->lista_tempo) / (float) (fila_operador_geral->lista_buffer_elementos + 0.5);
          delay_medio = delay_medio - delay_medio / (float) janela + tempo_novo / (float) janela;

          contagem_delay++;
          histograma_valores_previstos = (double*)realloc(histograma_valores_previstos, (contagem_delay) * sizeof(double));
          histograma_valores_previstos[contagem_delay-1] = 70 * (fila_operador_geral->lista_tempo_previsto - (tempo_auxiliar - fila_operador_geral->lista_tempo));
          if (histograma_valores_previstos == NULL) {
            perror("realloc");
            return -1;
          }

          index_previsao = (int) (70 * (fila_operador_geral->lista_tempo_previsto - (tempo_auxiliar - fila_operador_geral->lista_tempo)));
          erro_absoluto += fabs(70 * (fila_operador_geral->lista_tempo_previsto - (tempo_auxiliar - fila_operador_geral->lista_tempo)));
          erro_relativo += (70 * (fila_operador_geral->lista_tempo_previsto - (tempo_auxiliar - fila_operador_geral->lista_tempo)));
          if (index_previsao < 0) {
            if ((histograma_previsao_negativo = func_insercao_valores_histograma(index_previsao, &tamanho_histograma_previsao_negativo, histograma_previsao_negativo)) == NULL) {
              printf("\n\n\t\t!!! Erro na inserção de valores no histograma de previsão negativa !!!\n\n");
              return -1;
            }
          } else {
            if ((histograma_previsao_positivo = func_insercao_valores_histograma(index_previsao, &tamanho_histograma_previsao_positivo, histograma_previsao_positivo)) == NULL) {
              printf("\n\n\t\t!!! Erro na inserção de valores no histograma de previsão positiva !!!\n\n");
              return -1;
            }
          }

          fila_operador_geral = lista_remove(fila_operador_geral);
          tamanho_buffer++;
        }

      } else if (tipo_auxiliar == chegada_operador_especifico) {
        eventos_especifica++;  
    		if (canais_especial < numero_operadores_especiais) { 
          eventos = func_adiciona_novo_evento(tempo_auxiliar, tempo_de_chegada, partida_operador_especifico, eventos, lambda);
          canais_especial++; 
          canais_geral--; 
          if (fila_operador_geral != NULL) {
            canais_geral++; 
            if (func_chegada_geral_especial() == chegada_operador_geral) { 
              eventos = func_adiciona_novo_evento(fila_operador_geral->lista_tempo, fila_operador_geral->lista_tempo_chegada, partida_operador_geral, eventos, lambda);
            } else { 
              eventos = func_adiciona_novo_evento(fila_operador_geral->lista_tempo, fila_operador_geral->lista_tempo_chegada, chegada_operador_especifico, eventos, lambda);
            }

            index_delay = (int) 70 * (tempo_auxiliar - fila_operador_geral->lista_tempo);
            if ((histograma_delay = func_insercao_valores_histograma(index_delay, &tamanho_histograma_delay, histograma_delay)) == NULL) {
              printf("\n\n\t\tErro na inserção de valores no histograma de delay\n\n");
              return -1;
            }
            tempo_delay += tempo_auxiliar - fila_operador_geral->lista_tempo;

            tempo_novo = (tempo_auxiliar - fila_operador_geral->lista_tempo) / (float) (fila_operador_geral->lista_buffer_elementos + 0.5);
            delay_medio = delay_medio - delay_medio / (float) janela + tempo_novo / (float) janela;

            contagem_delay++;
            histograma_valores_previstos = (double*)realloc(histograma_valores_previstos, (contagem_delay) * sizeof(double));
            histograma_valores_previstos[contagem_delay-1] = 70 * (fila_operador_geral->lista_tempo_previsto - (tempo_auxiliar - fila_operador_geral->lista_tempo));
            if (histograma_valores_previstos == NULL) {
              perror("realloc");
              return -1;
            }

            index_previsao = (int) (70 * (fila_operador_geral->lista_tempo_previsto - (tempo_auxiliar - fila_operador_geral->lista_tempo)));
            erro_absoluto += fabs(70 * (fila_operador_geral->lista_tempo_previsto - (tempo_auxiliar - fila_operador_geral->lista_tempo)));
            erro_relativo += (70 * (fila_operador_geral->lista_tempo_previsto - (tempo_auxiliar - fila_operador_geral->lista_tempo)));
            if (index_previsao < 0) {
              if ((histograma_previsao_negativo = func_insercao_valores_histograma(index_previsao, &tamanho_histograma_previsao_negativo, histograma_previsao_negativo)) == NULL) {
                printf("\n\n\t\tErro na inserção de valores no histograma de previsão negativa\n\n");
                return -1;
              }
            } else {
              if ((histograma_previsao_positivo = func_insercao_valores_histograma(index_previsao, &tamanho_histograma_previsao_positivo, histograma_previsao_positivo)) == NULL) {
                printf( "\n\n\t\tErro na inserção de valores no histograma de previsão positiva\n\n");
                return -1;
              }
            }

            fila_operador_geral = lista_remove(fila_operador_geral);
            tamanho_buffer++;
          }
          delay_geral_especial += tempo_auxiliar - tempo_de_chegada;
    		} else { 
          fila_especifica = lista_adiciona(fila_especifica, chegada_operador_especifico, tempo_auxiliar, tempo_de_chegada, 0, 0);
        }

      } else if (tipo_auxiliar == partida_operador_especifico) { 
        if (fila_especifica != NULL) { 
          eventos = func_adiciona_novo_evento(tempo_auxiliar, fila_especifica->lista_tempo_chegada, partida_operador_especifico, eventos, lambda);
          delay_geral_especial += tempo_auxiliar - fila_especifica->lista_tempo_chegada;
          canais_geral--;
          if (fila_operador_geral != NULL) {
            canais_geral++; 
            if (func_chegada_geral_especial() == chegada_operador_geral) { 
              eventos = func_adiciona_novo_evento(fila_operador_geral->lista_tempo, fila_operador_geral->lista_tempo_chegada, partida_operador_geral, eventos, lambda);
            } else { 
              eventos = func_adiciona_novo_evento(fila_operador_geral->lista_tempo, fila_operador_geral->lista_tempo_chegada, chegada_operador_especifico, eventos, lambda);
            }

            index_delay = (int) 70 * (tempo_auxiliar - fila_operador_geral->lista_tempo);
            if ((histograma_delay = func_insercao_valores_histograma(index_delay, &tamanho_histograma_delay, histograma_delay)) == NULL) {
              printf("\n\n\t\t!!! Erro na inserção de valores no histograma de delay !!!\n\n");
              return -1;
            }
            
            tempo_delay += tempo_auxiliar - fila_operador_geral->lista_tempo;

            tempo_novo = (tempo_auxiliar - fila_operador_geral->lista_tempo) / (float) (fila_operador_geral->lista_buffer_elementos + 0.5);
            delay_medio = delay_medio - delay_medio / (float) janela + tempo_novo / (float) janela;

            contagem_delay++;
            histograma_valores_previstos = (double*)realloc(histograma_valores_previstos, (contagem_delay) * sizeof(double));
            histograma_valores_previstos[contagem_delay-1] = 70 * (fila_operador_geral->lista_tempo_previsto - (tempo_auxiliar - fila_operador_geral->lista_tempo));
            if (histograma_valores_previstos == NULL) {
              perror("realloc");
              return -1;
            }

            index_previsao = (int) (70 * (fila_operador_geral->lista_tempo_previsto - (tempo_auxiliar - fila_operador_geral->lista_tempo)));
            erro_absoluto += fabs(70 * (fila_operador_geral->lista_tempo_previsto - (tempo_auxiliar - fila_operador_geral->lista_tempo)));
            erro_relativo += (70 * (fila_operador_geral->lista_tempo_previsto - (tempo_auxiliar - fila_operador_geral->lista_tempo)));
            if (index_previsao < 0) {
              if ((histograma_previsao_negativo = func_insercao_valores_histograma(index_previsao, &tamanho_histograma_previsao_negativo, histograma_previsao_negativo)) == NULL) {
                printf("\n\n\t\t!!! Erro na inserção de valores no histograma de previsão negativa !!!\n\n");
                return -1;
              }
            } else {
              if ((histograma_previsao_positivo = func_insercao_valores_histograma(index_previsao, &tamanho_histograma_previsao_positivo, histograma_previsao_positivo)) == NULL) {
                printf("\n\n\t\t!!! Erro na inserção de valores no histograma de previsão positiva !!!\n\n");
                return -1;
              }
            }

            fila_operador_geral = lista_remove(fila_operador_geral);
            tamanho_buffer++;
          }
          fila_especifica = lista_remove(fila_especifica);
        } else { 
          canais_especial--;
        }
      }
    }

    if(saveInCSV(nome_ficheiro_delay, histograma_delay, tamanho_histograma_delay, DELAY, 0) < 0) {
      perror("guardar em CSV");
      return -1;
    }
    printf("\tFicheiro \"%s\" gravado corretamente\n\n", nome_ficheiro_delay);
    free(histograma_delay);

    if(saveInCSV(nome_ficheiro_previsao, histograma_previsao_negativo, tamanho_histograma_previsao_negativo, previsao, negativo) < 0) {
      perror("guardar em CSV");
      return -1;
    }
    printf("\tFicheiro \"%s\" gravado corretamente\n\n", nome_ficheiro_previsao);
    free(histograma_previsao_negativo);

    if(saveInCSV(nome_ficheiro_previsao, histograma_previsao_positivo, tamanho_histograma_previsao_positivo, previsao, positivo) < 0) {
      perror("guardar em CSV");
      return -1;
    }
    printf("\tFicheiro \"%s\" gravado corretamente\n\n", nome_ficheiro_previsao);
    free(histograma_previsao_positivo);

    printf("\n\n * Resources: General_queue = %d, # General_purpose_operators = %d, # Area_specific_operators = %d \n", fila_geral, numero_operadores_gerais, numero_operadores_especiais);

    printf("\nProbability of a call being delayed at the input of the general-purpose answering system:: %.3f%%\n",eventos_buffer / (float)(eventos_chegada) * 100);

    printf( "\nProbability of a call being lost at the input of the general-purpose answering system: %.3f%%\n",perdas / (float)(eventos_chegada) * 100);

    printf( "\nAverage delay of the calls (for the calls that suffer delay at the input of the general-purpose answering system): = %.3f sec\n",tempo_delay / (float)eventos_buffer * 70);

    printf( "\nAverage total delay of the calls, since they arrive at the general-purpose answering system until they are answered by the area-specific answering system: %.3f sec\n", delay_geral_especial / (float)(eventos_especifica) * 70);

    printf( "\nExpected waiting time error \n");

    printf( "\nMean of the absolute error of the expected waiting time in the incoming operator_general queue:  %.3f sec\n",erro_absoluto / (float)(eventos_buffer));

    printf( "\nMean of the relative error of the expected waiting time in the incoming operator_general queue: %.3f sec\n",erro_relativo / (float)(eventos_buffer));

    printf( "\nStandard deviation from the expected waiting time in the operator_general queue: %.3f sec\n",func_calc_desvio_standard(histograma_valores_previstos, contagem_delay));

    printf( "\n *  Sensitivity analysis for %.0f calls/hour \n", lambda * 70);

    printf( "\nGENERAL(90%%): %.3f +- %.3f seg\n",delay_geral_especial / (float)(eventos_especifica) * 70, 1.65 * (func_calc_desvio_standard(histograma_valores_previstos, contagem_delay) / sqrt(contagem_delay)));

    if (codigo_sensibilidade) {
      delays[j] = delay_geral_especial / (float)(eventos_especifica) * 70;
      j++;
    } else {
      break;
    }

  }

  if (codigo_sensibilidade) {
    FILE *sensibilidade;
    char nome_ficheiro_sensibilidade[50] = "sensibilidade.csv";
    printf( "\n\n\tA gravar histograma em \"%s\"\n", nome_ficheiro_sensibilidade);
    sensibilidade = fopen(nome_ficheiro_sensibilidade, "w");
    if(sensibilidade == NULL) {
      perror("fopen");
      return -1;
    }
    fprintf(sensibilidade, "Lambda, Delays\n");
    for (k = 0; k <= tamanho; k++) {
      fprintf(sensibilidade, "%d, %lf\n", valores_lambda[k], delays[k]);
    }
    fclose(sensibilidade);
    printf( "\tFicheiro \"%s\" gravado corretamente\n\n", nome_ficheiro_sensibilidade);
    free(valores_lambda);
    free(delays);
  }

  return 0;
}
