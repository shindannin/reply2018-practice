#include <domain/common.h>

#include <tanzaku/rng/random.h>
#include <tanzaku/sa/temp_manager.h>
#include <tanzaku/sa/time_manager.h>

using namespace tanzaku::sa::temp_manager;
using namespace tanzaku::sa::time_manager;
using namespace tanzaku::rng;

Random rng;

void dump_service_distribution()
{
  // サービス
  cout << "project\tservice\tunits" << endl;
  for (int i = 0; i < P; i++) {
    for (int j = 0; j < S; j++) {
      cout << i << "\t" << j << "\t" << projects[i].units[j] << endl;
    }
  }
}

int main()
{
  cin.tie(0);
  ios::sync_with_stdio(false);
  input(cin);

  // dump_service_distribution();
  // return 0;

  // {
  //   std::ifstream is("out.cereal", std::ios::binary);
  //   if (is.is_open()) {
  //     cerr << "loading" << endl;
  //     cereal::BinaryInputArchive archive(is);
  //     archive(solution, scores);
  //   }
  // }

  double score = 0;
  for (int i = 0; i < P; i++) {
    // if (i != 0) continue;
    // cerr << i << " " << scores[i].get_score() << endl;
    score += scores[i].get_score();
    // score += scores[i].get_score2();
  }

  // int iter = 0;
  // for (auto c : candidates) {
  //   int p, r;
  //   double _;
  //   tie(p, r, _) = c;
  //   double cur_score = scores[p].get_score();
  //   double prev_score = cur_score;
  //   int v = 0;
  //   while (1) {
  //     add(p, r, 1);
  //     double new_score = scores[p].get_score();
  //     if (v++ > 0 && cur_score >= new_score) {
  //       add(p, r, -1);
  //       break;
  //     }
  //     cur_score = new_score;
  //   }
  //   score += cur_score - prev_score;

  //   if (iter++ % 1000000 == 0) {
  //     cerr << "cur score " << score << endl;
  //   }
  // }

  // for (int delta = 1024; delta > 0; delta /= 2) {
  // for (int delta = 1; delta > 0; delta /= 2) {
  //   for (int i = 0; i < 200000000 / 10; i++) {
  //     while (1) {
  //       int pi = rng.nextInt(P);
  //       int ri = rng.nextInt(R);
  //       int d = rng.nextInt(11) - 6;
  //       if (d != 0 && can_add(pi, ri, d)) {
  //         double cur_score = scores[pi].get_score();
  //         add(pi, ri, d);
  //         double new_score = scores[pi].get_score();
  //         // if (cur_score - 100000 > new_score) { // 1
  //         // if (cur_score - 1000 > new_score) { // 2
  //         // if (cur_score - delta > new_score) { // 2
  //         if (cur_score - delta > new_score) { // 2
  //           add(pi, ri, -d);
  //         } else {
  //           score -= cur_score;
  //           score += new_score;
  //         }

  //         if (i % 100000 == 0) {
  //           cerr << "cur score " << i << " " << score << endl;
  //         }

  //         // if ((i + 1) % 10000000 == 0) {
  //         //   cerr << "saving" << endl;
  //         //   std::ofstream os("out.cereal", std::ios::binary);
  //         //   cereal::BinaryOutputArchive archive(os);
  //         //   archive(solution, scores);
  //         // }

  //         break;
  //       }
  //     }
  //   }
  // }

  // vector<double> project_bests;
  // for (int project = 0; project < P; project++) {
  //   const int p = project;

  //   TemperatureManager temp_manager(100, 0);
  //   ConstantTimeManager time_manager(2000000);

  //   double cur_score = scores[p].get_score();
  //   while (1) {
  //     auto t = time_manager.get_time();

  //     if (t >= 1) {
  //       break;
  //     }

  //     auto temp = temp_manager.get_temp(t);
  //     while (1) {
  //       int r = rng.nextInt(R);
  //       int d = rng.nextInt(13) - 6;

  //       if (regions[r].latency[projects[p].country] >= 1500) {
  //         continue;
  //       }

  //       if (d != 0 && can_add(p, r, d)) {
  //         add(p, r, d);
  //         double new_score = scores[p].get_score();

  //         double diff = -(new_score - cur_score);
  //         if (diff <= -temp * rng.nextLog()) {
  //           cur_score = new_score;
  //         } else {
  //           add(p, r, -d);
  //         }

  //         // if ((i + 1) % 10000000 == 0) {
  //         //   cerr << "saving" << endl;
  //         //   std::ofstream os("out.cereal", std::ios::binary);
  //         //   cereal::BinaryOutputArchive archive(os);
  //         //   archive(solution, scores);
  //         // }

  //         break;
  //       }
  //     }
  //   }

  //   cerr << p << ": " << cur_score << endl;
  //   project_bests.push_back(cur_score);

  //   int used_region_count = 0;
  //   for (int r = 0; r < R; r++) {
  //     if (solution.buy[p][r]) {
  //       used_region_count++;
  //     }
  //     add(p, r, -solution.buy[p][r]);
  //   }
  //   cerr << used_region_count << " / " << R << endl;
  // }

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
    std::ofstream os("out.cereal", std::ios::binary);
    cereal::BinaryOutputArchive archive(os);
    archive(solution, scores);
  }

  // vector<int64_t> bought(R);
  // for (int p = 0; p < P; p++) {
  //   for (int r = 0; r < R; r++) {
  //     if (r != 724) continue;
  //     if (solution.buy[p][r] > 0) {
  //       cerr << "add " << p << " " << r << " " << solution.buy[p][r] << endl;
  //       bought[r] += solution.buy[p][r];
  //     }
  //   }
  // }

  // for (int r = 0; r < R; r++) {
  //   if (r != 724) continue;
  //   cerr << bought[r] << " " << solution.bought_packages[r] << " " << regions[r].available_packages << endl;
  //   if (bought[r] != solution.bought_packages[r]) throw;
  //   if (solution.bought_packages[r] > regions[r].available_packages) throw;
  // }

  // cerr << score << endl;

  // for (int i = 0; i < P; i++) {
  //   bool first = true;
  //   for (int j = 0; j < R; j++) {
  //     // if (i != 0) continue;
  //     // if (solution.buy[i][j] < 0) throw;
  //     // used[j] += solution.buy[i][j];
  //     // cerr << used[j] << " " << solution.bought_packages[j] << " " << regions[j].available_packages << endl;
  //     // if (used[j] > regions[j].available_packages) throw;
  //     if (solution.buy[i][j] > 0) {
  //       if (!first) cout << " ";
  //       first = false;
  //       cout << regions[j].provider_index << " " << regions[j].region_index << " " << solution.buy[i][j];
  //     }
  //   }
  //   cout << endl;
  // }

  // for (int i = 0; i < R; i++) {
  //   cerr << used[i] << " " << solution.bought_packages[i] << " " << regions[i].available_packages << endl;
  //   if (used[i] != solution.bought_packages[i]) throw;
  //   if (solution.bought_packages[i] > regions[i].available_packages) throw;
  // }
}
