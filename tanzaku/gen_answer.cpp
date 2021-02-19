#include <domain/common.h>

int main(int argc, char *argv[])
{
  cin.tie(0);
  ios::sync_with_stdio(false);

  std::ifstream in(argv[1]);
  input(in);

  {
    std::ifstream is(argv[2], std::ios::binary);
    if (is.is_open()) {
      cerr << "loading" << endl;
      cereal::BinaryInputArchive archive(is);
      archive(solution, scores);
    }
  }

  std::ofstream ofs(argv[3]);

  for (int i = 0; i < P; i++) {
    bool first = true;
    for (int j = 0; j < R; j++) {
      if (solution.buy[i][j] > 0) {
        if (!first) ofs << " ";
        first = false;
        ofs << regions[j].provider_index << " " << regions[j].region_index << " " << solution.buy[i][j];
      }
    }
    ofs << endl;
  }
}
