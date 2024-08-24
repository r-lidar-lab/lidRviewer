#ifndef PSQUARE_H
#define PSQUARE_H

class PSquare
{
public:
  PSquare(double quantile) : q(quantile)
  {
    markers.resize(5);
    positions.resize(5);
    heights.resize(5);
    desiredPositions.resize(5);
    increments.resize(5);

    for (int i = 0; i < 5; ++i)
    {
      positions[i] = i + 1;
      desiredPositions[i] = 1 + 2 * i * q;
      increments[i] = q;
    }
  }

  void addDataPoint(double x)
  {
    if (n < 5)
    {
      markers[n] = x;
      n++;
      if (n == 5)
      {
        std::sort(markers.begin(), markers.end());
        for (int i = 0; i < 5; ++i)
        {
          heights[i] = markers[i];
        }
      }
      return;
    }

    int k;
    if (x < heights[0])
    {
      k = 0;
      heights[0] = x;
    }
    else if (x >= heights[4])
    {
      k = 3;
      heights[4] = x;
    }
    else
    {
      for (k = 1; k < 4; ++k)
      {
        if (x < heights[k])
        {
          break;
        }
      }
      k--;
    }

    for (int i = k + 1; i < 5; ++i)
    {
      positions[i]++;
    }

    for (int i = 0; i < 5; ++i)
    {
      desiredPositions[i] += increments[i];
    }

    for (int i = 1; i < 4; ++i)
    {
      double d = desiredPositions[i] - positions[i];
      if ((d >= 1 && positions[i + 1] - positions[i] > 1) || (d <= -1 && positions[i - 1] - positions[i] < -1))
      {
        d = (d >= 1) ? 1 : -1;
        double h = heights[i] + d / (positions[i + 1] - positions[i - 1]) *
          ((positions[i] - positions[i - 1] + d) * (heights[i + 1] - heights[i]) /
            (positions[i + 1] - positions[i]) +
              (positions[i + 1] - positions[i] - d) * (heights[i] - heights[i - 1]) /
                (positions[i] - positions[i - 1]));

        if (h > heights[i - 1] && h < heights[i + 1])
        {
          heights[i] = h;
        }
        else
        {
          heights[i] = heights[i] + d * (heights[i + d] - heights[i]) / (positions[i + d] - positions[i]);
        }
        positions[i] += d;
      }
    }
  }

  double getQuantile() const
  {
    return heights[2];
  }

private:
  int n = 0;
  double q;
  std::vector<double> markers;
  std::vector<int> positions;
  std::vector<double> heights;
  std::vector<double> desiredPositions;
  std::vector<double> increments;
};

#endif
