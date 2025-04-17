#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <array>
#include <random>

// Gerador de número aleatório
std::random_device dev;
std::mt19937 rng(dev());
std::uniform_int_distribution<std::mt19937::result_type> dist3(0,2);

// Classe TicTacToe
class TicTacToe {
private:
  std::array<std::array<char, 3>, 3> board; // Tabuleiro do jogo

public:
  std::mutex board_mutex; // Mutex para controle de acesso ao tabuleiro
  std::condition_variable turn_cv; // Variável de condição para alternância de turnos

  TicTacToe() {
    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
        board.at(i).at(j) = ' ';
      }
    }
  }

  // Printa no terminal o estado do tabuleiro
  void display_board() {
    for (int i = 0; i < 3; i++) 
    {
      for (int j = 0; j < 3; j++) 
      {
        switch (board.at(i).at(j)) {
          case (' '):
            std::cout << "   ";
            break;

          case ('X'):
            std::cout << "\033[1;31m X \033[0m";
            break;

          case ('O'):
            std::cout << "\033[1;34m O \033[0m";
            break; 
        }
        if (j!=2) {
          std::cout << "|";
        }
      }
      if (i!=2) {
        std::cout << "\n-----------\n";
      }
    }
    std::cout << '\n';
  }

  /* se uma jogada for possível realiza ela e retorna true; 
     se não retorna false e não muda o tabuleiro */
  bool make_move(char player, int row, int col) {
    if (board.at(row).at(col) != ' '){
      return false;
    }
    board.at(row).at(col) = player;
    return true;
  }

  // Verifica se o jogador passado como argumento venceu
  bool check_win(char player) {
    if (board.at(0).at(0) == player && board.at(0).at(1) == player && board.at(0).at(2) == player)
      return true;
    if (board.at(1).at(0) == player && board.at(1).at(1) == player && board.at(1).at(2) == player)
      return true;
    if (board.at(2).at(0) == player && board.at(2).at(1) == player && board.at(2).at(2) == player)
      return true;
    if (board.at(0).at(0) == player && board.at(1).at(0) == player && board.at(2).at(0) == player)
      return true;
    if (board.at(0).at(1) == player && board.at(1).at(1) == player && board.at(2).at(1) == player)
      return true;
    if (board.at(0).at(2) == player && board.at(1).at(2) == player && board.at(2).at(2) == player)
      return true;
    if (board.at(0).at(0) == player && board.at(1).at(1) == player && board.at(2).at(2) == player)
      return true;
    if (board.at(2).at(0) == player && board.at(1).at(1) == player && board.at(0).at(2) == player)
      return true;
      
    return false;
  }

  // Verifica se o jogo terminou em empate
  bool check_draw() {
    for (int i = 0; i < 3 ; i++) {
      for (int j = 0; j < 3; j++) {
        if (board.at(i).at(j) == ' '){
          return false;
        }
      }
    }
    if (check_win('X') || check_win('O')){
      return false;
    }
    return true;
  }

  // Verifica se o jogo terminou
  bool is_game_over() {
    if (check_win('X') || check_win('O') || check_draw()){
      return true;
    }
    return false;
  }

  // Retorna como terminou o jogo
  char get_winner() {
    if (check_win('X'))
      return 'X';
    if (check_win('O'))
      return 'O';
    if (check_draw())
      return 'D';
    
    return 'E';
  }
};

// Classe Player
class Player {
private:
  TicTacToe& game; // Referência para o jogo
  char symbol; // Símbolo do jogador ('X' ou 'O')
  std::string strategy; // Estratégia do jogador

public:
  Player(TicTacToe& g, char s, std::string strat) 
      : game(g), symbol(s), strategy(strat) {}

  // Realiza as jogadas, com controle de sincronização por variavel de condição
  void play() {
    std::unique_lock<std::mutex>  lock(game.board_mutex);

    game.turn_cv.notify_all();

    while(!game.is_game_over()){

      game.turn_cv.wait(lock);
      if(game.is_game_over()){
        break;
      }

      if (strategy == "sequential") {
        play_sequential();
      }

      if (strategy == "random") {
        play_random();
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(500));
      system("clear");
      game.display_board();

      game.turn_cv.notify_all();
    }
  }

private:
  // Faz uma jogada com estratégia sequencial
  void play_sequential() {
    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
        if (game.make_move(symbol, i, j))
          return;
      }
    }
  }

  // Faz uma jogada com estratégia aleatória
  void play_random() {
    while(1){
      if(game.make_move(symbol, dist3(rng), dist3(rng)))
        return;
    }
  }
};

// Função principal
int main() {
  TicTacToe board;
  Player PlayerX = Player(board, 'X', "sequential");
  Player PlayerO = Player(board, 'O', "random");

  system("clear");
  board.display_board();
  std::cout << "Pressione enter para iniciar.";
  getchar();
  
  std::thread PX_thread(&Player::play, &PlayerX);
  std::thread PO_thread(&Player::play, &PlayerO);

  PX_thread.join();
  PO_thread.join();

  switch (board.get_winner()){
    case ('X'):
      std::cout << "O vencedor foi o jogador X!\n";
      break;
    case ('O'):
      std::cout << "O vencedor foi o jogador O!\n";
      break;
    case ('D'):
      std::cout << "O jogo terminou em empate!\n";
      break;
    case ('E'):
      std::cout << "Erro: O jogo terminou inconclusivamente!\n";
      break;
  }

  return 0;
}
