#ifndef HW_SURFACESIMPLIFICATION_H
#define HW_SURFACESIMPLIFICATION_H

#define _USE_MATH_DEFINES
#include <Eigen/Dense>
#include <vector>

namespace meshSimplification
{
	std::pair<std::vector<Eigen::Vector3f>, std::vector<Eigen::Vector3i>> simplifyMesh(std::vector<Eigen::Vector3f>, std::vector<Eigen::Vector3i>, float);
}

#endif //HW_SURFACESIMPLIFICATION_H
