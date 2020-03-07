#include <cstdlib>
#include <vector>
#include <list>
#include <map>
#include <numeric>
#include "kmer_t.hpp"
#include "read_kmers.hpp"

int main(int argc, char **argv) {
  std::vector <kmer_t> kmers = read_kmers(argv[1]);
  printf("%d kmers read.\n", kmers.size());

  std::map <std::string, kmer_t> kmer_hash;
  std::list <kmer_t> start_nodes;
  for (const auto &kmer : kmers) {
    kmer_hash[kmer.key] = kmer;
    if (kmer.backwardExt() == 'F') {
      start_nodes.push_back(kmer);
    }
  }
  kmers.clear();

  std::list <std::list <kmer_t>> contigs;

  for (const auto &kmer : start_nodes) {
    std::list <kmer_t> contig;
    contig.push_back(kmer);
    while (contig.back().forwardExt() != 'F') {
      contig.push_back(kmer_hash[contig.back().nextKmer()]);
    }
    contigs.push_back(contig);
  }

  int numKmers = std::accumulate(contigs.begin(), contigs.end(), 0,
    [] (int sum, const std::list <kmer_t> &contig) {
      return sum + contig.size();
    });

  printf("Made hash table!\n");
  printf("StartNodes has %d elements.\n", start_nodes.size());
  printf("%d contigs with a total of %d elements (%d average)\n", contigs.size(), numKmers,
    numKmers / contigs.size());
  return 0;
}
