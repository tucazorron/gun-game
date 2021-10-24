// ------------------------------------
//  BIBLIOTECAS

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// ------------------------------------
// JOGADORES

// array dos ids dos jogadores escolhidos
int choosen_players[6];

// array de todos os possiveis jogadores da 102FR
char all_players[16][12] = {
		"AUGUSTO",
		"BERNARDES",
		"FILLIPO",
		"GOIAS",
		"JOHN JOHN",
		"LEO ARANTES",
		"LEOZINHO",
		"LUIZ MITO",
		"MATEUS",
		"MICHEL",
		"PAGANO",
		"PZ",
		"SANTISTA",
		"TUCA",
		"VITAO",
		"VITINHO",
};

// struct de um jogador
typedef struct
{
	char name[12];
	int gun;
	int area;
	int is_dead;
} player;

// cria seis structs de jogadores
player players[6];

// ------------------------------------
// ARMAS

// array de todas as armas do gun game em sequencia
char guns[20][22] = {
		"PYTHON SPEED RELOADER",
		"MAKAROV DUAL WIELD",
		"SPAS-12",
		"STAKEOUT",
		"MP5K",
		"SKORPION DUAL WIELD",
		"AK74U",
		"M14",
		"M16",
		"FAMAS",
		"AUG",
		"HK21",
		"M60",
		"L96A1",
		"WA2000",
		"GRIM REAPER",
		"M72 LAW",
		"CHINA LAKE",
		"CROSSBOW",
		"BALLISTIC KNIFE",
};

// ------------------------------------
// NUKETOWN

// struct de uma area da nuketown
typedef struct
{
	char name[21];
	int connections[6];
} nuketown_area;

// cria 9 areas dentro da nuketown
nuketown_area nuketown[9];

// cria o mutex
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// cria barreira
pthread_barrier_t barrier;

// cria variavel de controle do jogo
int game_over = 0;

void humiliate(int killer, int victim)
{
	printf("\nHUMILHAÇÃO!\n");
	printf("%s humilhou %s\n", players[killer].name, players[victim].name);
	if (players[victim].gun > 0)
	{
		printf("%s: %s => %s\n", players[victim].name, guns[players[victim].gun], guns[players[victim].gun - 1]);
		players[victim].gun -= 1;
	}
	printf("\n");
}

void kill(int killer, int victim)
{
	printf("\nMORTE!\n");
	printf("%s (%s) %s\n", players[killer].name, guns[players[killer].gun], players[victim].name);
	if (players[killer].gun == 19)
	{
		game_over = 1;
	}
	else
	{
		printf("%s: %s => %s\n\n", players[killer].name, guns[players[killer].gun], guns[players[killer].gun + 1]);
		players[killer].gun += 1;
	}
}

void combat(int player1, int player2)
{
	int killer, victim;
	int russian_roulette = rand() % 2;
	if (russian_roulette)
	{
		killer = player1;
		victim = player2;
	}
	else
	{
		killer = player2;
		victim = player1;
	}
	players[victim].is_dead = 1;
	int gun_kill = rand() % 7;
	if (gun_kill)
	{
		kill(killer, victim);
	}
	else
	{
		humiliate(killer, victim);
	}
}

int get_enemy_id(int id)
{
	int enemys_ids[6];
	for (int i = 0; i < 6; i++)
	{
		enemys_ids[i] = -1;
	}
	int counter = 0;
	for (int i = 0; i < 6; i++)
	{
		if (players[i].area == players[id].area && id != i)
		{
			enemys_ids[counter] = i;
			counter++;
		}
	}
	int enemy = rand() % counter;
	return enemys_ids[enemy];
}

int is_someone_here(int id)
{
	int answer = 0;
	for (int i = 0; i < 6; i++)
	{
		if (players[i].area == players[id].area && id != i)
		{
			answer = 1;
			break;
		}
	}
	return answer;
}

void move(int id)
{
	int current_area = players[id].area;
	int new_area;
	int connection_index;
	do
	{
		connection_index = rand() % 6;
		new_area = nuketown[current_area].connections[connection_index];
	} while (new_area == -1);
	players[id].area = new_area;
	printf("%s entrou em %s\n", players[id].name, nuketown[new_area].name);
	sleep(1);
}

void spawn(int id)
{
	players[id].is_dead = 0;
	int area = rand() % 9;
	players[id].area = area;
	printf("%s nasceu em %s\n", players[id].name, nuketown[area].name);
	sleep(1);
}

void initial_countdown()
{
	printf("\n5\n");
	sleep(1);
	printf("4\n");
	sleep(1);
	printf("3\n");
	sleep(1);
	printf("2\n");
	sleep(1);
	printf("1\n\n");
	sleep(1);
	printf("PRIMEIRO JOGADOR A MATAR COM TODAS AS ARMAS VENCE\n\n");
	sleep(1);
}

