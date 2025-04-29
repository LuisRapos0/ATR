#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <semaphore>
#include <atomic>
#include <chrono>
#include <random>

// Global variables for synchronization
const int NUM_JOGADORES = 4;
std::counting_semaphore<NUM_JOGADORES> semaforo_cadeiras(NUM_JOGADORES); // Inicia com n-1 cadeiras, capacidade máxima n
std::condition_variable music_cv;
std::mutex music_mutex;

std::random_device dev;
std::mt19937 rng(dev());
std::uniform_int_distribution<std::mt19937::result_type> dist3(0,3);

// Administra a interface no terminal
class Interface_Terminal {
public:
  void print_inicio_jogo(int num_jogadores, int cadeiras){
    system("clear");
    std::cout << "-----------------------------------------------\n";
    std::cout << "Bem-vindo ao Jogo das Cadeiras Concorrente!\n";
    std::cout << "-----------------------------------------------\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "\nIniciando partida com " << num_jogadores << " jogadores e " << cadeiras << " cadeiras.\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "A música está tocando... 🎵";
    std::cout.flush();
  }

  void print_inicio_rodada(int num_jogadores, int cadeiras){
    std::cout << "-----------------------------------------------\n";
    std::cout << "\n";
    std::cout << "Próxima rodada com " << num_jogadores << " jogadores e " << cadeiras;
    if(cadeiras == 1){
      std::cout << " cadeira.\n";
    }
    else {
      std::cout << " cadeiras.\n";
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "A música está tocando... 🎵";
    std::cout.flush();
  }

  void print_fim_musica(){
    std::cout << "\n\n";
    std::cout << "> A música parou! Os jogadores estão tentando se sentar...\n";
    std::cout << "\n";
    std::cout << "-----------------------------------------------\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout.flush();
  }

  void print_sentado(int cadeira, int jogador_id){
    std::cout << "[Cadeira " << cadeira << "]: Ocupada por P" << jogador_id<< "\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout.flush();
  }

  void print_eliminado(int jogador_id){
    std::cout << "\n";
    std::cout << "Jogador P" << jogador_id << " não conseguiu uma cadeira e foi eliminado!\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout.flush();
  }

  void print_fim_jogo(int jogador_id){
    
    std::cout << "-----------------------------------------------\n";
    std::cout << "\n";
    std::cout << "🏆 Vencedor: Jogador P" << jogador_id << "! Parabéns! 🏆\n";
    std::cout << "\n";
    std::cout << "-----------------------------------------------\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "\n";
    std::cout << "Obrigado por jogar o Jogo das Cadeiras Concorrente!\n";
    std::cout.flush();
  }

};


class JogoDasCadeiras {
private:
  int num_jogadores;
  int cadeiras;
  int sentados;
  Interface_Terminal terminal;
  std::mutex printando;

public:
  std::mutex rodada_terminada;
  std::condition_variable rodada_cv;

  JogoDasCadeiras(int num_jogadores)
    : num_jogadores(num_jogadores), cadeiras(num_jogadores - 1), sentados(0) {}

  int get_num_cadeiras (){
    return cadeiras;
  }

  // Interface para iniciar o jogo
  void iniciar_jogo() {
    terminal.print_inicio_jogo(num_jogadores, cadeiras);
  }
  
  // Reduz o número de cadeiras e zera o número de jogadores sentados, e então chama no terminal para printar o inicio da rodada
  void iniciar_rodada() {
    cadeiras--;
    sentados = 0;
    terminal.print_inicio_rodada(num_jogadores, cadeiras);
  }

  // Libera a variavel de condição das threads dos jogadores e chama no terminal para que printe que a musica parou
  void parar_musica() {
    terminal.print_fim_musica();
    music_cv.notify_all();
  }

  // Reduz o número de jogadores restantes e chama no terminal para que printe os jogadores eliminados
  void eliminar_jogador(int jogador_id) {
    num_jogadores--;
    terminal.print_eliminado(jogador_id);
  }

  // Se conseguir sentar, chama no terminal para printar que o jogador se sentou, incrementa o numero de jogadores sentados e retorna true; False caso não consiga se sentar.
  bool tentar_sentar(int jogador_id) {
    if (sentados < cadeiras){
      printando.lock();
      terminal.print_sentado(sentados+1, jogador_id);
      sentados++;
      if (sentados == cadeiras){
        rodada_cv.notify_all();
      }
      printando.unlock();
      return true;
    }
    return false;
  }

  bool jogo_acabou() {
    if (num_jogadores == 1){
      return true;
    }
    return false;
  }

  void finalizar_jogo(int jogador_id){
    terminal.print_fim_jogo(jogador_id);
  }

};

class Jogador {
private:
  int id;
  JogoDasCadeiras& jogo;

public:
  Jogador(int id, JogoDasCadeiras& jogo)
      : id(id), jogo(jogo) {}

  void jogar() {
    std::unique_lock<std::mutex> lock(music_mutex);

    while(true){
      music_cv.wait(lock);

      semaforo_cadeiras.acquire();

      if(jogo.jogo_acabou()){
        jogo.finalizar_jogo(id);
        return;
      }

      bool sentou = jogo.tentar_sentar(id);
      if(!sentou){
        jogo.eliminar_jogador(id);
        jogo.rodada_cv.notify_all();
        return;
      }
    }
  }
};


class Coordenador {
private:
  JogoDasCadeiras& jogo;

public:
  Coordenador(JogoDasCadeiras& jogo)
    : jogo(jogo) {}

  void iniciar_jogo() {
    std::unique_lock<std::mutex> progresso_rodada(jogo.rodada_terminada);

    jogo.iniciar_jogo();
    
    while (true){
      std::this_thread::sleep_for(std::chrono::seconds(2+dist3(rng)));
      jogo.parar_musica();
      jogo.rodada_cv.wait(progresso_rodada);
      semaforo_cadeiras.release();
      jogo.rodada_cv.wait(progresso_rodada);

      if(jogo.jogo_acabou()){
        music_cv.notify_all();
        break;
      }

      jogo.iniciar_rodada();
      semaforo_cadeiras.release(jogo.get_num_cadeiras());
    }
  }
};


// Main function
int main() {
  JogoDasCadeiras jogo(NUM_JOGADORES);
  Coordenador coordenador(jogo);
  std::vector<std::thread> jogadores;

  // Criação das threads dos jogadores
  std::vector<Jogador> jogadores_objs;
  for (int i = 1; i <= NUM_JOGADORES; ++i) {
      jogadores_objs.emplace_back(i, jogo);
  }

  for (int i = 0; i < NUM_JOGADORES; ++i) {
      jogadores.emplace_back(&Jogador::jogar, &jogadores_objs[i]);
  }

  // Thread do coordenador
  std::thread coordenador_thread(&Coordenador::iniciar_jogo, &coordenador);

  // Esperar pelas threads dos jogadores
  for (auto& t : jogadores) {
      if (t.joinable()) {
          t.join();
      }
  }

  // Esperar pela thread do coordenador
  if (coordenador_thread.joinable()) {
      coordenador_thread.join();
  }

  return 0;
}

