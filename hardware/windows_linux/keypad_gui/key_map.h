#include <vector>

struct KeyPadKey {
  float *vertx;
  float *verty;
  int num_vert;
  char key;
  int id;
};

std::vector<KeyPadKey> loadKeypadMap();