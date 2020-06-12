#include<stdio.h>
#include<stdlib.h>
#include "linked_list.h"

// função que remove o primeiro elemento da lista
list * lista_remove (list * apontador){
	list * lista_aponta = (list *)apontador -> lista_proximo;
	free(apontador);
	return lista_aponta;
}

// função que adiciona um novo elemento na lista (ordenado pelo tempo)
list * lista_adiciona (list * apontador, int tipo_x, double tempo, double tempo_chegada, double tempo_prevido, int x_buffer_elementos){
	list * lista_aponta = apontador;
	list * apontador_auxiliar, * apontador_para_proximo;
	if(apontador == NULL) {
		apontador = (list *) malloc(sizeof (list));
		apontador -> lista_proximo = NULL;
		apontador -> lista_tipo = tipo_x;
		apontador -> lista_tempo = tempo;
		apontador -> lista_tempo_chegada = tempo_chegada;
		apontador -> lista_tempo_previsto = tempo_prevido;
		apontador -> lista_buffer_elementos = x_buffer_elementos;
		return apontador;
	} else {
		if (apontador->lista_tempo > tempo) {
	        apontador_auxiliar = (list *) malloc(sizeof (list));
	        apontador_auxiliar -> lista_tipo = tipo_x;
          apontador_auxiliar -> lista_tempo = tempo;
					apontador_auxiliar -> lista_tempo_chegada = tempo_chegada;
					apontador_auxiliar -> lista_tempo_previsto = tempo_prevido;
					apontador_auxiliar -> lista_buffer_elementos = x_buffer_elementos;
          apontador_auxiliar -> lista_proximo = (struct list *) apontador;
          return apontador_auxiliar;
	  }

		apontador_para_proximo = (list *)apontador -> lista_proximo;
		while(apontador != NULL) {
			if((apontador_para_proximo == NULL) || ((apontador_para_proximo -> lista_tempo) > tempo)) {
				break;
			}
			apontador = (list *)apontador -> lista_proximo;
			apontador_para_proximo = (list *)apontador -> lista_proximo;
		}
		apontador_auxiliar = (list *)apontador -> lista_proximo;
		apontador -> lista_proximo = (struct list *) malloc(sizeof (list));
		apontador = (list *)apontador -> lista_proximo;
		if(apontador_auxiliar != NULL) {
			apontador -> lista_proximo = (struct list *)apontador_auxiliar;
		}else{
			apontador -> lista_proximo = NULL;
		}
		apontador -> lista_tipo = tipo_x;
		apontador -> lista_tempo = tempo;
		apontador -> lista_tempo_chegada = tempo_chegada;
		apontador -> lista_tempo_previsto = tempo_prevido;
		apontador -> lista_buffer_elementos = x_buffer_elementos;
		return lista_aponta;
	}
}