// funcao dos jogadores
void *players_function(void *arg)
{
	int id = (*(int *)arg);
	spawn(id);
	if (id == 0)
	{
		initial_countdown();
	}
	pthread_barrier_wait(&barrier);
	while (1)
	{
		pthread_mutex_lock(&mutex);
		if (game_over)
		{
			pthread_mutex_unlock(&mutex);
			break;
		}
		if (players[id].is_dead)
		{
			spawn(id);
		}
		if (is_someone_here(id))
		{
			int enemy = get_enemy_id(id);
			combat(id, enemy);
		}
		int to_move = rand() % 2;
		if (to_move)
		{
			move(id);
		}
		if (is_someone_here(id))
		{
			int enemy = get_enemy_id(id);
			combat(id, enemy);
		}
		pthread_mutex_unlock(&mutex);
	}
	pthread_exit(0);
}

void start_game()
{
	system("clear");
	pthread_t threads[6];
	pthread_barrier_init(&barrier, NULL, 6);
	int *id;
	for (int i = 0; i < 6; i++)
	{
		id = (int *)malloc(sizeof(int));
		*id = i;
		pthread_create(&threads[i], NULL, players_function, (void *)id);
	}

	for (int i = 0; i < 6; i++)
	{
		pthread_join(threads[i], NULL);
	}
}

void create_player(int id, int index)
{
	choosen_players[id] = index;
	strcpy(players[id].name, all_players[index]);
	players[id].gun = 0;
	players[id].is_dead = 1;
	printf("- %s\n", players[id].name);
}

int was_player_chosen(int index)
{
	int answer = 0;
	for (int i = 0; i < 6; i++)
	{
		if (choosen_players[i] == index)
		{
			answer = 1;
			break;
		}
	}
	return answer;
}

void choose_players()
{
	system("clear");
	printf("=> JOGADORES\n\n");
	int index;
	for (int i = 0; i < 6; i++)
	{
		do
		{
			index = rand() % 16;
		} while (was_player_chosen(index));
		create_player(i, index);
	}
	printf("\n>> PRESSIONE ENTER:\n");
	getchar();
}

void initialize_nuketown_connections()
{
	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 6; j++)
		{
			nuketown[i].connections[j] = -1;
		}
	}

	nuketown[0].connections[0] = 1;
	nuketown[0].connections[1] = 2;
	nuketown[0].connections[2] = 3;
	nuketown[0].connections[3] = 8;

	nuketown[1].connections[0] = 0;
	nuketown[1].connections[1] = 2;
	nuketown[1].connections[2] = 3;
	nuketown[1].connections[3] = 8;

	nuketown[2].connections[0] = 0;
	nuketown[2].connections[1] = 1;
	nuketown[2].connections[2] = 8;

	nuketown[3].connections[0] = 0;
	nuketown[3].connections[1] = 1;
	nuketown[3].connections[2] = 8;

	nuketown[4].connections[0] = 5;
	nuketown[4].connections[1] = 6;
	nuketown[4].connections[2] = 7;
	nuketown[4].connections[3] = 8;

	nuketown[5].connections[0] = 4;
	nuketown[5].connections[1] = 6;
	nuketown[5].connections[2] = 8;

	nuketown[6].connections[0] = 4;
	nuketown[6].connections[1] = 5;
	nuketown[6].connections[2] = 8;

	nuketown[7].connections[0] = 4;
	nuketown[7].connections[1] = 8;

	nuketown[8].connections[0] = 0;
	nuketown[8].connections[1] = 1;
	nuketown[8].connections[2] = 3;
	nuketown[8].connections[3] = 4;
	nuketown[8].connections[4] = 5;
	nuketown[8].connections[5] = 7;
}

void initialize_nuketown_names()
{
	strcpy(nuketown[0].name, "CASA AMARELA JARDIM ");
	strcpy(nuketown[1].name, "CASA AMARELA EMBAIXO ");
	strcpy(nuketown[2].name, "CASA AMARELA EM CIMA ");
	strcpy(nuketown[3].name, "CASA AMARELA GARAGEM ");
	strcpy(nuketown[4].name, "CASA VERDE JARDIM ");
	strcpy(nuketown[5].name, "CASA VERDE EMBAIXO ");
	strcpy(nuketown[6].name, "CASA VERDE EM CIMA ");
	strcpy(nuketown[7].name, "CASA VERDE GARAGEM ");
	strcpy(nuketown[8].name, "RUA");
}

void create_nuketown()
{
	initialize_nuketown_names();
	initialize_nuketown_connections();
}

void welcome_screen()
{
	system("clear");
	printf("PROGRAMACAO CONCORRENTE\n");
	printf("2021/1\n");
	printf("PROJETO FINAL\n\n");
	printf("ARTUR FILGUEIRAS SCHEIBA ZORRON\n");
	printf("180013696\n\n");
	printf("BLACK OPS\n");
	printf("NUKETOWN\n");
	printf("GUN GAME\n");
	printf("102FR\n\n");
	printf(">> PRESSIONE ENTER:\n");
	getchar();
}

int main()
{
	srand(time(NULL));
	welcome_screen();
	create_nuketown();
	choose_players();
	start_game();
	return 0;
}
