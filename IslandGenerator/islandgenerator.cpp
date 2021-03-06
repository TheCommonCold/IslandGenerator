#include "islandgenerator.h"
#include "PerlinNoise.h"
#include<string.h>
#include <iostream>
#include <math.h>
#include <time.h>
#include <ctime>
#include <vector>
#include <algorithm>
#include <fstream>

IslandGenerator::IslandGenerator()
{
	points_string = "[0.3,0.2],[0.1,0.9],[0.9,0.9],[0.6,0.7],[0.8,0.1]";
    land_noise = (float)0.36;
    land_height = (float)0.1;
    seabed_noise = (float)0.08;
    seabed_height = (float)0.15;
	water_height = (float)0.0;
	frequency = (float)3.6;
	resolution = 40;
	octaves = (float)0.43;
	scroll = (float)2.47;
	roughness = (float)0.14;
	roughness_frequency = (float)7;
    flatness_size = (float) 0.5;
    flatness_freq = (float) 3.6;
    flatness_strength = (float) 60;
	srand((unsigned)time(0));
	seed = rand();
	name = "output";
}

void IslandGenerator::saveToFile() {
	std::ofstream myfile(name + ".txt");
	for (int i = 0; i < resolution; i += 1) {
		for (int j = 0; j < resolution; j += 1) {
			myfile << height_map[i][j] << " ";
		}
		myfile << "\n";
	}
	myfile.close();

	std::ofstream jsonFile(name + "_params.json");
	jsonFile << "{\n";
	jsonFile << "\"points_string\"" << ": \"" << points_string << "\",\n";
	jsonFile << "\"land_noise\"" << ": \"" << land_noise << "\",\n";
	jsonFile << "\"land_height\"" << ": \"" << land_height << "\",\n";
	jsonFile << "\"seabed_noise\"" << ": \"" << seabed_noise << "\",\n";
	jsonFile << "\"seabed_height\"" << ": \"" << seabed_height << "\",\n";
	jsonFile << "\"water_height\"" << ": \"" << water_height << "\",\n";
	jsonFile << "\"frequency\"" << ": \"" << frequency << "\",\n";
	jsonFile << "\"resolution\"" << ": \"" << resolution << "\",\n";
	jsonFile << "\"octaves\"" << ": \"" << octaves << "\",\n";
	jsonFile << "\"scroll\"" << ": \"" << scroll << "\",\n";
    jsonFile << "\"flatness_size\"" << ": \"" << flatness_size << "\",\n";
    jsonFile << "\"flatness_freq\"" << ": \"" << flatness_freq << "\",\n";
    jsonFile << "\"flatness_strength\"" << ": \"" << flatness_strength << "\",\n";
	jsonFile << "\"roughness\"" << ": \"" << roughness << "\",\n";
	jsonFile << "\"roughness_frequency\"" << ": \"" << roughness_frequency << "\",\n";
	jsonFile << "\"seed\"" << ": \"" << seed << "\"\n";
	jsonFile << "}\n";
	jsonFile.close();
}

std::vector<std::vector<float>> IslandGenerator::generatePerlinMatrix() {
	PerlinNoise pn(seed);
	std::vector<std::vector<float>> perlinMatrix(resolution);
	for (int i = 0; i < resolution; i += 1) {
		perlinMatrix[i].resize(resolution, 0);
		for (int j = 0; j < resolution; j += 1) {
			double x = (double)j / ((double)resolution);
			double y = (double)i / ((double)resolution);
			perlinMatrix[i][j] =
				(float)(pn.noise(frequency * x, frequency * y, scroll) * (1 - octaves) +
					octaves * (pn.noise(frequency * 2 * x, frequency * 2 * y, scroll) / 2.0 +
						pn.noise(frequency * 4 * x, frequency * 4 * y, scroll) / 4.0 +
						pn.noise(frequency * 8 * x, frequency * 8 * y, scroll) / 8.0 +
						pn.noise(frequency * 16 * x, frequency * 16 * y, scroll) / 16.0 +
						pn.noise(frequency * 32 * x, frequency * 32 * y, scroll) / 32.0 +
						pn.noise(frequency * 64 * x, frequency * 64 * y, scroll) / 64.0));
		}
	}
	float min = perlinMatrix[0][0];
	float max = perlinMatrix[0][0];
	for (int i = 0; i < resolution; i += 1) {
		float temp_min = *std::min_element(perlinMatrix[i].begin(), perlinMatrix[i].end());
		if (temp_min < min) min = temp_min;
		float temp_max = *std::max_element(perlinMatrix[i].begin(), perlinMatrix[i].end());
		if (temp_max > max) max = temp_max;
	}
	for (int i = 0; i < resolution; i += 1) {
		for (int j = 0; j < resolution; j += 1) {
			perlinMatrix[i][j] = (perlinMatrix[i][j] - min) / (max - min);
		}
	}
	return perlinMatrix;
}

