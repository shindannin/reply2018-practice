#include <domain/common.h>

int main(int argc, char *argv[])
{
  cin.tie(0);
  ios::sync_with_stdio(false);

  std::ifstream in(argv[1]);
  input(in);

  {
    std::ifstream is("out.cereal", std::ios::binary);
    if (is.is_open()) {
      cerr << "loading" << endl;
      cereal::BinaryInputArchive archive(is);
      archive(solution, scores);
    }
  }

  for (int i = 0; i < P; i++) {
    bool first = true;
    for (int j = 0; j < R; j++) {
      // if (i != 0) continue;
      // if (solution.buy[i][j] < 0) throw;
      // used[j] += solution.buy[i][j];
      // cerr << used[j] << " " << solution.bought_packages[j] << " " << regions[j].available_packages << endl;
      // if (used[j] > regions[j].available_packages) throw;
      if (solution.buy[i][j] > 0) {
        if (!first) cout << " ";
        first = false;
        cout << regions[j].provider_index << " " << regions[j].region_index << " " << solution.buy[i][j];
      }
    }
    cout << endl;
  }

  // for (int i = 0; i < R; i++) {
  //   cerr << used[i] << " " << solution.bought_packages[i] << " " << regions[i].available_packages << endl;
  //   if (used[i] != solution.bought_packages[i]) throw;
  //   if (solution.bought_packages[i] > regions[i].available_packages) throw;
  // }
}
