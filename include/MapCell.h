#pragma once

struct MapCell {
  float elevation;
  float intensity;
  int pointCount;

  float elevationSum;
  float minimumElevation;
};