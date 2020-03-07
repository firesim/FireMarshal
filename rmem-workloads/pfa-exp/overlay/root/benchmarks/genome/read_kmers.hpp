#pragma once

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "kmer_t.hpp"

int line_count(std::string fname) {
  std::ifstream fin(fname);
  std::string buf;
  int num_lines = 0;
  while (std::getline(fin, buf))
    ++num_lines;
  fin.close();
  return num_lines;
}

std::vector <kmer_t> read_kmers(std::string fname) {
  std::ifstream fin(fname);
  std::string buf;
  std::vector <kmer_t> kmers;
  while (std::getline(fin, buf)) {
    char key_buf[256];
    char fb_ext_buf[256];
    sscanf(buf.c_str(), "%s %s", key_buf, fb_ext_buf);
    kmers.push_back(kmer_t(std::string(key_buf), std::string(fb_ext_buf)));
  }
  return kmers;
}
