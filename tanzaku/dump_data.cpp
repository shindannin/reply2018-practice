#include <domain/common.h>

void dump_unit(ostream &out)
{
  // サービス
  out << "project\tservice\tunits" << endl;
  for (int i = 0; i < P; i++) {
    for (int j = 0; j < S; j++) {
      out << i << "\t" << j << "\t" << projects[i].units[j] << endl;
    }
  }
}

void dump_latency(ostream &out)
{
  out << "region\tcountry\tlatency" << endl;
  for (int i = 0; i < R; i++) {
    for (int j = 0; j < C; j++) {
      out << i << "\t" << j << "\t" << regions[i].latency[j] << endl;
    }
  }
}

int main(int argc, char *argv[])
{
  ifstream in(argv[1]);
  ofstream ofs_latency(argv[2]);
  ofstream ofs_unit(argv[3]);

  input(in);
  dump_latency(ofs_latency);
  dump_unit(ofs_unit);
}
