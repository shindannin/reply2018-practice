#include <domain/common.h>

#include <tanzaku/rng/random.h>
#include <tanzaku/sa/temp_manager.h>
#include <tanzaku/sa/time_manager.h>

using namespace tanzaku::sa::temp_manager;
using namespace tanzaku::sa::time_manager;
using namespace tanzaku::rng;

Random rng;

int main(int argc, char *argv[])
{
  std::ifstream in(argv[1], std::ios::binary);
  input(in);

  std::ifstream ifs_state(argv[2], std::ios::binary);
  if (ifs_state.is_open()) {
    cerr << "loading" << endl;
    cereal::BinaryInputArchive archive(ifs_state);
    archive(solution, scores);
  }

  vector<double> cur_scores(P);
  vector<pair<int64_t, int>> unit_project_pair;
  for (int p = 0; p < P; p++) {
    cur_scores[p] = scores[p].get_score();
    int64_t sum_unit = std::accumulate(projects[p].units.begin(), projects[p].units.end(), int64_t(0));
    unit_project_pair.emplace_back(sum_unit, p);
  }
  sort(begin(unit_project_pair), end(unit_project_pair));

  for (int pi = 0; pi < P; pi++) {
    const int p = unit_project_pair[pi].second;
    TemperatureManager temp_manager(10, 0);
    ConstantIterationTimeManager time_manager(100000 * 2 / 10);

    for (int64_t iter = 0;; iter++) {
      auto t = time_manager.get_time();

      if (t >= 1) {
        break;
      }

      auto temp = temp_manager.get_temp(t);
      // int p = rng.nextInt(P);
      int r = rng.nextInt(R);
      int d = rng.nextInt(13) - 6;

      if (regions[r].latency[projects[p].country] >= 1500) {
        continue;
      }

      if (d != 0 && can_add(p, r, d)) {
        add(p, r, d);
        double new_score = scores[p].get_score();

        double diff = -(new_score - cur_scores[p]);
        if (diff <= -temp * rng.nextLog()) {
          cur_scores[p] = new_score;
        } else {
          add(p, r, -d);
        }
      }
    }

    auto sum_score = std::accumulate(cur_scores.begin(), cur_scores.end(), int64_t(0));
    cerr << pi << " " << cur_scores[p] << " " << sum_score << endl;
  }

  {
    cerr << "saving" << endl;
    std::ofstream os(argv[2], std::ios::binary);
    cereal::BinaryOutputArchive archive(os);
    archive(solution, scores);
  }
}
