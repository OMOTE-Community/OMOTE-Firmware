#include <fstream>
#include <iostream>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

#include "key_map.h"

std::vector<KeyPadKey> loadKeypadMap() {
  // Map was created with https://n-peugnet.github.io/image-map-creator/ 
  std::ifstream f("./hardware/windows_linux/keypad_gui/buttons.map.json");

  json data = json::parse(f);

  int num_keys = data["map"]["areas"].size();

  std::vector<KeyPadKey> keypad_keys(num_keys);

  for (int i=0; i<num_keys; i++)
  {
    auto coords = data["map"]["areas"][i]["coords"];
    
    int num_coords = coords.size();

    float *vertx = new float[num_coords];
    float *verty = new float[num_coords];

    for (int j=0; j<num_coords; j++)
    {
      vertx[j] = (float)(coords[j]["x"]);
      verty[j] = (float)(coords[j]["y"]);
    }
    
    keypad_keys[i].vertx = vertx;
    keypad_keys[i].verty = verty;
    keypad_keys[i].num_vert = num_coords;
    std::string title = data["map"]["areas"][i]["title"].get<std::string>();
    keypad_keys[i].key = title[0];
    keypad_keys[i].id = i;
  }

  return keypad_keys;
}

