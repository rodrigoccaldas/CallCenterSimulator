#ifndef LINKED_LIST_H
#define LINKED_LIST_H

// definição da lista com os dados
typedef struct{
	int lista_tipo;
	double lista_tempo;
	double lista_tempo_chegada; // tempo de chegada de um evento ao atendimento geral
	double lista_tempo_previsto; // o tempo previsto de quanto tempo um a chamada fica no buffer do atendimento geral
	int lista_buffer_elementos; // número de elementos que estão no buffer do atendimento geral quando esse evento chega (exceto ele próprio)
	struct list * lista_proximo;
} list;

list * lista_remove (list * apontador);
list * lista_adiciona (list * apontador, int tipo_x, double tempo, double tempo_chegada, double tempo_prevido, int x_buffer_elementos);


#endif
