#ifndef UTIL_H
#define    UTIL_H

#include <string_view>
#include <vector>
#include <algorithm>
#include <iostream>
#include <chrono>

/// Cont�m fun��es utilit�rias que n�o se encaixam em nenhuma classe,
/// como convers�o de escalas lineares, tempo, data etc.
namespace Util
{

/// Encontra valor linear relativo � posi��o tridimensional em
/// uma matriz.
/// 
/// @param y, x, z Posi��o tridimensional na matriz.
/// @param Y, Z Dimens�es da matriz.
/// @return Um valor linear relativo � posi��o tridimensional.
constexpr int getPosition(int y, int x, int z, int Y, int Z);

/// Encontra a posi��o tridimensional atrav�s da posi��o linear e dimens�es
/// da matriz.
/// 
/// @param pLinear Posi��o linear conhecida.
/// @param [out] triDimensional Posi��o tridimensional, na ordem de 
///        bloco, dia, camada.
/// @param X, Y, Z Dimens�es da matriz.
void get3DMatrix(std::size_t pLinear, int triDimensional[3], int X, int Y, int Z);

/// Divide uma string em peda�os separados por um delimitador.
/// 
/// @param s String de origem, que ser� separada.
/// @param delim Delimitador que ser� usado para separar a string.
/// @param [out] elems Vector que ir� conter o resultado. Os elementos ser�o.
///        inseridos ao final, respeitando os que j� existem no container.
/// @return Refer�ncia para o vector de resultado, o mesmo passado em `elems`.
std::vector<std::string>& strSplit(const std::string& s, char delim, std::vector<std::string>& elems);

/// Divide uma string em peda�os separados por um delimitador.
/// 
/// @param s String de origem, que ser� separada.
/// @param delim Delimitador que ser� usado para separar a string.
/// @return Vector com as string separadas.
std::vector<std::string> strSplit(const std::string& s, char delim);

/// Calcula a diferen�a em milissegundos entre tf e t0.
/// 
/// @param tf Tempo final.
/// @param t0 Tempo inicial.
/// @return Diferen�a em milissegundos.
constexpr double timeDiff(clock_t tf, clock_t t0)
{
  return (tf - t0) / 1000000.0 * 1000;
}

/// Calcula diferen�a em milissegundos entre t_end e t_begin.
/// 
/// @param t_end Tempo final.
/// @param t_begin Tempo inicial.
/// @return Diferen�a em milissegundos.
long long chronoDiff(
    std::chrono::time_point<std::chrono::high_resolution_clock> t_end,
    std::chrono::time_point<std::chrono::high_resolution_clock> t_begin);

/// Constr�i um objeto que representa o momento em que a fun��o foi chamada.
///
/// Usado principalmente para calcular intervalos de tempo, em conjunto com
/// a fun��o Util::chronoDiff.j
/// 
/// @return Objeto representando o momento da chamada da fun��o.
std::chrono::time_point<std::chrono::high_resolution_clock> now();

/// Gera um inteiro aleat�rio entre min e max.
/// 
/// @param min Limitante inferior do intervalo.
/// @param max Limitante superior do intervalo.
/// @return Um inteiro em [min, max).
int randomBetween(int min, int max);

/// Gera um double aleat�rio entre 0 e 1.
/// 
/// @return Um double em [0, 1).
double randomDouble();

/// Ajusta `i` para que fique dentro do intervalo.
/// 
/// @param i �ndice a ser ajustado.
/// @param tamIntervalo Tamanho do intervalo.
/// @param comecoIntervalo Come�o do intervalo.
/// @return Valor ajustado.
constexpr int warpIntervalo(int i, int tamIntervalo, int comecoIntervalo)
{
    return (i - comecoIntervalo) % tamIntervalo + comecoIntervalo;
}

/// Insere um `item` em um container `c`, que deve se encontrar
/// ordenado.
/// 
/// @tparam Container Um Container que suporte std::upper_bound.
/// @tparam T Qualquer tipo compar�vel.
/// @tparam Compare Um functor usado para comparar T's.
/// @param c Container em que o item ser� inserido.
/// @param item Item a ser inserido.
/// @param cmp Function object usado para comparar os itens.
template <typename Container, typename T, typename Compare = std::less<T>>
void insert_sorted(Container& c, const T& item, Compare cmp = Compare())
{
    c.insert(
        std::upper_bound(begin(c), end(c), item, cmp),
        item
    );
}

/// Insere todos os item de `first` a `last` num container ordenado
/// 
/// @tparam Container Um Container que suporte std::upper_bound.
/// @tparam ForwardIterator Um iterator que possa ser incrementado.
/// @tparam Compare Um functor usado para comparar T's.
/// @param c Container em que os itens ser�o inseridos.
/// @param first Come�o do intervalo em que itens ser�o buscados.
/// @param last Fim do intervalo em que itens ser�o buscados.
/// @param cmp Function object que compara dois T's.
template <typename Container, typename ForwardIterator,
          typename Compare = std::less<typename ForwardIterator::value_type>>
void insert_sorted(Container& c, ForwardIterator first,
                   ForwardIterator last, Compare cmp = Compare())
{
    for (; first != last; ++first) {
        insert_sorted(c, *first, cmp);
    }
}

/// Une uma lista de nomes de pastas e um arquivo em uma string de forma
/// independente de plataforma.
/// 
/// @param folders Lista de pastas que comp�em o caminho.
/// @param file (Opcional) Arquivo dentro da �ltima pasta do caminho.
/// @return O caminho formatado da forma apopriada para o SO.
std::string join_path(const std::vector<std::string>& folders,
                      const std::string& file = "");

/// Cria uma pasta de forma independente de plataforma.
/// 
/// @param path Nome do arquivo, no formato suportado pelo
///        sistema operacional
void create_folder(const std::string& path);

/// Calcula o hash da string usando std::hash.
///
/// @param str String cujo hash ser� calculado.
/// @return Hash da string calculado pelo std::hash.
std::size_t hash_string(const std::string& str);

/// Calcula o fatorial de n, em tempo de compila��o se poss�vel.
/// 
/// @tparam Integral Um tipo inteiro que suporta multiplica��o.
/// @param n N�mero cujo fatorial ser� calculado (deve ser maior que 0).
/// @return Fatorial de n.
template <typename Integral>
constexpr Integral
factorial(Integral n)
{
  Integral x = 1;
  for (Integral i = 2; i <= n; ++i) {
    x *= i;
  }
  return x;
}

/// Calcula o ceil de um double, convertendo para int,
/// em dire��o a +inf.
/// 
/// @param x Double a ser arredondado para +inf.
/// @return Int que representa o resultado do ceil de x.
constexpr int
fast_ceil(double x)
{
  const auto ix = static_cast<int>(x);
  return ix + (x > ix);
}

/// Calcula o floor de um double, convertendo para int,
/// em dire��o a -inf.
/// 
/// @param x Double a ser arredondado para -inf.
/// @return Int que representa o resultado do floor de x.
constexpr int
fast_floor(double x)
{
  const auto ix = static_cast<int>(x);
  return ix - (x < ix);
}

/// Retorna iterator para um item aleat�rio do container.
/// 
/// @tparam Container Qualquer tipo que implemente ForwardIterator.
/// @param c Container cujo elemento ser� obtido.
/// @return Iterator para um elemento aleat�rio de `c`.
template <typename Container>
auto
randomChoice(Container& c)
{
  const auto n = randomBetween(0, static_cast<int>(c.size()));
  auto it = begin(c);
  std::advance(it, n);
  return it;
}

/// A partir de um vector de pares representando a chance de cada elemento
/// ocorrer, gera uma array com elementos proporcionais a essas chances.
///
/// Por exemplo, para um vector com {{1, 40}, {2, 50}, {3, 10}} � gerada
/// uma array com 40 1's, 50 2's e 10 3's.
/// A soma total das chances n�o deve exceder 100. Se o soma for menor
/// que 100, o �ltimo elemento ir� preencher at� totalizar 100.
/// 
/// @tparam T Qualquer tipo que possa ser copiado.
/// @param chances Vector com pares de (item, chance).
/// @return Array com 100 elementos do tipo T, de acordo com as
///         chances fornecidas.
template <typename T>
std::array<T, 100>
gen_chance_array(const std::vector<std::pair<T, int>>& chances)
{
  std::array<T, 100> chance_array;
  auto it = begin(chance_array);

  for (const auto& p : chances) {
    it = std::fill_n(it, p.second, p.first);
  }

  if (it != end(chance_array)) {
    std::fill(it, end(chance_array), chances.back().first);
  }

  return chance_array;
}

/// Calcula o hash de um valor qualquer, combinando com um
/// outro hash obtido anteriormente.
/// 
/// @tparam T Um tipo que seja suportado por std::hash.
/// @param seed [in,out] Valor que ser� combinado com o hash de `v`.
/// @param v Valor cujo hash ser� combinado.
template <class T>
void hash_combine(std::size_t& seed, const T& v)
{
    seed ^= std::hash<T>{ }(v) +0x9e3779b9 + (seed << 6) + (seed >> 2);
}

/// Functor que define um hash para std::pair's gen�ricos.
/// 
/// @tparam S, T Tipos que comp�em o pair.
template <typename S, typename T>
struct hash_pair
{
  /// Calcula o hash do par atrav�s da combina��o dos hashes dos elementos.
  /// 
  /// @param p Par cujo hash ser� calculado.
  /// @return Hash do par calculado pela combina��o dos hashes.
  std::size_t
  operator()(const std::pair<S, T>& p) const
  {
    std::size_t seed{ 0 };
    hash_combine(seed, p.first);
    hash_combine(seed, p.second);
    return seed;
  }
};

/// Obt�m a data atual no formato *d-m-Y H:M:S*.
/// 
/// @return Data atual no formato *d-m-Y H:M:S*.
std::string date_time();

/// Imprime `data` na sa�da padr�o e na stream `log`.
/// 
/// @param log Stream onde `data` tamb�m ser� impressa.
/// @param data Dados que ser�o impressos nas streams.
void logprint(std::ostream& log, std::string_view data);

/// Obt�m o hostname do computador. Limitado ao Windows.
///
/// @returns Nome do computador.
std::string get_computer_name();

} // namespace Util

#endif /* UTIL_H */

