#pragma once

#include <string>

class kmer_t {
public:
  std::string key;
  std::string fb_ext;

  kmer_t(std::string key, std::string fb_ext) :
    key(key), fb_ext(fb_ext) {}

  kmer_t(const kmer_t &kmer) {
    key = kmer.key;
    fb_ext = kmer.fb_ext;
  }

  const char &forwardExt() const {
    return fb_ext[1];
  }

  const char & backwardExt() const {
    return fb_ext[0];
  }

  std::string nextKmer() const {
    return key.substr(1, std::string::npos) + forwardExt();
  }

  kmer_t() {}
};