std::vector<std::vector<float>> IslandGenerator::generateHeightMap() {
    float range = 1 - seabed_height;
	std::vector<std::vector<float>> height_map(resolution);
    for (int i = 0; i < resolution; i++){
        height_map[i].resize(resolution, water_height);
            for (int j = 0; j < resolution; j++)
                height_map[i][j] = (float)((seabed_noise * 2.0 * (perlinMatrix[i][j] - 0.5) * range) - seabed_height);
    }
	return height_map;
}

std::vector<std::vector<float>> IslandGenerator::plotPolygon() {
    PerlinNoise pn(seed);
	std::vector<int> nodeX(resolution);
	int  nodes, pixelX, pixelY, i, j, swap;
	int polyCorners = points.size();
    std::vector<std::vector<float>> height_map(resolution);
    for (int i = 0; i < resolution; i += 1) {
        height_map[i].resize(resolution, 0);
    }
	std::vector<std::vector<float>> points_processed(polyCorners);

	for (i = 0; i < polyCorners; i++) {
		points_processed[i].push_back((int)floor(points[i][0] * (resolution - 3)) + 1);
		points_processed[i].push_back((int)floor(points[i][1] * (resolution - 3)) + 1);
	}

	//  Loop through the rows of the image.
	for (pixelY = 0; pixelY < resolution; pixelY++) {
		//  Build a list of nodes.
		nodes = 0; 
		j = polyCorners - 1;
		for (i = 0; i < polyCorners; i++) {
            if (points_processed[i][1] < (double)pixelY && points_processed[j][1] >= (double)pixelY
                || points_processed[j][1] < (double)pixelY && points_processed[i][1] >= (double)pixelY) {
				nodeX[nodes++] = (int)(points_processed[i][0] + (pixelY - points_processed[i][1]) / (points_processed[j][1] - points_processed[i][1])
					* (points_processed[j][0] - points_processed[i][0]));
			}
			j = i;
		}

		//  Sort the nodes, via a simple “Bubble” sort.
		i = 0;
		while (i < nodes - 1) {
			if (nodeX[i] > nodeX[i + 1]) {
				swap = nodeX[i]; nodeX[i] = nodeX[i + 1]; nodeX[i + 1] = swap; if (i) i--;
			}
			else {
				i++;
			}
		}

		//  Fill the pixels between node pairs.
		for (i = 0; i < nodes; i += 2) {
			if (nodeX[i] >= resolution) break;
			if (nodeX[i + 1] > 0) {
				if (nodeX[i] < 0) nodeX[i] = 0;
				if (nodeX[i + 1] > resolution) nodeX[i + 1] = resolution;
				for (pixelX = nodeX[i]; pixelX < nodeX[i + 1]; pixelX++) {
                    int splash = (int)abs(roughness * resolution * (pn.noise(roughness_frequency/20 * pixelX, roughness_frequency/20 * pixelY, scroll) - 0.5));
                    for (int h = std::max(pixelY-splash,0); h <= std::min(pixelY+splash,resolution-2); h++) {
                        height_map[pixelX][h] = 1;
                    }
                    for (int h = std::max(pixelX-splash,0); h <= std::min(pixelX+splash,resolution-2); h++) {
                        height_map[h][pixelY] = 1;
                    }
				}
			}
		}
	}
	return height_map;
}

