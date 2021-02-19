#include <domain/common.h>

#include <tanzaku/rng/random.h>
#include <tanzaku/sa/temp_manager.h>
#include <tanzaku/sa/time_manager.h>

using namespace tanzaku::sa::temp_manager;
using namespace tanzaku::sa::time_manager;
using namespace tanzaku::rng;

Random rng;

int main()
{
  cin.tie(0);
  ios::sync_with_stdio(false);
  input(cin);

  double score = 0;
  for (int i = 0; i < P; i++) {
    score += scores[i].get_score();
  }

  for (int delta = 1024; delta > 0; delta /= 2) {
    for (int i = 0; i < 200000000 / 10; i++) {
      while (1) {
        int pi = rng.nextInt(P);
        int ri = rng.nextInt(R);
        int d = rng.nextInt(11) - 6;
        if (d != 0 && can_add(pi, ri, d)) {
          double cur_score = scores[pi].get_score();
          add(pi, ri, d);
          double new_score = scores[pi].get_score();
          if (cur_score - delta > new_score) { // 2
            add(pi, ri, -d);
          } else {
            score -= cur_score;
            score += new_score;
          }

          if (i % 100000 == 0) {
            cerr << "cur score " << i << " " << score << endl;
          }

          break;
        }
      }
    }
  }

  cerr << "saving" << endl;
  std::ofstream os("out.cereal", std::ios::binary);
  cereal::BinaryOutputArchive archive(os);
  archive(solution, scores);
}
