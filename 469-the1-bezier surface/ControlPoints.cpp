#include "ControlPoints.h"

using namespace std;

ControlPoints::ControlPoints(vector<vector<double>> matrix)
{
    n = matrix.size(), m = matrix[0].empty() ? 0 : matrix[0].size();
    double startingX = -0.5f, startingY = -0.5f;
    double endingX = 0.5f, endingY = 0.5f;
    double currX = startingX, currY = startingY;
    double stepX = (endingX - startingX) / (n - n/4), stepY = (endingY - startingY) / (m- m/4);        // MAYBE defined again n stuff
    double tx = 0, ty = 0;
    for(int i = 0; i < n; i+=4)
    {
        ty = 0;
        for(int j = 0; j < m; j+=4)
        {
            currX = (i-tx)*stepX + startingX;
            for(int k = 0; k < 4; k++)
            {
                currY = (j-ty)*stepY + startingY;
                for(int l = 0; l < 4; l++)
                {
                    if(abs(currX) < margin) currX = 0;
                    if(abs(currY) < margin) currY = 0;
                    CPV.push_back(glm::vec3(currX, currY, matrix[i+k][j+l]));
                    currY += stepY;
                }
                currX += stepX;
            }
            ty++;
        }
        tx++;
    }
}