float distance(int x1, int y1, int x2, int y2)
{
    return sqrt(pow(x2 - x1, 2) +
                pow(y2 - y1, 2) * 1.0);
}

std::vector<std::vector<float>> IslandGenerator::plateu() {
    std::vector<std::vector<float>> height_map(resolution);
    for (int i = 0; i < resolution; i += 1) {
        height_map[i].resize(resolution, 0);
    }

    PerlinNoise pn(seed);
        for (int i = 0; i < resolution; i++) {
            for (int j = 0; j < resolution; j++) {
                if (pn.noise(flatness_freq * 0.03*i, scroll, flatness_freq * 0.03*j)<flatness_size)
                    height_map[i][j] = pn.noise(flatness_freq * 0.03*i, scroll, flatness_freq * 0.03*j);
            }
        }

    return height_map;
}

std::vector<std::vector<float>> IslandGenerator::extrudeTerrain(std::vector<std::vector<float>> landMap) {
    float range = 1 - land_height;
    std::vector<std::vector<float>> height_map = generateHeightMap();
    for (int i = 0; i < resolution; i += 1) {
        for (int j = 0; j < resolution; j += 1) {
            if(landMap[i][j]>0){
                height_map[i][j] = float((landMap[i][j] * land_noise * 2 * (perlinMatrix[i][j] - 0.5) * range) + land_height);
            }
        }
    }

    std::vector<std::vector<float>> output_map(resolution);
    for (int i = 0; i < resolution; i += 1) {
        output_map[i].resize(resolution, 0);
    }

    std::vector<std::vector<float>> avg_map = plateu();
    int size = 1;
    for (int k = 0; k < flatness_strength; k++){
        for (int i = 0; i < resolution; i += 1) {
            for (int j = 0; j < resolution; j += 1) {
                float avg = 0.0;
                int divider = 0;
                for (int h = std::max(i-size,0); h <= std::min(i+size,resolution-2); h++) {
                    if(j!=h)  {
                        avg += height_map[j][h];
                        divider++;
                    }
                }
                for (int h = std::max(j-size,0); h <= std::min(j+size,resolution-2); h++) {
                    if(i!=h) {
                        avg += height_map[h][i];
                        divider++;
                    }
                }
                avg = avg / divider;
                output_map[j][i] = (1-avg_map[j][i])*height_map[j][i] + avg_map[j][i]*avg;
            }
        }
        height_map = output_map;
    }

    return height_map;
}

void IslandGenerator::loadPoints() {
	std::vector<std::vector<float>> new_points;
	float point_x, point_y;
	size_t pos = 0;
	points_string.erase(std::remove(points_string.begin(), points_string.end(), '['), points_string.end());
	points_string.erase(std::remove(points_string.begin(), points_string.end(), ']'), points_string.end());
	std::string temp_string = points_string;
	std::string token;
	int i = 0;
	while ((pos = temp_string.find(',')) != std::string::npos) {
		token = temp_string.substr(0, pos);
		if (i == 0) {
			point_x = std::stof(token);
		}
		else {
			point_y = std::stof(token);
		}
		temp_string.erase(0, pos + 1);
		i++;
		if (i == 2) {
			new_points.push_back(std::vector<float>({ point_x, point_y }));
			i = 0;
		}
	}

	if (i == 0) {
		point_x = std::stof(temp_string);
	}
	else {
		point_y = std::stof(temp_string);
	}
	i++;
	if (i == 2) {
		new_points.push_back(std::vector<float>({ point_x, point_y }));
	}

	points = new_points;
}

void IslandGenerator::generateIsland() {
    loadPoints();
	perlinMatrix = generatePerlinMatrix();
    height_map = extrudeTerrain(plotPolygon());
}
